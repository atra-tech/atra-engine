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

#define ATRA_LIST_INIT_SIZE         8
#define ATRA_QUEUE_INIT_SIZE        8
#define ATRA_HASHMAP_INIT_SIZE      8
#define ATRA_MEMPOOL_CHUNK_SIZE     16777216    /* 16MB */

#define ATRA_LIST_BYTE_ALIGN    8
#define ATRA_HASHMAP_MAX_LOAD   0.7


enum hashmap_free_mode {
    HASH_FREE_KEY = BIT(0),
    HASH_FREE_VAL = BIT(1)
};

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
    i8 *keys;           /* Byte array storing key data */
    i8 *vals;           /* Byte array storing value data */
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
ATRA_API void atra_hashmap_delete(HashMap *map, int free_mode);

/* Adds a value to the hashmap with an associated key */
ATRA_API int atra_hashmap_add(HashMap *map, const void *key, const void *val);

/* Returns a pointer to the stored hashmap value, returns NULL if not found */
ATRA_API void * atra_hashmap_get(HashMap *map, const void *key);

/* Returns a pointer to the key at the given index */
ATRA_API void * atra_hashmap_getkey(HashMap *map, size_t id);

/* Returns a pointer to the value at the given index */
ATRA_API void * atra_hashmap_getval(HashMap *map, size_t id);

/* Removes the entry with the associated key */
ATRA_API void atra_hashmap_remove(HashMap *map, const void *key, int free_mode);

/* Removes all values and keys from the hashmap */
ATRA_API void atra_hashmap_clear(HashMap *map, int free_mode);

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


/* 
 *  Single-Type Dynamic List
 */

#define ATRA_LIST_DECLARE(type) \
    typedef struct List_##type { \
        size_t cap; \
        size_t size; \
        type *data; \
    } List_##type; \
\
/* Returns a new, empty list */ \
List_##type atra_list_##type##_create(size_t init_cap); \
\
/* Deletes the list, deallocating contained heap memory */ \
void atra_list_##type##_delete(List_##type *restrict list); \
\
/* Copies `data` to the first empty block of the list */ \
void atra_list_##type##_add(List_##type *restrict list, const type data); \
\
/* Copies the data from `data` to the first empty block of the list */ \
void atra_list_##type##_addptr(List_##type *restrict list, type * const data); \
\
/* Returns the element at the given index */ \
type atra_list_##type##_get(List_##type *restrict list, size_t index); \
\
/* Returns a pointer to the element at the given index */ \
type * atra_list_##type##_getptr(List_##type *restrict list, size_t index); \
\
/* Returns the last element in the list */ \
type atra_list_##type##_top(List_##type *restrict list); \
\
/* Removes the last element in the list */ \
void atra_list_##type##_pop(List_##type *restrict list); \
\
/* Clears the list */ \
void atra_list_##type##_clear(List_##type *restrict list); \
\
/* Returns 1 if the list is empty, otherwise 0 */ \
int atra_list_##type##_isempty(List_##type *restrict list); \
\
/* Removes element at `index` from the list */ \
void atra_list_##type##_remove(List_##type *restrict list, size_t index); \
\
/* Removes elements in range [start, end) from the list */ \
void atra_list_##type##_removerange(List_##type *restrict list, size_t start, size_t end); \
\
/* Moves an element from index `src` to `dest` */ \
void atra_list_##type##_move(List_##type *restrict list, size_t src, size_t dest); \
\
/* Doubles the size of the list */ \
void atra_list_##type##_grow(List_##type *list); 


#define ATRA_LIST_DEFINE(type) \
\
/* Returns a new list with the given element size, in bytes */\
List_##type \
atra_list_##type##_create(size_t init_cap) \
{ \
    List_##type list; \
\
    list.size = 0; \
    list.cap = DECIDE(init_cap == 0, ATRA_LIST_INIT_SIZE, init_cap); \
    list.data = malloc(list.cap * sizeof(type)); \
\
    return list; \
} \
\
/* Deletes the list, deallocating contained heap memory */ \
void \
atra_list_##type##_delete(List_##type *list) \
{ \
    free(list->data); \
} \
\
/* Copies the data from `data` to the first empty block of the list */ \
void \
atra_list_##type##_add(List_##type *restrict list, const type data) \
{ \
    if (list->size >= list->cap) { \
        atra_list_##type##_grow(list); \
    } \
    list->data[list->size] = data; \
    list->size++; \
} \
\
/* Copies the data from `data` to the first empty block of the list */ \
void \
atra_list_##type##_addptr(List_##type *restrict list, type * const data) \
{ \
    if (list->size >= list->cap) { \
        atra_list_##type##_grow(list); \
    } \
    list->data[list->size] = *data; \
    list->size++; \
} \
\
/* Returns a pointer to the element at the given index */ \
type \
atra_list_##type##_get(List_##type *restrict list, size_t index) \
{ \
    return list->data[index]; \
} \
\
/* Returns a pointer to the element at the given index */ \
type * \
atra_list_##type##_getptr(List_##type *restrict list, size_t index) \
{ \
    return &list->data[index]; \
} \
\
/* Returns a pointer to the last element in the list */ \
type \
atra_list_##type##_top(List_##type *restrict list) \
{ \
    return list->data[list->size - 1]; \
} \
\
/* Removes the last element in the list */ \
void \
atra_list_##type##_pop(List_##type *restrict list) \
{ \
    list->size--; \
} \
\
/* Clears the list */ \
void \
atra_list_##type##_clear(List_##type *restrict list) \
{ \
    list->size = 0; \
} \
\
/* Returns 1 if the list is empty, otherwise 0 */ \
int \
atra_list_##type##_isempty(List_##type *restrict list) \
{ \
    return list->size == 0; \
} \
\
/* Removes element at `index` from the list */ \
void \
atra_list_##type##_remove(List_##type *restrict list, size_t index) \
{ \
    const size_t len = list->size - (index + 1); \
    memmove(list->data + index, list->data + index + 1, len * sizeof(type)); \
    list->size--; \
} \
\
/* Removes elements in range [start, end) from the list */ \
void \
atra_list_##type##_removerange(List_##type *restrict list, size_t start, size_t end) \
{ \
    const size_t len = list->size - end; \
    memmove(list->data + start, list->data + end, len * sizeof(type)); \
    list->size -= end - start; \
} \
\
/* Moves an element from index `src` to `dest` */ \
void \
atra_list_##type##_move(List_##type *restrict list, size_t src, size_t dest) \
{ \
    const type tmp = list->data[src]; \
    if (src < dest) { \
        memmove(list->data + src, list->data + src + 1, (dest - src) * sizeof(type)); \
        list->data[dest] = tmp; \
    } else if (src > dest) { \
        memmove(list->data + dest + 1, list->data + dest, (src - dest) * sizeof(type)); \
        list->data[dest] = tmp; \
    } \
} \
\
/* Doubles the size of the list */ \
void \
atra_list_##type##_grow(List_##type *list) \
{ \
    list->cap *= 2; \
    list->data = realloc(list->data, list->cap * sizeof(type)); \
} \


ATRA_LIST_DECLARE(char)
ATRA_LIST_DECLARE(short)
ATRA_LIST_DECLARE(int)
ATRA_LIST_DECLARE(long)

ATRA_LIST_DECLARE(uchar)
ATRA_LIST_DECLARE(ushort)
ATRA_LIST_DECLARE(uint)
ATRA_LIST_DECLARE(ulong)

ATRA_LIST_DECLARE(i8)
ATRA_LIST_DECLARE(u8)
ATRA_LIST_DECLARE(i16)
ATRA_LIST_DECLARE(u16)
ATRA_LIST_DECLARE(i32)
ATRA_LIST_DECLARE(u32)
ATRA_LIST_DECLARE(i64)
ATRA_LIST_DECLARE(u64)

ATRA_LIST_DECLARE(float)
ATRA_LIST_DECLARE(double)
ATRA_LIST_DECLARE(f32)
ATRA_LIST_DECLARE(f64)

ATRA_LIST_DECLARE(size_t)
ATRA_LIST_DECLARE(uintptr_t)


#endif /* ATRA_MEMORY_H */
