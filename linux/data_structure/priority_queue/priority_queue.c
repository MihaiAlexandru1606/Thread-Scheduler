/**
 * @file : priority_queue.c
 *
 * @author : mihai
 */
#include <stdio.h>
#include <errno.h>

#include "priority_queue.h"


int create_priority_queue(Priority_Queue **pPriority_queue, Type_Funct_Cmp cmp)
{
	if (pPriority_queue == NULL)
		return INIT_FAIL;

	if (cmp == NULL)
		return INIT_FAIL;

	*pPriority_queue = calloc(1, sizeof(Priority_Queue));
	if (*pPriority_queue == NULL) {
		errno = ENOMEM;
		return MEMERROR;
	}

	(*pPriority_queue)->head = NULL;
	(*pPriority_queue)->cmp = cmp;

	return ALOC_SUCCESS;
}

int add_priority_queue(Priority_Queue *priority_queue, void *data)
{
	int rez = ADD_SUCCESS;
	Cell *new_node;
	Cell **iterator;

	if (priority_queue == NULL) {
		rez = NULL_PQ;
		goto exit;
	}

	if (data == NULL) {
		rez = NULL_DATA;
		goto exit;
	}


	iterator = &(*priority_queue).head;
	while (*iterator != NULL) {
		if (priority_queue->cmp((*iterator)->data, data) < 0)
			break;

		iterator = &(*iterator)->next;
	}

	new_node = calloc(1, sizeof(Cell));
	if (new_node == NULL) {
		rez = ENOMEM;
		goto exit;
	}

	new_node->data = data;
	new_node->next = *iterator;
	*iterator = new_node;

exit:
	return rez;
}

int add_list_to_priority_queue(Priority_Queue *priority_queue,
			       Linked_List *pList)
{
	Cell *current;
	Cell *iterator_list = *pList;
	Cell **iterator_pq;
	int number_cell = 0;

	while (iterator_list) {
		current = iterator_list;
		iterator_list = iterator_list->next;

		iterator_pq = &(*priority_queue).head;
		while (*iterator_pq != NULL) {
			if (priority_queue->cmp((*iterator_pq)->data,
						current->data) < 0)
				break;

			iterator_pq = &(*iterator_pq)->next;
		}

		current->next = *iterator_pq;
		*iterator_pq = current;

		number_cell++;
	}

	*pList = NULL;

	return number_cell;
}

void *top_pq(Priority_Queue *priority_queue)
{
	if (priority_queue->head == NULL)
		return NULL;

	return priority_queue->head->data;
}

void *pop_priority_queue(Priority_Queue *priority_queue)
{
	return pop_linked_list(&priority_queue->head);
}

void destroy_priority_queue(Priority_Queue **pPriority_Queue,
			    Type_Funct_Del del)
{
	destroy_linked_list(&(*pPriority_Queue)->head, del);

	free(*pPriority_Queue);
	*pPriority_Queue = NULL;
}

