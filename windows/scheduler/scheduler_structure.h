/**
 * @author : mihai
 *
 * @file : scheduler_structure.h
 */

#ifndef SO____TEMA_4___PLANIFICATOR_DE_THREADURI_SCHEDULER_STRUCTURE_H
#define SO____TEMA_4___PLANIFICATOR_DE_THREADURI_SCHEDULER_STRUCTURE_H

#include "priority_queue.h"
#include "so_scheduler.h"

#define INIT_FAILURE		-1
#define OPERATION_SUCCESS	0

typedef enum {
	READY = 1,
	WAITING = 2,
	TERMINATE = 3
} SO_STATE;

typedef struct {
	/** id -ul thread-ului */
	tid_t id;
	/** handelul thread-ului */
	HANDLE hThread;
	/** functia rulata de respectvul thread */
	so_handler *handler;
	/** prioritatea thredului */
	int priority;
	/** timplul de rulare de cand thread-ul a fost planificat */
	int time_run;
	/** timpul total care a rulat thread-ul de cand a fost creat */
	int total_run_time;
	/** pentru a bloca thead-ul curent pana cand este planificat */
	HANDLE wait;
	/**
	 * pentru a bloca procesul parinte pana cand thread-ul creat este in
	 * starea ready
	 */
	HANDLE create;
} so_thread_info;

typedef struct {
	/** coada cu thread-urile "active" din sistem */
	Priority_Queue *ready_thread;
	/**
	 * un array de liste, fiecare elemet reprezinta o lista de thread-uri
	 * care asteapta dupa dispozitivul cu index-ul egal cu elementul
	 */
	Linked_List *wait_thread;
	/** lista de procese care au terminat */
	Linked_List terminate_thread;
	/** thread-ul care ruleza pe procesor */
	so_thread_info *runnig_thread;
	/** indica dac thread-urile au terminat de execut task-urile */
	HANDLE finish;
	/** adaca a fost inistializat schedeler */
	int init;
	/** daca o fost creat cel putin un thread */
	BOOL threadInSystem;
	/** numarul maxim de dispozitive io */
	int max_io;
	/** quanta de timp pentru  un thread */
	int time_quantum;

} scheduler_type;

#define FAIL_OPERATION		-1
#define SUCCESS_OPERATION	0
#define NULL_DEVICE		-1

#define RELEASE_NEXT_THREAD(scheduler, next_thread, rc)			\
	do {								\
		/** alegerea unui thread care o sa ruleze */		\
		pop_priority_queue(scheduler->ready_thread);		\
		scheduler->runnig_thread = next_thread;			\
		/** "eliberearea" thread-ului care o sa ruleze */	\
		rc = ReleaseSemaphore(next_thread->wait, 1, NULL);	\
		DIE(rc == FALSE, "ReleaseSemaphore");			\
	} while (0)

#endif /**SO____TEMA_4___PLANIFICATOR_DE_THREADURI_SCHEDULER_STRUCTURE_H */