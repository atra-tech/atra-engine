/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  memory.h - A memory management system for fast dynamic memory.
 *             Also provides useful memory debugging tools.
 *             Implements general-purpose heap memory pool/arena, and a
 *             dynamic data list.
 *
 *  */

#ifndef ATRA_MEMORY_H
#define ATRA_MEMORY_H

#include "common.h"

#define ATRA_LIST_INIT_CAP 64

typedef struct List {
    size_t size;
    size_t elem_size;
    i8 *head;
    i8 *start;
} List;

/* Returns a new list with the given size, in bytes */
ATRA_API List atra_list_create(size_t elem_size);

/* Deletes the list, deallocating contained heap memory */
ATRA_API void atra_list_delete(List *list);

/* Copies the data from `data` to the first empty block of the list */
ATRA_API void atra_list_add(List *list, const void *data);

/* Returns a pointer to the element at the given index */
ATRA_API void * atra_list_get(List *list, size_t index);

/* Copies the list contents at `index` into `dest` */
ATRA_API void atra_list_copy(List *list, size_t index, void *dest);

/* Returns a pointer to the last element in the list */
ATRA_API void * atra_list_top(List *list);

/* Removes the last element in the list */
ATRA_API void atra_list_pop(List *list);

/* Clears the list */
ATRA_API void atra_list_clear(List *list);

/* Returns 1 if the list is empty, otherwise 0 */
ATRA_API inline int atra_list_isempty(List *list);

/* Removes element at `index` from the list */
ATRA_API void atra_list_remove(List *list, size_t index);

/* Removes elements in range [start, end) from the list */
ATRA_API void atra_list_removerange(List *list, size_t start, size_t end);

/* Removes and deallocates element at `index` from the list */
ATRA_API void atra_list_remove_f(List *list, size_t index);

/* Removes and deallocates elements in range [start, end) from the list */
ATRA_API void atra_list_removerange_f(List *list, size_t start, size_t end);

/* Moves an element from index `src` to `dest` */
ATRA_API void atra_list_move(List *list, size_t src, size_t dest);

/* Deallocates all elements in the list */
ATRA_API void atra_list_dealloc(List *list);

#endif /* ATRA_MEMORY_H */
