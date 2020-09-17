#ifndef SO____TEMA_4___PLANIFICATOR_DE_THREADURI_LINKED_LIST_H
#define SO____TEMA_4___PLANIFICATOR_DE_THREADURI_LINKED_LIST_H

#include <stdlib.h>
#include <errno.h>

#define ALOC_SUCCESS		0

#define ADD_SUCCESS		0
#define NULL_DATA		-2
#define ADD_FAILURE		-3

typedef void (*Type_Funct_Del)(void *data);

typedef struct cell {
	void *data;
	struct cell *next;
} Cell, *Linked_List;

/**
 * adauda intr-o lista
 */
int add_linked_list(Linked_List *pLinked_list, void *data);

/**
 * functia scoate primul elemnt, daca exista
 * @param pLinked_list adresa listei
 * @return primul elemnt sau NULL daca nu exista
 */
void *pop_linked_list(Linked_List *pLinked_list);

/**
 * distruge o lista
 * @param pLinked_list adresa listei
 * @param del functia care stie sa elimine, elementele din lista, ea fiind una
 * generica
 */
void destroy_linked_list(Linked_List *pLinked_list, Type_Funct_Del del);

#endif /**SO____TEMA_4___PLANIFICATOR_DE_THREADURI_LINKED_LIST_H */
