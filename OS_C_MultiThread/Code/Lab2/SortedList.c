#define _GNU_SOURCE
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "SortedList.h"



void SortedList_insert(SortedList_t *list, SortedListElement_t *element){
	//List and new element must exist!
	if(list==NULL || element==NULL)
		return;
	SortedList_t *prev = list;	
	if(prev->key !=NULL)
		return;
	SortedList_t *curr = prev->next;
	//Exit, if list isn't a head.
	while(curr != list){
		if(strcmp(curr->key, element->key) < 0)
			break;
		prev = curr;
		curr = curr->next;
	}
	
	if(opt_yield & INSERT_YIELD)
		pthread_yield();

	element->next = prev->next;
	element->prev = prev;
	prev->next = element;
	element->next->prev = element;
}


int SortedList_delete(SortedListElement_t *element){
	//Can't remove what doesn't exist and the header
	if(element == NULL || element->key ==NULL)
		return 1;
	//If not consistently linked => corrupted. Probs due to race condition
	if(element->prev->next != element || element->next->prev != element)
		return 1;

	if(opt_yield & DELETE_YIELD)
		pthread_yield();
	element->prev->next = element->next;
	element->next->prev = element->prev;
	return 0;
}


SortedListElement_t* SortedList_lookup(SortedList_t* list, const char* key){
	//Gotta have a list and also provide the head
	if (list == NULL || list->key != NULL)
		return NULL;
	SortedListElement_t* curr = list->next;
	while(curr != list){
		if(!strcmp(curr->key, key))
			return curr;
		if(opt_yield & LOOKUP_YIELD)
			pthread_yield();
		curr = curr->next;
	}
	return NULL;
}


int SortedList_length(SortedList_t * list){
	//List gotta exist and give us head
	if(list == NULL || list->key !=NULL)
		return -1;
	int length = 0;
	SortedList_t* curr = list->next;
	while(curr != list){
		//If any point, the link isn't consistent => corrupted	
		if(curr->prev->next != curr || curr->next->prev !=curr)
			return -1;
		length++;
		if(opt_yield & LOOKUP_YIELD)
			pthread_yield();
		curr = curr->next;
	}
	return length;
}




