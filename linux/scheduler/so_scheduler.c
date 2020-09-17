#include <assert.h>
#include <math.h>

#include "utils.h"
#include "so_scheduler.h"
#include "scheduler_structure.h"
#include "priority_queue.h"

/**
 * planificator sistemului de thread-uri
 **/
static scheduler_type * g_scheduler;

/**
 * functia care ordoneaza thread-urile in codada cu prioritate
 * se alege thread-ul cu prioritatea mai mare, daca au aceiasi prioritate se
 * alege threadul care a rulat mai putin
 * @param data1 so_thread_info1
 * @param data2 so_thread_info2
 * @return == 0  daca au aceiasi prioritate thread-urile
 *         <  0  daca thread-ul 1 trebui planificat inaintea lui 2
 *         >  0  daca thread-ul 2 trebui planificat inaintea lui 1
 */
static int compare_priority(void *data1, void *data2)
{

	so_thread_info thread1 = *(so_thread_info *) data1;
	so_thread_info thread2 = *(so_thread_info *) data2;

	return thread1.priority - thread2.priority;
}

/**
 * functia elibereaza memoria pentru un thread
 * @param data so_thread_info
 */
static void free_thread_info(void *data)
{
	so_thread_info *thread = (so_thread_info *) data;
	int rc;

	rc = sem_destroy(&thread->wait);
	DIE(rc != 0, "Sem Destroy");
	rc = sem_destroy(&thread->create);
	DIE(rc != 0, "Sem Destroy");

	free(thread);
}


/**
 * functia care ruleaza comporntamentul planificatorului cand un thread se
 * bloceza si asteapta un despozitiv io
 * @param io indexul dipozitivului
 */
static void scheduler_wait_state(scheduler_type *scheduler, int io)
{
	int rc;
	/** thread-ul care executa instructiunea curenta */
	so_thread_info *current_thread = scheduler->runnig_thread;
	/** primul thread din coada ready, urmatorul care va fi planificat */
	so_thread_info *next_thread = top_pq(scheduler->ready_thread);

	/**
	 * nu se permite decat unui thread din sistem sa apeleza so_wait,
	 * si trebui sa existe cel putin un alt thread in sistem in acel moment
	 */
	if (current_thread == NULL || next_thread == NULL)
		DIE(TRUE, "Illegal Call");

	/** este reset indicatorul care indica de cat timp ruleaza thread-ul */
	current_thread->time_run = 0;
	/** incrementarea timuplui de rulare pt un thread */
	current_thread->total_run_time++;

	/** adaugarea in lista de astetare */
	rc = add_linked_list(&scheduler->wait_thread[io], current_thread);
	DIE(rc != ADD_SUCCESS, "Add fail");
	/** eliberea urmatorului thread */
	RELEASE_NEXT_THREAD(scheduler, next_thread, rc);

	/** thread-ul va astepta */
	rc = sem_wait(&current_thread->wait);
	DIE(rc != 0, "Sem Wait");

}

/**
 * thread-ul curent este adugat in list care indica tread-urile care au terminat
 * execitia si verica daca mai sunt thread-uril in sistem, in care negativ
 * se incrementeaza semaforul care indica terminarea programului
 */
static void scheduler_terminate_state(scheduler_type *scheduler)
{
	int rc;
	so_thread_info *current_thread = scheduler->runnig_thread;
	so_thread_info *next_thread = top_pq(scheduler->ready_thread);

	rc = add_linked_list(&scheduler->terminate_thread, current_thread);
	DIE(rc != ADD_SUCCESS, "Add Linked List!");

	/**
	 * nu  mai sunt thread-uri in sistem, asta presupunem ca s-a termintat
	 * toata executai programului
	 */
	if (next_thread == NULL) {
		rc = sem_post(&scheduler->finish);
		DIE(rc != 0, "Sem Post");

		return;
	}

	/** eliberea/planificarea urmatorului thread */
	RELEASE_NEXT_THREAD(scheduler, next_thread, rc);
}

/**
 * cand un thread este tot in starea ready, alege daca continua el sau este
 * inlocuit
 **/
static void scheduler_ready_state(scheduler_type *scheduler)
{
	int rc;
	so_thread_info *current_thread = scheduler->runnig_thread;
	so_thread_info *next_thread = top_pq(scheduler->ready_thread);

	if (current_thread == NULL && next_thread == NULL)
		DIE(TRUE, "Algorithm Error");


	/** cazul in care este primul fork */
	if (current_thread == NULL) {
		RELEASE_NEXT_THREAD(scheduler, next_thread, rc);
		return;
	}

	/** nu exista alt thread in sistem */
	if (next_thread == NULL)
		return;

	/** incrementarea timuplui de rulare pt un thread */
	current_thread->total_run_time++;

	/**
	 * daca thread-ul are prioritatea cea mai mare, continua executia
	 * idiferent de celelate thread-uri
	 */
	if (current_thread->priority > next_thread->priority) {
		current_thread->time_run = (current_thread->time_run + 1) %
					   scheduler->time_quantum;
		return;
	} else if (current_thread->priority == next_thread->priority) {
		current_thread->time_run++;
		/** daca nu a expirat cuanta de timp pentru proces */
		if (current_thread->time_run < scheduler->time_quantum)
			return;
	}

	/**
	 * daca exista un thread cu prioritate mai mare care a rulat mai
	 * putin sau i-a expirat quanta de timp
	 **/
	current_thread->time_run = 0;
	rc = add_priority_queue(scheduler->ready_thread, current_thread);
	DIE(rc != ADD_SUCCESS, "Add Priority Queue!");

	/** alegerea unui thread care o sa ruleze */
	pop_priority_queue(scheduler->ready_thread);
	scheduler->runnig_thread = next_thread;

	/** "eliberearea" thread-ului care o sa ruleze */
	rc = sem_post(&next_thread->wait);
	DIE(rc != 0, "Sem Post");

	rc = sem_wait(&current_thread->wait);
	DIE(rc != 0, "Sem Wait");
}

/**
 * functuia ruleaza algoritmul de planificare pentru toate situatile
 * @param scheduler panificatorul
 * @param next_state urmatoarea stare
 * @param io index-ul dispozitivului io
 */
static void scheduler_algorithm(scheduler_type *scheduler, SO_STATE next_state,
				int io)
{
	switch (next_state) {
	case WAITING:
		scheduler_wait_state(scheduler, io);
		break;
	case READY:
		scheduler_ready_state(scheduler);
		break;
	case TERMINATE:
		scheduler_terminate_state(scheduler);
		break;
	default:
		DIE(TRUE, "Algorithm Error");
	}
}

int so_init(unsigned int time_quantum, unsigned int io)
{

	int rc;

	/** daca a fost deja initializat */
	if (g_scheduler)
		return INIT_FAILURE;

	/** verificarea paramemtrilor */
	if (time_quantum <= 0)
		return INIT_FAILURE;

	if (io < 0 || io > SO_MAX_NUM_EVENTS)
		return INIT_FAILURE;

	/** parte de initializare */
	g_scheduler = calloc(1, sizeof(scheduler_type));
	DIE(g_scheduler == NULL, "malloc");

	g_scheduler->max_io = io;
	g_scheduler->time_quantum = time_quantum;

	rc = sem_init(&g_scheduler->finish, 0, 0);
	DIE(rc == -1, "Init Sem");

	g_scheduler->ready_thread = NULL;
	g_scheduler->terminate_thread = NULL;
	g_scheduler->wait_thread = calloc(io, sizeof(Linked_List));
	if (g_scheduler->wait_thread == NULL)
		return INIT_FAILURE;

	rc = create_priority_queue(&g_scheduler->ready_thread,
				   compare_priority);

	if (rc != ALOC_SUCCESS) {
		free(g_scheduler->wait_thread);
		return INIT_FAILURE;
	}

	return OPERATION_SUCCESS;
}

/**
 * rutina pe care o va executa un thread
 */
void *thread_routine(void *args)
{
	int rc;
	so_thread_info *thread_info = (so_thread_info *) args;

	/** semnmalizeaza ca este gata */
	rc = sem_post(&thread_info->create);
	DIE(rc != 0, "Sem Post");

	/** asteapta sa fie planificat */
	rc = sem_wait(&thread_info->wait);
	DIE(rc != 0, "Sem Wait");

	/** rularea handler-ului */
	thread_info->handler(thread_info->priority);

	/** semalizarea faptului ca a termint */
	scheduler_algorithm(g_scheduler, TERMINATE, -1);
	return NULL;
}

/**
 * functia care creaza un nou thread
 * @param func functia
 * @param priority prioritatea
 * @return id noului thread sau INVALID_ID
 */
tid_t so_fork(so_handler *func, unsigned int priority)
{
	tid_t id;
	so_thread_info *thread_info = NULL;
	int rc;

	/** verificarea parametrilor */
	if (func == NULL)
		goto error;

	if (priority > SO_MAX_PRIO)
		goto error;

	thread_info = calloc(1, sizeof(so_thread_info));

	if (thread_info == NULL)
		goto error;

	thread_info->priority = priority;
	thread_info->handler = func;

	rc = sem_init(&thread_info->create, 0, 0);
	if (rc != 0)
		goto error;
	rc = sem_init(&thread_info->wait, 0, 0);
	if (rc != 0)
		goto error;

	/** indica faptul ca exista cel putin un thread in sistem */
	g_scheduler->threadInSystem = TRUE;

	/**
	 * adauaga thread-ul curent in coada ready, pentru a putea sa
	 * fie planificat
	 */
	rc = add_priority_queue(g_scheduler->ready_thread, thread_info);
	DIE(rc != ADD_SUCCESS, "Add Priority Queue");

	rc = pthread_create(&id, NULL, thread_routine, thread_info);
	if (rc != 0)
		goto error;

	/** astepta ca thread-ul curent sa fie in starea ready */
	rc = sem_wait(&thread_info->create);
	if (rc != 0)
		goto error;

	thread_info->id = id;
	scheduler_algorithm(g_scheduler, READY, NULL_DEVICE);

	return id;

error:
	if (!thread_info)
		free(thread_info);

	return INVALID_TID;
}

int so_wait(unsigned int io)
{
	/** se verifica daca este valid index-ul */
	if (io >= g_scheduler->max_io || io < 0) {
		/** daca nu tot in starea ready | running */
		scheduler_algorithm(g_scheduler, READY, NULL_DEVICE);
		return FAIL_OPERATION;
	}


	scheduler_algorithm(g_scheduler, WAITING, io);

	return SUCCESS_OPERATION;
}

int so_signal(unsigned int io)
{
	int ret;

	/** verificarea parametrilor */
	if (io >= g_scheduler->max_io || io < 0)
		return FAIL_OPERATION;

	/** sunt adaugate thread-urile care asteapta */
	ret = add_list_to_priority_queue(g_scheduler->ready_thread,
					 &g_scheduler->wait_thread[io]);
	scheduler_algorithm(g_scheduler, READY, NULL_DEVICE);

	return ret;
}

void so_exec(void)
{
	scheduler_algorithm(g_scheduler, READY, NULL_DEVICE);
}

void so_end(void)
{
	int rc;
	Linked_List iterator;
	so_thread_info threadInfo;

	if (!g_scheduler)
		return;

	/**
	 * daca au fost create thread-uri se asteapta ca ele sa termine
	 */
	if (g_scheduler->threadInSystem) {
		rc = sem_wait(&g_scheduler->finish);
		DIE(rc != 0, "Sem Wait In END");
	}

	/** eliminarea planificatorului */
	iterator = g_scheduler->terminate_thread;

	for (; iterator != NULL; iterator = iterator->next) {

		threadInfo = *(so_thread_info *) iterator->data;
		rc = pthread_join(threadInfo.id, NULL);
		DIE(rc != 0, "ERROR JOIN\n");
	}

	destroy_linked_list(&g_scheduler->terminate_thread, free_thread_info);
	free(g_scheduler->wait_thread);

	destroy_priority_queue(&g_scheduler->ready_thread, free_thread_info);

	rc = sem_destroy(&g_scheduler->finish);
	DIE(rc != 0, "Sem Destroy");

	free(g_scheduler);
	g_scheduler = NULL;
}
