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
#include <pthread.h>

typedef struct List {
    size_t size;        /* Size, in bytes, that the list has available */
    size_t elem_size;   /* Size, in bytes, that each element occupies */
    i8 *head;           /* First free byte of the array */
    i8 *start;          /* First byte of the array */
} List;

typedef struct Queue {
    size_t size;        /* Size, in bytes, that the list has available */
    size_t elem_size;   /* Size, in bytes, that each element occupies */
    i8 *head;           /* The first byte of the last element in the queue */
    i8 *tail;           /* The first byte of the first element in the queue */
    i8 *start;          /* First byte of the array */
} Queue;

typedef struct HashMap {
    size_t size;        /* Total number of spaces for keys and values */
    size_t occupied;    /* Number of spaces currently occupied */
    size_t key_size;    /* Size, in bytes, that each key occupies */ 
    size_t val_size;    /* Size, in bytes, that each value occupies */
    i8 *restrict keys;  /* Byte array storing key data */
    i8 *restrict vals;  /* Byte array storing value data */
    size_t (*hash)(const void *);               /* Key hashing function */
    int (*equal)(const void *, const void *);   /* Key equality function */
} HashMap;

/*
 *  Dynamic Array List Type
 */

/* Returns a new list with the given element size, in bytes */
ATRA_API List atra_list_create(size_t elem_size);

/* Deletes the list, deallocating contained heap memory */
ATRA_API void atra_list_delete(List *restrict list);

/* Copies the data from `data` to the first empty block of the list */
ATRA_API void atra_list_add(List *restrict list, const void *restrict data);

/* Returns a pointer to the element at the given index */
ATRA_API void * atra_list_get(List *restrict list, size_t index);

/* Copies the list contents at `index` into `dest` */
ATRA_API void atra_list_copyelem(List *restrict list, size_t index, void *restrict dest);

/* Returns a pointer to the last element in the list */
ATRA_API void * atra_list_top(List *restrict list);

/* Removes the last element in the list */
ATRA_API void atra_list_pop(List *restrict list);

/* Returns the number of elements present in the list */
ATRA_API size_t atra_list_length(List *restrict list);

/* Clears the list */
ATRA_API void atra_list_clear(List *restrict list);

/* Returns 1 if the list is empty, otherwise 0 */
ATRA_API int atra_list_isempty(List *restrict list);

/* Removes element at `index` from the list */
ATRA_API void atra_list_remove(List *restrict list, size_t index);

/* Removes elements in range [start, end) from the list */
ATRA_API void atra_list_removerange(List *restrict list, size_t start, size_t end);

/* Removes and deallocates element at `index` from the list */
ATRA_API void atra_list_remove_f(List *restrict list, size_t index);

/* Removes and deallocates elements in range [start, end) from the list */
ATRA_API void atra_list_removerange_f(List *restrict list, size_t start, size_t end);

/* Moves an element from index `src` to `dest` */
ATRA_API void atra_list_move(List *restrict list, size_t src, size_t dest);

/* Deallocates all memory pointed to by elements in the list */
ATRA_API void atra_list_dealloc(List *restrict list);


/*
 *  Dynamic, Circular Queue Type
 */

/* Returns a new queue with the given element size, in bytes */
ATRA_API Queue atra_queue_create(size_t elem_size);

/* Deletes the queue, deallocating contained heap memory */
ATRA_API void atra_queue_delete(Queue *queue);

/* Adds a new element to the front of the queue */
ATRA_API void atra_queue_pushfront(Queue *queue, const void *elem);

/* Adds a new element to the back of the queue */
ATRA_API void atra_queue_pushback(Queue *queue, const void *elem);

/* Returns a pointer to the element at the head of the queue */
ATRA_API void * atra_queue_front(Queue *queue);

/* Returns a pointer to the element at the tail of the queue */
ATRA_API void * atra_queue_back(Queue *queue);

/* Returns a pointer to the element at the head of the queue, and removes it */
ATRA_API void * atra_queue_popfront(Queue *queue);

/* Returns a pointer to the element at the tail of the queue, and deletes it */
ATRA_API void * atra_queue_popback(Queue *queue);

/* Clears the queue of all elements */
ATRA_API void atra_queue_clear(Queue *queue);

/* Returns 1 if the queue is empty, otherwise returns 0 */
ATRA_API int atra_queue_isempty(Queue *queue);


/*
 *  Dynamic Hashmap
 */

/* Returns a new hashmap with the given hash and equality functions */
ATRA_API HashMap atra_hashmap_create(size_t key_size, size_t val_size,
        size_t (*hash)(const void *),
        int (*eq)(const void *, const void *));

/* Deallocates hashmap internal memory */
ATRA_API void atra_hashmap_delete(HashMap *map);

/* Adds a value to the hashmap with an associated key */
ATRA_API int atra_hashmap_add(HashMap *map, const void *key, const void *val);

/* Returns a pointer to the stored hashmap value */
ATRA_API void * atra_hashmap_get(HashMap *map, const void *key);

/* Removes the entry with the associated key */
ATRA_API void atra_hashmap_remove(HashMap *map, const void *key);

/* Removes all values and keys from the hashmap */
ATRA_API void atra_hashmap_clear(HashMap *map);

/* Returns 1 if the hashmap is empty, otherwise returns 0 */
ATRA_API int atra_hashmap_isempty(HashMap *map);

/* Built-in hash and equality functions for common key types */
ATRA_API size_t atra_hashmap_str_hash(const void *str);
ATRA_API int atra_hashmap_str_eq(const void *s1, const void *s2);


/*
 *  Dynamic Heap Memory Manager
 */

typedef struct MemPool {
    size_t free_space;      /* Total number of free bytes in the pool */
    size_t size;            /* Size, in bytes, that each chunk occupies */
    List chunks;            /* List of pointers to the start of each chunk */
    i8 *head, *tail;        /* Pointers to the first and last memory blocks */
    pthread_mutex_t lock;   /* Lock for safe multi-threaded operation */
} MemPool;

/* Initialises the engine's internal memory pool manager */
ATRA_API void atra_mempool_init(size_t chunk_size);

/* Deallocates all memory in the pool */
ATRA_API void atra_mempool_stop(void);

/* Returns a pointer to a block of allocated memory of requested size */
ATRA_API void * atra_mempool_malloc(size_t size);

/* Returns a pointer to a block of allocated, zeroed memory of requested size */
ATRA_API void * atra_mempool_calloc(size_t n, size_t size);

/* Returns a pointer to a new or resized block of memory of requested size */
ATRA_API void * atra_mempool_realloc(void *ptr, size_t size);

/* Deallocates the memory at the given address */
ATRA_API void atra_mempool_free(void *ptr);

/* Returns a pointer to the internal memory pool structure */
ATRA_API MemPool * atra_mempool_getpool(void);

#endif /* ATRA_MEMORY_H */
