/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  memory.c - A memory management system for fast dynamic memory.
 *             Also provides useful memory debugging tools.
 *             Implements general-purpose heap memory pool/arena, dynamic
 *             data list, and .
 *
 *  */

#include "atra.h"

#include <string.h>
#include <pthread.h>

#define ATRA_LIST_INIT_SIZE         8
#define ATRA_QUEUE_INIT_SIZE        8
#define ATRA_HASHMAP_INIT_SIZE      8
#define ATRA_MEMPOOL_CHUNK_SIZE     16777216    /* 16MB */

#define ATRA_LIST_BYTE_ALIGN    8
#define ATRA_HASHMAP_MAX_LOAD   0.7


ATRA_INLINE void atra_list_grow(List *list);
ATRA_INLINE void atra_queue_grow(Queue *queue);
ATRA_INLINE int atra_queue_isnearfull(Queue *queue);

/*
 *  Dynamic Array List Type
 */

/* Returns a new list with the given element size, in bytes */
ATRA_API
List
atra_list_create(size_t elem_size)
{
    List list;

    list.size = ATRA_LIST_INIT_SIZE * elem_size;
    list.elem_size = elem_size;

    if (elem_size == 0) {
        atra_log_print(LOG_ERROR, "list: illegal list size: " \
                "list cannot have elements of size 0");
        list.start = NULL;
    } else if (elem_size >= SIZE_MAX / ATRA_LIST_INIT_SIZE) {
        atra_log_print(LOG_ERROR, "list: illegal list size: " \
                "element exceeds maximum size of %lu bytes",
                SIZE_MAX / ATRA_LIST_INIT_SIZE);
        list.start = NULL;
    } else {
        list.start = malloc(ATRA_LIST_INIT_SIZE * elem_size);
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

/* Copies the data from `data` to the first empty block of the list.
 * The memory pointed to by `data` must not overlap `list` */
ATRA_API
void
atra_list_add(List *restrict list, const void *restrict data)
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
atra_list_get(List *restrict list, size_t index)
{
    return list->start + (index * list->elem_size);
}

/* Copies the list contents at `index` into `dest` */
ATRA_API
void
atra_list_copyelem(List *restrict list, size_t index, void *restrict dest)
{
    memcpy(dest, list->start + (index * list->elem_size), list->elem_size);
}

/* Returns a pointer to the last element in the list */
ATRA_API
void *
atra_list_top(List *restrict list)
{
    if (list->head == list->start) {
        return NULL;
    }
    
    return list->head - list->elem_size;
}

/* Removes the last element in the list */
ATRA_API
void
atra_list_pop(List *restrict list)
{
    if (list->head == list->start) {
        return;
    }
    list->head -= list->elem_size;
}

/* Returns the number of elements present in the list */
ATRA_API
size_t
atra_list_length(List *restrict list)
{
    return (size_t)(list->head - list->start) / list->elem_size;
}

/* Clears the list */
ATRA_API
void
atra_list_clear(List *restrict list)
{
    list->head = list->start;
}

/* Returns 1 if the list is empty, otherwise 0 */
ATRA_API
int
atra_list_isempty(List *restrict list)
{
    return (list->head == list->start);
}

/* Removes element at `index` from the list */
ATRA_API
void
atra_list_remove(List *restrict list, size_t index)
{
    i8 *dest = list->start + (index * list->elem_size);
    i8 *src = dest + list->elem_size;
    const size_t size = list->head - src;

    if (size > 0) {
        memmove(dest, src, size);
    }
    list->head -= list->elem_size;
}

/* Removes elements in range [start, end) from the list */
ATRA_API
void
atra_list_removerange(List *restrict list, size_t start, size_t end)
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
atra_list_remove_f(List *restrict list, size_t index)
{
    i8 *dest = list->start + (index * list->elem_size);
    const i8 *src = dest + list->elem_size;
    const size_t size = list->head - src;

    free(*(void **) dest);

    if (size > 0) {
        memmove(dest, src, list->head - src);
    }
    list->head -= list->elem_size;
}

/* Removes and deallocates elements in range [start, end) from the list */
ATRA_API
void
atra_list_removerange_f(List *restrict list, size_t start, size_t end)
{
    i8 *i;
    i8 *dest = list->start + (start * list->elem_size);
    const i8 *src = dest + ((end - start) * list->elem_size);
    const size_t size = list->head - src;

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
atra_list_move(List *restrict list, size_t src, size_t dest)
{
    if (src == dest) {
        return;
    }
    i8 *const s = list->start + src * list->elem_size;
    i8 *const d = list->start + dest * list->elem_size;
    
    if (list->head + list->elem_size >= list->start + list->size) {
        atra_list_grow(list);
    }

    if (s < d) {
        memcpy(list->head, s, list->elem_size);
        memmove(s, s + list->elem_size, d - s);
        memcpy(d, list->head, list->elem_size);
    } else if (s > d) {
        memcpy(list->head, s, list->elem_size);
        memmove(d + list->elem_size, d, s - d);
        memcpy(d, list->head, list->elem_size);
    }
}

/* Deallocates all memory pointed to by elements in the list */
ATRA_API
void
atra_list_dealloc(List *restrict list)
{
    i8 *i;
    void *elem;

    for (i = list->start; i < list->head; i += list->elem_size) {
        elem = *(void **)i;
        free(elem);
    }
}

/* Doubles the size of the list */
ATRA_INLINE
void
atra_list_grow(List *list)
{
    i8 *newmem = realloc(list->start, list->size * 2);
    const size_t length = list->head - list->start;

    atra_util_checkalloc(newmem);
    list->start = newmem;
    list->head = list->start + length;
    list->size *= 2;
}


/*
 *  Dynamic, Circular Queue Type
 */

/* Returns a new queue with the given element size, in bytes */
ATRA_API
Queue
atra_queue_create(size_t elem_size)
{
    Queue queue;

    queue.size = ATRA_QUEUE_INIT_SIZE * elem_size;
    queue.elem_size = elem_size;

    if (elem_size == 0) {
        atra_log_print(LOG_ERROR, "queue: queue element size set to 0");
        queue.start = NULL;
    } else if (elem_size >= SIZE_MAX / ATRA_QUEUE_INIT_SIZE) {
        atra_log_print(LOG_ERROR, "queue: element size too large");
        queue.start = NULL;
    } else {
        queue.start = malloc(ATRA_QUEUE_INIT_SIZE * elem_size);
    }
    queue.head = queue.start;
    queue.tail = queue.start;

    return queue;
}

/* Deletes the queue, deallocating contained heap memory */
ATRA_API
void
atra_queue_delete(Queue *queue)
{
    queue->size = 0;
    free(queue->start);
}

/* Adds a new element to the front of the queue */
ATRA_API
void
atra_queue_pushfront(Queue *queue, const void *elem)
{   
    i8 *end;
    size_t pos_new;

    if (atra_queue_isnearfull(queue)) {
        atra_queue_grow(queue);
    }
    end = queue->start + queue->size - queue->elem_size;
    pos_new = (end - queue->head + queue->elem_size) % queue->size;

    queue->head = end - pos_new;
    memcpy(queue->head, elem, queue->elem_size);
}

/* Adds a new element to the back of the queue */
ATRA_API
void
atra_queue_pushback(Queue *queue, const void *elem)
{
    size_t pos_next;

    if (atra_queue_isnearfull(queue)) {
        atra_queue_grow(queue);
    }
    pos_next = (queue->tail - queue->start + queue->elem_size) % queue->size;

    memcpy(queue->tail, elem, queue->elem_size);
    queue->tail = queue->start + pos_next;
}

/* Returns a pointer to the element at the head of the queue */
ATRA_API
void *
atra_queue_front(Queue *queue)
{
    return queue->head;
}

/* Returns a pointer to the element at the tail of the queue */
ATRA_API
void *
atra_queue_back(Queue *queue)
{
    i8 *end = queue->start + queue->size - queue->elem_size;
    const size_t pos_new = (end - queue->tail + queue->elem_size) % queue->size;

    return end - pos_new;
}

/* Returns a pointer to the element at the head of the queue, and removes it */
ATRA_API
void *
atra_queue_popfront(Queue *queue)
{
    void *elem = queue->head;
    const size_t pos_curr = queue->head - queue->start;
    const size_t pos_next = (pos_curr + queue->elem_size) % queue->size;

    queue->head = queue->start + pos_next;

    return elem;
}

/* Returns a pointer to the element at the tail of the queue, and deletes it */
ATRA_API
void *
atra_queue_popback(Queue *queue)
{
    i8 *end = queue->start + queue->size - queue->elem_size;
    const size_t pos_curr = end - queue->tail;
    const size_t pos_new = (pos_curr + queue->elem_size) % queue->size;

    queue->tail = end - pos_new;

    return queue->tail;
}

/* Clears the queue of all elements */
ATRA_API
void
atra_queue_clear(Queue *queue)
{
    queue->tail = queue->head = queue->start;
}

/* Returns 1 if the queue is empty, otherwise returns 0 */
ATRA_API
int
atra_queue_isempty(Queue *queue)
{
    return queue->head == queue->tail;
}

/* Doubles the size of the queue */
ATRA_INLINE
void
atra_queue_grow(Queue *queue)
{
    const i8 *const end = queue->start + queue->size;
    const size_t size_head = end - queue->head;
    const size_t size_tail = queue->tail - queue->start;
    i8 *newmem;

    newmem = malloc(queue->size * 2);

    if (queue->tail < queue->head) {
        memcpy(newmem, queue->head, size_head);
        memcpy(newmem + size_head, queue->start, size_tail);
    } else {
        memcpy(newmem, queue->head, queue->tail - queue->head);
    }
    free(queue->start);
    queue->start = newmem;
    queue->head = queue->start;
    queue->tail = queue->start + size_head + size_tail;
    queue->size *= 2;
}

ATRA_INLINE
int
atra_queue_isnearfull(Queue *queue)
{
    const size_t pos_curr = queue->tail - queue->start;
    const size_t pos_next = (pos_curr + queue->elem_size) % queue->size;

    return queue->start + pos_next == queue->head;
}


/*
 *  Dynamic Hashmap
 */

ATRA_INLINE void atra_hashmap_grow(HashMap *map);
ATRA_INLINE int atra_hashmap_keyisempty(HashMap *map, size_t index);

/* Returns a new hashmap with the given hash and equality functions */
ATRA_API
HashMap
atra_hashmap_create(size_t key_size, size_t val_size,
        size_t (*hash)(const void *),
        int (*eq)(const void *, const void *))
{
    HashMap map;

    map.size = ATRA_HASHMAP_INIT_SIZE;
    map.occupied = 0;
    map.key_size = key_size;
    map.val_size = val_size;
    map.keys = malloc(map.size * key_size);
    map.vals = malloc(map.size * val_size);
    map.hash = hash;
    map.equal = eq;

    return map;
}

/* Deallocates hashmap internal memory */
ATRA_API
void
atra_hashmap_delete(HashMap *map)
{
    free(map->keys);
    free(map->vals);
}

/* Adds a value to the hashmap with an associated key
 * Returns 1 on success, otherwise returns 0 */
ATRA_API
int
atra_hashmap_add(HashMap *map, const void *key, const void *val)
{
    size_t i, hash;
    double loadfactor = (map->occupied + 1) / (double)map->size;
    i8 *key_dest, *val_dest;

    if (loadfactor > ATRA_HASHMAP_MAX_LOAD) {
        atra_hashmap_grow(map);
    }
    hash = map->hash(key) % map->size;

    for (i = hash; i < map->size; i = (i + 1) % map->size) {
        key_dest = map->keys + i * map->key_size;
        if (atra_hashmap_keyisempty(map, i)) {
            val_dest = map->vals + i * map->val_size;
            memcpy(key_dest, key, map->key_size);
            memcpy(val_dest, val, map->val_size);
            break;
        } else if (map->equal(key, key_dest)) {
            atra_log_print(LOG_WARN, "memory: unable to add element to " \
                    "hashmap: duplicate key found");
            return 0;
        }
    }
    map->occupied++;

    return 1;
}

/* Returns a pointer to the stored hashmap value */
ATRA_API
void *
atra_hashmap_get(HashMap *map, const void *key)
{
    size_t i;
    size_t hash = map->hash(key) % map->size;

    for (i = hash; i < map->size; i = (i + 1) % map->size) {
        if (map->equal(key, map->keys + i * map->key_size)) {
            return map->vals + i * map->val_size;
        } else if (atra_hashmap_keyisempty(map, i)) {
            return NULL;
        } 
    }

    return NULL;
}

/* Removes the entry with the associated key */
ATRA_API
void
atra_hashmap_remove(HashMap *map, const void *key)
{
    size_t i;
    size_t hash = map->hash(key) % map->size;

    for (i = hash; i < map->size; i = (i + 1) % map->size) {
        if (map->equal(key, map->keys + i * map->key_size)) {
            memset(map->keys + i * map->key_size, 0, map->key_size);
            return;
        } else if (atra_hashmap_keyisempty(map, i)) {
            atra_log_print(LOG_WARN, "memory: unable to remove hashmap " \
                    "element: key does not exist");
            return; 
        }
    }
}

/* Removes all values and keys from the hashmap */
ATRA_API
void
atra_hashmap_clear(HashMap *map)
{
    memset(map->keys, 0, map->key_size * map->size);
    map->occupied = 0;
}

/* Returns 1 if the hashmap is empty, otherwise returns 0 */
ATRA_API
int
atra_hashmap_isempty(HashMap *map)
{
    return map->occupied == 0;
}

ATRA_INLINE
void
atra_hashmap_grow(HashMap *map)
{
    i8 *keys = map->keys;
    i8 *vals = map->vals;
    size_t size = map->size;
    size_t i;

    size = map->size;
    map->size *= 2;
    map->keys = calloc(map->size, map->key_size);
    map->vals = malloc(map->size * map->val_size);

    for (i = 0; i < size; i++) {
        if (atra_hashmap_keyisempty(map, i) == 0) {
            atra_hashmap_add(map,
                    keys + i * map->key_size,
                    vals + i * map->val_size);
        }
    }
    free(keys);
    free(vals);
}

ATRA_INLINE
int
atra_hashmap_keyisempty(HashMap *map, size_t index)
{
    const static char nulls[64] = {0};
    size_t size, i;
    int found;
    i8 *key_pos;

    i = 0;
    found = 0;
    size = map->size;
    key_pos = map->keys + index * map->key_size; 

    while (size > 64) {
        found += memcmp(key_pos + i, nulls, map->key_size) != 0;
        i += 64;
        size -= 64;
    }
    found += memcmp(key_pos + i, nulls, size) != 0;

    return found == 0;
}

ATRA_API
size_t
atra_hashmap_str_hash(const void *str)
{
    const i8 *pstr = str;
    u64 hash = 0;
    int c;

    for (c = *pstr; c != '\0'; c = *(++pstr)) {
        hash = c + (hash << 6) + (hash << 16) - hash;
    }

    return hash;
}

ATRA_API
int
atra_hashmap_str_eq(const void *s1, const void *s2)
{
    return strcmp(s1, s2) == 0;
}


/*
 *  Dynamic Heap Memory Manager
 */

#define writeheader(block, size, active) \
    *(size_t *)block = size; \
    *(i32 *)(block + sizeof(size_t)) = active

#define writefooter(block, size, active) \
    *(size_t *)(block + GUARD_SIZE + size) = size; \
    *(i32 *)(block + GUARD_SIZE + size + sizeof(size_t)) = active

#define writebody(block, next, prev) \
    *(i8 **)(block + GUARD_SIZE) = next; \
    *(i8 **)(block + GUARD_SIZE + sizeof(i8 *)) = prev

#define getnext(block) (*(i8 **)(block + GUARD_SIZE))
#define getprev(block) (*(i8 **)(block + GUARD_SIZE + sizeof(i8 *)))
#define setnext(block, next) *(i8 **)(block + GUARD_SIZE) = next
#define setprev(block, prev) *(i8 **)(block + GUARD_SIZE + sizeof(i8 *)) = prev
#define getsize(block) (*(size_t *)block)
#define addguardsize(size) (size + GUARD_SIZE * 2)
#define subguardsize(size) (size - GUARD_SIZE * 2)

static const size_t GUARD_SIZE = sizeof(size_t) + sizeof(i32);

static MemPool pool;

/* Initialises the engine's internal memory pool manager */
ATRA_API
void
atra_mempool_init(size_t chunk_size)
{
    i8 *start;

    if (chunk_size <= 2 * (GUARD_SIZE + sizeof(i8 *))) {
        atra_log_print(LOG_WARN, "memory: pool block size is less than " \
                "minimum size of %lu bytes: defaulting to 16MB block size",
                2 * (GUARD_SIZE + sizeof(i8 *)));
        chunk_size = ATRA_MEMPOOL_CHUNK_SIZE;
    } else if (chunk_size > 512 * 1024 * 1024) { /* 512MB */
        atra_log_print(LOG_WARN, "memory: pool block size is greater than " \
                "maximum size of %luMB: defaulting to 16MB block size",
                512);
        chunk_size = ATRA_MEMPOOL_CHUNK_SIZE;

    }
    pool.size = chunk_size;
    pool.free_space = subguardsize(pool.size);
    
    start = malloc(chunk_size);
    pool.chunks = atra_list_create(sizeof(i8 *));
    atra_list_add(&pool.chunks, &start);
    pool.head = start;
    pool.tail = start;

    writeheader(start, pool.free_space, 0);
    writebody(start, NULL, NULL);
    writefooter(start, pool.free_space, 0);
}

ATRA_API
void
atra_mempool_stop(void)
{
    size_t i, chunk_count;
    void **chunk;

    chunk_count = atra_list_length(&pool.chunks);

    for (i = 0; i < chunk_count; i++) {
        chunk = atra_list_top(&pool.chunks);
        free(*chunk);
        atra_list_pop(&pool.chunks);
    }
    atra_list_delete(&pool.chunks);
}

/* Returns a pointer to a block of allocated memory of requested size */
ATRA_API
void *
atra_mempool_malloc(size_t size)
{
    const size_t real_size = addguardsize(size);
    i8 *block, *next, *prev, *rem;
    size_t block_size, rem_size;

    if (real_size > pool.size) {
        atra_log_print(LOG_ERROR, "memory: requested memory block larger " \
                "than maximum chunk size: allocating from main memory...");
        return malloc(size);
    }
    /*  TODO: Perform quick check on pool free space to see if there is enough
     *      total space.  If total free space is not enough, jump to new chunk
     */

    block = pool.head;

    while (block != NULL) {
        block_size = getsize(block);
        if (block_size < size) {
            block = getnext(block);
        } else {
            prev = getprev(block);
            next = getnext(block);

            if (block_size - size < 2 * (GUARD_SIZE + sizeof(i8 *))) {
                /* Link the list without the remaining block space */
                if (prev) {
                    setnext(prev, next);
                }
                if (next) {
                    setprev(next, prev);
                }
                if (block == pool.head) {
                    pool.head = next;
                }
                if (block == pool.tail) {
                    pool.tail = prev;
                }
                /* Consume the rest of the block */
                size = block_size;
                pool.free_space -= size;
            } else {
                rem = block + real_size;
                rem_size = block_size - size - 2 * GUARD_SIZE;

                /* Calibrate remaining free memory as a new block */
                writeheader(rem, rem_size, 0);
                writebody(rem, next, prev);
                writefooter(rem, rem_size, 0);

                /* Update free memory linked list pointers */
                if (prev) {
                    setnext(prev, rem);
                }
                if (next) {
                    setprev(next, rem);
                }

                if (block == pool.head) {
                    pool.head = rem;
                }
                if (block == pool.tail) {
                    pool.tail = rem;
                }
                /* Shave off extra space for rem guards */
                pool.free_space -= size + GUARD_SIZE * 2;
            }
            /* Update new allocated block guards */
            writeheader(block, size, 1);
            writefooter(block, size, 1);

            return block + GUARD_SIZE;
        }
    }

    /* No viable memory block found, add a new chunk to the pool */
    block = malloc(pool.size);
    atra_list_add(&pool.chunks, &block);

    if (pool.size - real_size >= 2 * (GUARD_SIZE + sizeof(i8 *))) {
        /* Allocate part of new block */
        rem = block + real_size;
        rem_size = subguardsize(pool.size - real_size);
        
        if (pool.tail) {
            setnext(pool.tail, rem);
        }
        writeheader(rem, rem_size, 0);
        writebody(rem, NULL, pool.tail);
        writefooter(rem, rem_size, 0);

        pool.tail = rem;
        pool.free_space += rem_size;
    } else {
        size = subguardsize(pool.size);
    }
    writeheader(block, size, 1);
    writefooter(block, size, 1);

    return block + GUARD_SIZE;
}

/* Returns a pointer to a block of allocated, zeroed memory of requested size */
ATRA_API
void *
atra_mempool_calloc(size_t n, size_t size)
{
    void *mem;

    mem = atra_mempool_malloc(n * size);
    memset(mem, 0, n * size);

    return mem;
}

/* Returns a pointer to a new or resized block of memory of requested size */
ATRA_API
void *
atra_mempool_realloc(void *ptr, size_t size)
{
    /* Check if the given ptr has any consolidable space */

    ATRA_UNUSED(ptr);
    ATRA_UNUSED(size);

    return NULL;
}

/* Deallocates the memory at the given address */
ATRA_API
void
atra_mempool_free(void *ptr)
{
    i8 *block, *next, *prev;
    size_t block_size, prev_size;
    int cons_next;

    if (ptr == NULL) {
        return;
    }
    block = (i8 *)ptr - GUARD_SIZE;
    block_size = getsize(block);
    pool.free_space += block_size;
    next = pool.head;
    prev = NULL;
    cons_next = 0;

    /* Find position in linked list where block belongs */
    while (next != NULL && next < block) {
        prev = next;
        next = getnext(next);
    }
    
    /* Check the next list element for consolidation */
    if (next && next == block + addguardsize(block_size)) {
        block_size += addguardsize(getsize(next));
        cons_next = 1;
        pool.free_space += GUARD_SIZE * 2;
    }

    /* Check previous list element for consolidation */
    if (prev) {
        prev_size = addguardsize(getsize(prev));

        if (prev == block - prev_size) {
            block = prev;
            prev = getprev(prev);
            block_size += prev_size;
            pool.free_space += GUARD_SIZE * 2;

            if (prev == NULL) {
                pool.head = block;
            }
        } else {
            setnext(prev, block);
        }
    } else {
        pool.head = block;
    }

    /* Link the next node in the list */
    if (cons_next) {
        next = getnext(next);
    }
    if (next) {
        setprev(next, block);
    } else {
        pool.tail = block;
    }

    /* Finalise the new free block */
    writeheader(block, block_size, 0);
    writebody(block, next, prev);
    writefooter(block, block_size, 0);
}

/* Returns a pointer to the internal memory pool structure */
ATRA_API
MemPool *
atra_mempool_getpool(void)
{
    return &pool;
}
