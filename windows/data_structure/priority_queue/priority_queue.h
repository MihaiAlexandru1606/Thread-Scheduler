/**
 * @file : priority_queue.h
 *
 * @author : mihai
 */

#ifndef TEMA_1_PRIORITY_QUEUE_H
#define TEMA_1_PRIORITY_QUEUE_H

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "linked_list.h"

#define ALOC_SUCCESS			0
#define MEMERROR			12
#define ADD_SUCCESS			0
#define NULL_PQ				-1
#define INIT_FAIL			-3


typedef int (*Type_Funct_Cmp)(void *data1, void *data);

typedef struct {
	Linked_List head;
	Type_Funct_Cmp cmp;
} Priority_Queue;


/**
 * functia care creeaza/initializeaza coada cu prioiritate
 * @param pPriority_queue adresa unui pointer de tipul Priority_Queue
 * @param cmp functia de comparare pentru prioritati
 * @return ALOC_SUCCESS in cazul in care s-a realizat cu succes alocarea de
 *	   memorie; INIT_FAIL in caz contrar, daca este din cauza alocari de
 *	   memorie se seteaza errno la ENOMEM
 */

int
create_priority_queue(Priority_Queue **pPriority_queue, Type_Funct_Cmp cmp);

/**
 * functia insereaza in coada c perechia (data, prioritate), inserarea se face
 * ordonat (se cauat prima perechie care nu are prioritate mai mica, apoi se
 * insereaza inaintea ei)
 * @param priority_queue adresa cozi cu prioritate
 * @param data data inserat
 * @param priority prioritatea cuvantului
 * @return NULL_PQ     -> nu este alocata memoria pentru coada
 *	   NULL_WORD   -> cuvanul este null
 *	   ENOMEM      -> nu s-a putat aloca memoria
 *	   ADD_SUCCESS -> inserarea reusita
 */

int add_priority_queue(Priority_Queue *priority_queue, void *data);

/**
 * adauga o lista intrega in coada, acest lucru facndu-se eficient din punct de
 * vedere al memoriei
 * @param priority_queue adresa cozi cu prioritate
 * @param pList adresa listei
 * @return numarul de celule adaugate
 */
int add_list_to_priority_queue(Priority_Queue *priority_queue,
			       Linked_List *pList);

/**
 * functia returneaza data care are prioritate cea mai mare, daca coada nu
 * este goala
 * @param priority_queue coada cuvprioritate
 * @return NULL -> coada goala;
 *	   adresa cuvantului cu cea mai mare prioritate
 */
void *top_pq(Priority_Queue *priority_queue);

/**
 * functia elimina data care are prioritate cea mai mare, daca coada nu este
 * goala si il returneaza
 * @param pPriority_Queue coada cu prioritate
 */

void *pop_priority_queue(Priority_Queue *priority_queue);

/**
 * functia elimina toate elementele din coada si dealoca mememoria pentru
 * strcutura pentru coada
 * @param pPriority_Queue adresa unui pointer de tipul Priority_Queue
 */

void destroy_priority_queue(Priority_Queue **pPriority_Queue,
			    Type_Funct_Del del);

#endif
