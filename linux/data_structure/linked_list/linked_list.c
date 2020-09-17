#include "linked_list.h"

int add_linked_list(Linked_List *pLinked_list, void *data)
{
	Cell *new_cell;

	if (data == NULL)
		return NULL_DATA;

	new_cell = calloc(1, sizeof(Cell));
	if (new_cell == NULL) {
		errno = ENOMEM;
		return ADD_FAILURE;
	}

	new_cell->data = data;
	new_cell->next = *pLinked_list;

	*pLinked_list = new_cell;

	return ADD_SUCCESS;
}

void *pop_linked_list(Linked_List *pLinked_list)
{
	Cell *aux;
	void *data;

	if (pLinked_list == NULL)
		return NULL;

	if (*pLinked_list == NULL)
		return NULL;

	aux = *pLinked_list;
	data = aux->data;
	*pLinked_list = aux->next;

	free(aux);

	return data;
}


void destroy_linked_list(Linked_List *pLinked_list, Type_Funct_Del del)
{
	Cell *aux;

	while (*pLinked_list != NULL) {
		aux = *pLinked_list;
		*pLinked_list = aux->next;

		del(aux->data);
		free(aux);
	}

	*pLinked_list = NULL;
}

