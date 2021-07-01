/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  memory.c - A memory management system for fast dynamic memory.
 *             Also provides useful memory debugging tools.
 *             Implements general-purpose heap memory pool/arena, and a
 *             dynamic data list.
 *
 *  */

#include "atra.h"

#include <string.h>


/*  --------------------------------------------------------------------------
 *      DYNAMIC ARRAY LIST TYPE
 *  --------------------------------------------------------------------------
 */

/* Doubles the size of the list */
ATRA_INLINE
void
atra_list_grow(List *list)
{
    i8 *newmem = realloc(list->start, list->size * 2);

    atra_util_checkalloc(newmem);
    list->head = newmem;
    list->size *= 2;
}

/* Returns a new list with the given size, in bytes */
ATRA_API
List
atra_list_create(size_t elem_size)
{
    List list;

    list.size = ATRA_LIST_INIT_CAP;
    list.elem_size = elem_size;

    if (elem_size == 0) {
        atra_log_print(LOG_ERROR, "list: Illegal list size: " \
                "List cannot have elements of size 0");
        list.start = NULL;
    } else if (elem_size >= SIZE_MAX / ATRA_LIST_INIT_CAP) {
        atra_log_print(LOG_ERROR, "list: Illegal list size: " \
                "Element exceeds maximum size of %d bytes",
                SIZE_MAX / ATRA_LIST_INIT_CAP);
        list.start = NULL;
    } else {
        list.start = malloc(ATRA_LIST_INIT_CAP * elem_size);
    }
    list.head = list.start;
    
    return list;
}

/* Deletes the list, deallocating contained heap memory */
ATRA_API
void
atra_list_delete(List *list)
{
    free(list->start);
}

/* Copies the data from `data` to the first empty block of the list */
ATRA_API
void
atra_list_add(List *list, const void *data)
{
    memcpy(list->head, data, list->elem_size);
    list->head += list->elem_size;

    if (list->head >= list->start + list->size) {
        atra_list_grow(list);
    }
}

/* Returns a pointer to the element at the given index */
ATRA_API
void *
atra_list_get(List *list, size_t index)
{
    return list->start + (index * list->elem_size);
}

/* Copies the list contents at `index` into `dest` */
ATRA_API
void
atra_list_copy(List *list, size_t index, void *dest)
{
    memcpy(dest, list->start + (index * list->elem_size), list->elem_size);
}

/* Returns a pointer to the last element in the list */
ATRA_API
void *
atra_list_top(List *list)
{
    if (list->head == list->start) {
        return NULL;
    }
    
    return list->head - list->elem_size;
}

/* Removes the last element in the list */
ATRA_API
void
atra_list_pop(List *list)
{
    if (list->head == list->start) {
        return;
    }
    list->head -= list->elem_size;
}

/* Clears the list */
ATRA_API
void
atra_list_clear(List *list)
{
    list->head = list->start;
}

/* Returns 1 if the list is empty, otherwise 0 */
ATRA_API
inline int
atra_list_isempty(List *list)
{
    return (list->head == list->start);
}

/* Removes element at `index` from the list */
ATRA_API
void
atra_list_remove(List *list, size_t index)
{
    i8 *dest = list->start + (index * list->elem_size);
    const i8 *src = dest + list->elem_size;
    size_t size = list->head - src;

    if (size > 0) {
        memmove(dest, src, list->head - src);
    }
    list->head -= list->elem_size;
}

/* Removes elements in range [start, end) from the list */
ATRA_API
void
atra_list_removerange(List *list, size_t start, size_t end)
{
    i8 *dest = list->start + (start * list->elem_size);
    const i8 *src = dest + ((end - start) * list->elem_size);
    size_t size = list->head - src;

    if (size > 0) {
        memmove(dest, src, list->head - src);
    }
    list->head -= list->elem_size * (end - start);
}

/* Removes and deallocates element at `index` from the list */
ATRA_API
void
atra_list_remove_f(List *list, size_t index)
{
    i8 *dest = list->start + (index * list->elem_size);
    const i8 *src = dest + list->elem_size;
    size_t size = list->head - src;

    free(*(void **) dest);

    if (size > 0) {
        memmove(dest, src, list->head - src);
    }
    list->head -= list->elem_size;
}

/* Removes and deallocates elements in range [start, end) from the list */
ATRA_API
void
atra_list_removerange_f(List *list, size_t start, size_t end)
{
    i8 *i;
    i8 *dest = list->start + (start * list->elem_size);
    const i8 *src = dest + ((end - start) * list->elem_size);
    size_t size = list->head - src;

    for (i = dest; i < src; i += list->elem_size) {
        free(*(void **) i);
    }

    if (size > 0) {
        memmove(dest, src, list->head - src);
    }
    list->head -= list->elem_size * (end - start);
}

/* Moves an element from index `src` to `dest` */
ATRA_API
void
atra_list_move(List *list, size_t src, size_t dest)
{
    i8 *s = list->start + src * list->elem_size;
    i8 *d = list->start + dest * list->elem_size;
    
    if (list->head + list->elem_size >= list->start + list->size) {
        atra_list_grow(list);
    }
    
    if (s < d) {
        memcpy(list->head, s, list->elem_size);
        memmove(s, s + list->elem_size, d - (s + list->elem_size));
        memcpy(d, list->head, list->elem_size);
    } else if (s > d) {
        memcpy(list->head, d, list->elem_size);
        memmove(d, d + list->elem_size, s - (d + list->elem_size));
        memcpy(s, list->head, list->elem_size);
    }
}

/* Deallocates all elements in the list */
ATRA_API
void
atra_list_dealloc(List *list)
{
    i8 *i;

    for (i = list->start; i < list->head; i += list->elem_size) {
        free(*(void **) i);
    }
}
