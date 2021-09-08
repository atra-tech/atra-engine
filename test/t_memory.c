#include <check.h>
#include <atra.h>

static const char *test_data[5] = {
    "0123456789ABCDEF",
    "FEDCBA9876543210",
    "1234123412341234",
    "ABCDABCDABCDABCD",
    "The data to test"
};

static const float quad[4][2] = {
    {1.0f, 0.0f},
    {0.0f, 0.0f},
    {0.0f, 1.0f},
    {1.0f, 1.0f}
};


static List list;
static Queue queue;

/******************************************************************************
 *  List Tests
 ******************************************************************************/

ATRA_INLINE
void
list_loadtestdata(void)
{
    atra_list_add(&list, test_data[0]);
    atra_list_add(&list, test_data[1]);
    atra_list_add(&list, test_data[2]);
    atra_list_add(&list, test_data[3]);
    atra_list_add(&list, test_data[4]);
}

ATRA_INLINE
void
queue_loadtestdata_pushback(void)
{
    atra_queue_pushback(&queue, test_data[0]);
    atra_queue_pushback(&queue, test_data[1]);
    atra_queue_pushback(&queue, test_data[2]);
    atra_queue_pushback(&queue, test_data[3]);
    atra_queue_pushback(&queue, test_data[4]);
}

ATRA_INLINE
void
queue_loadtestdata_pushfront(void)
{
    atra_queue_pushfront(&queue, test_data[0]);
    atra_queue_pushfront(&queue, test_data[1]);
    atra_queue_pushfront(&queue, test_data[2]);
    atra_queue_pushfront(&queue, test_data[3]);
    atra_queue_pushfront(&queue, test_data[4]);
}

ATRA_INLINE
void
mempool_loadtestdata_str(void)
{
    /*  Expected memory layout: 
     *  | G P P P P P G | G C ... C G | G C ... C G | G C ... C G | G C ... C G | G C ... C G |
     */

    int i;
    char **data_cpy;

    data_cpy = atra_mempool_malloc(5 * sizeof(char *));

    for (i = 0; i < 5; i++) {
        data_cpy[i] = atra_mempool_malloc(17);
        memcpy(data_cpy[i], test_data[i], 17);
    }
}

ATRA_INLINE
void
mempool_loadtestdata_vec3(void)
{
    /*  Expected memory layout:
     *  | G F F G | G F F G | G F F G | G F F G |
     */

    int i;
    float *quad_cpy[4];

    for (i = 0; i < 4; i++) {
        quad_cpy[i] = atra_mempool_malloc(2 * sizeof(float));
        memcpy(quad_cpy[i], quad[i], 2 * sizeof(float));
    }
}


START_TEST(list_create)
{
    list = atra_list_create(64);

    ck_assert_ptr_nonnull(list.start);
    ck_assert_msg(list.elem_size == 64, "Expected elem size of 64 bytes");

    atra_list_delete(&list);
}
END_TEST

START_TEST(list_create_zero)
{
    list = atra_list_create(0);
    
    ck_assert_ptr_null(list.start);
}
END_TEST

START_TEST(list_create_bigelem)
{
    list = atra_list_create(SIZE_MAX / 2);

    ck_assert_ptr_null(list.start);
}
END_TEST

START_TEST(list_addandget)
{
    char *output[5];

    list = atra_list_create(17);

    list_loadtestdata();
    list_loadtestdata();

    output[0] = atra_list_get(&list, 0);
    output[1] = atra_list_get(&list, 1);
    output[2] = atra_list_get(&list, 2);
    output[3] = atra_list_get(&list, 3);
    output[4] = atra_list_get(&list, 4);

    ck_assert_str_eq(output[0], test_data[0]);
    ck_assert_str_eq(output[1], test_data[1]);
    ck_assert_str_eq(output[2], test_data[2]);
    ck_assert_str_eq(output[3], test_data[3]);
    ck_assert_str_eq(output[4], test_data[4]);

    ck_assert_msg(list.size == 16 * list.elem_size,
            "Expected list to have grown from default size");

    atra_list_delete(&list);
}
END_TEST

START_TEST(list_topandpop)
{
    char *output;

    list = atra_list_create(17);

    list_loadtestdata();
    list_loadtestdata();

    output = atra_list_top(&list);
    ck_assert_str_eq(output, test_data[4]);
    atra_list_pop(&list);
    output = atra_list_top(&list);
    ck_assert_str_eq(output, test_data[3]);

    ck_assert_msg(list.size == 16 * list.elem_size,
            "Expected list to have grown from default size");

    atra_list_delete(&list);
}
END_TEST

START_TEST(list_emptiness)
{
    list = atra_list_create(17);

    ck_assert_int_eq(atra_list_isempty(&list), 1);

    list_loadtestdata();

    ck_assert_msg(list.head - list.start == 17 * 5, "Invalid list head position");
    ck_assert_int_eq(atra_list_isempty(&list), 0);
    atra_list_clear(&list);
    ck_assert_int_eq(atra_list_isempty(&list), 1);
    
    atra_list_delete(&list);
}
END_TEST

START_TEST(list_remove)
{
    list = atra_list_create(17);

    list_loadtestdata();

    ck_assert_mem_eq(atra_list_get(&list, 2), test_data[2], 17);
    atra_list_remove(&list, 2);
    ck_assert_mem_eq(atra_list_get(&list, 2), test_data[3], 17);

    atra_list_delete(&list);
}
END_TEST

START_TEST(list_removerange)
{
    list = atra_list_create(17);

    list_loadtestdata();

    ck_assert_mem_eq(atra_list_get(&list, 1), test_data[1], 17);
    atra_list_removerange(&list, 1, 4);
    ck_assert_mem_eq(atra_list_get(&list, 1), test_data[4], 17);

    atra_list_delete(&list);
}
END_TEST

START_TEST(list_move)
{
    list = atra_list_create(17);

    list_loadtestdata();

    ck_assert_mem_eq(atra_list_get(&list, 0), test_data[0], 17);
    ck_assert_mem_eq(atra_list_get(&list, 1), test_data[1], 17);
    ck_assert_mem_eq(atra_list_get(&list, 2), test_data[2], 17);
    ck_assert_mem_eq(atra_list_get(&list, 3), test_data[3], 17);
    ck_assert_mem_eq(atra_list_get(&list, 4), test_data[4], 17);
    atra_list_move(&list, 3, 1);
    atra_list_move(&list, 0, 2);
    atra_list_move(&list, 2, 4);
    ck_assert_mem_eq(atra_list_get(&list, 0), test_data[3], 17);
    ck_assert_mem_eq(atra_list_get(&list, 1), test_data[1], 17);
    ck_assert_mem_eq(atra_list_get(&list, 2), test_data[2], 17);
    ck_assert_mem_eq(atra_list_get(&list, 3), test_data[4], 17);
    ck_assert_mem_eq(atra_list_get(&list, 4), test_data[0], 17);

    atra_list_delete(&list);
}
END_TEST

/******************************************************************************
 *  Queue Tests
 ******************************************************************************/

START_TEST(queue_create)
{
    queue = atra_queue_create(17);

    ck_assert_ptr_nonnull(queue.start);

    atra_queue_delete(&queue);
}
END_TEST

START_TEST(queue_create_zero)
{
    queue = atra_queue_create(0);

    ck_assert_ptr_null(queue.start);
}
END_TEST

START_TEST(queue_create_bigelem)
{
    queue = atra_queue_create(SIZE_MAX / 2);

    ck_assert_ptr_null(queue.start);
}
END_TEST

START_TEST(queue_pushandpop_back)
{
    queue = atra_queue_create(17);

    queue_loadtestdata_pushback();

    ck_assert_mem_eq(atra_queue_back(&queue), test_data[4], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popback(&queue), test_data[4], queue.elem_size);
    ck_assert_mem_eq(atra_queue_back(&queue), test_data[3], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popback(&queue), test_data[3], queue.elem_size);
    ck_assert_mem_eq(atra_queue_back(&queue), test_data[2], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popback(&queue), test_data[2], queue.elem_size);
    ck_assert_mem_eq(atra_queue_back(&queue), test_data[1], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popback(&queue), test_data[1], queue.elem_size);
    ck_assert_mem_eq(atra_queue_back(&queue), test_data[0], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popback(&queue), test_data[0], queue.elem_size);

    atra_queue_delete(&queue);
}
END_TEST

START_TEST(queue_pushandpop_front)
{
    queue = atra_queue_create(17);

    queue_loadtestdata_pushfront();

    ck_assert_mem_eq(atra_queue_front(&queue), test_data[4], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popfront(&queue), test_data[4], queue.elem_size);
    ck_assert_mem_eq(atra_queue_front(&queue), test_data[3], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popfront(&queue), test_data[3], queue.elem_size);
    ck_assert_mem_eq(atra_queue_front(&queue), test_data[2], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popfront(&queue), test_data[2], queue.elem_size);
    ck_assert_mem_eq(atra_queue_front(&queue), test_data[1], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popfront(&queue), test_data[1], queue.elem_size);
    ck_assert_mem_eq(atra_queue_front(&queue), test_data[0], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popfront(&queue), test_data[0], queue.elem_size);

    atra_queue_delete(&queue);
}
END_TEST

START_TEST(queue_pushback_popfront)
{
    queue = atra_queue_create(17);

    queue_loadtestdata_pushback();

    ck_assert_mem_eq(atra_queue_front(&queue), test_data[0], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popfront(&queue), test_data[0], queue.elem_size);
    ck_assert_mem_eq(atra_queue_front(&queue), test_data[1], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popfront(&queue), test_data[1], queue.elem_size);
    ck_assert_mem_eq(atra_queue_front(&queue), test_data[2], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popfront(&queue), test_data[2], queue.elem_size);
    ck_assert_mem_eq(atra_queue_front(&queue), test_data[3], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popfront(&queue), test_data[3], queue.elem_size);
    ck_assert_mem_eq(atra_queue_front(&queue), test_data[4], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popfront(&queue), test_data[4], queue.elem_size);
    
    atra_queue_delete(&queue);
}
END_TEST

START_TEST(queue_pushfront_popback)
{
    queue = atra_queue_create(17);

    queue_loadtestdata_pushfront();
    
    ck_assert_mem_eq(atra_queue_back(&queue), test_data[0], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popback(&queue), test_data[0], queue.elem_size);
    ck_assert_mem_eq(atra_queue_back(&queue), test_data[1], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popback(&queue), test_data[1], queue.elem_size);
    ck_assert_mem_eq(atra_queue_back(&queue), test_data[2], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popback(&queue), test_data[2], queue.elem_size);
    ck_assert_mem_eq(atra_queue_back(&queue), test_data[3], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popback(&queue), test_data[3], queue.elem_size);
    ck_assert_mem_eq(atra_queue_back(&queue), test_data[4], queue.elem_size);
    ck_assert_mem_eq(atra_queue_popback(&queue), test_data[4], queue.elem_size);
    
    atra_queue_delete(&queue);
}
END_TEST

START_TEST(queue_circular_pushpop)
{
    int i, f, b;

    queue = atra_queue_create(17);

    queue_loadtestdata_pushback();

    for (i = 0; i < 32; i++) {
        f = i % 5;
        b = (i + 4) % 5;
        
        ck_assert_mem_eq(atra_queue_popfront(&queue), test_data[f], queue.elem_size);
        ck_assert_mem_eq(atra_queue_back(&queue), test_data[b], queue.elem_size);

        atra_queue_pushback(&queue, test_data[f]);
    }
    atra_queue_delete(&queue);
}
END_TEST


START_TEST(queue_emptiness)
{
    queue = atra_queue_create(17);
    
    ck_assert_int_eq(atra_queue_isempty(&queue), 1);
    queue_loadtestdata_pushback();

    ck_assert_int_eq(atra_queue_isempty(&queue), 0);
    atra_queue_clear(&queue);
    ck_assert_int_eq(atra_queue_isempty(&queue), 1);
    
    atra_queue_delete(&queue);
}
END_TEST

/******************************************************************************
 *  Hashmap Tests
 ******************************************************************************/


/******************************************************************************
 *  Memory Pool Tests
 ******************************************************************************/

#define getnext(block) *(i8 **)(block + GUARD_SIZE)
#define getprev(block) *(i8 **)(block + GUARD_SIZE + sizeof(void *))
#define setnext(block, next) getnext(block) = next
#define setprev(block, prev) getprev(block) = prev
#define getsize(block) *(size_t *)block
#define addguardsize(size) (size + GUARD_SIZE * 2)
#define subguardsize(size) (size - GUARD_SIZE * 2)

static const size_t GUARD_SIZE = sizeof(size_t) + sizeof(i32);

#define MEMPOOL_DEFAULT_SIZE 16 * 1024 * 1024

START_TEST(mempool_create)
{
    MemPool *pool;
    i8 **chunk;

    atra_mempool_init(64);
    pool = atra_mempool_getpool();
    chunk = atra_list_get(&pool->chunks, 0);

    ck_assert_msg(pool->free_space == subguardsize(64), "Bad free space count");
    ck_assert_msg(pool->size == 64, "Bad block size");
    ck_assert_int_eq(atra_list_isempty(&pool->chunks), 0);
    ck_assert_int_eq(atra_list_length(&pool->chunks), 1);

    ck_assert_ptr_nonnull(*chunk);
    ck_assert_ptr_eq(*chunk, pool->head);
    ck_assert_ptr_eq(*chunk, pool->tail);

    atra_mempool_stop();
}
END_TEST

START_TEST(mempool_create_zerosize)
{
    MemPool *pool;
    i8 **chunk;

    atra_mempool_init(0);
    pool = atra_mempool_getpool();
    chunk = atra_list_get(&pool->chunks, 0);

    ck_assert_msg(pool->free_space == subguardsize(MEMPOOL_DEFAULT_SIZE),
            "Bad free space count");
    ck_assert_msg(pool->size == MEMPOOL_DEFAULT_SIZE, "Bad block size");
    ck_assert_int_eq(atra_list_isempty(&pool->chunks), 0);
    ck_assert_int_eq(atra_list_length(&pool->chunks), 1);

    ck_assert_ptr_nonnull(*chunk);
    ck_assert_ptr_eq(*chunk, pool->head);
    ck_assert_ptr_eq(*chunk, pool->tail);

    atra_mempool_stop();
}
END_TEST

START_TEST(mempool_create_hugesize)
{
    MemPool *pool;
    i8 **chunk;

    atra_mempool_init(1024 * 1024 * 1024);
    pool = atra_mempool_getpool();

    chunk = atra_list_get(&pool->chunks, 0);

    ck_assert_msg(pool->free_space == subguardsize(MEMPOOL_DEFAULT_SIZE),
            "Bad free space count");
    ck_assert_msg(pool->size == MEMPOOL_DEFAULT_SIZE, "Bad block size");
    ck_assert_int_eq(atra_list_isempty(&pool->chunks), 0);
    ck_assert_int_eq(atra_list_length(&pool->chunks), 1);

    ck_assert_ptr_nonnull(*chunk);
    ck_assert_ptr_eq(*chunk, pool->head);
    ck_assert_ptr_eq(*chunk, pool->tail);

    atra_mempool_stop();
}
END_TEST

START_TEST(mempool_malloc_simple)
{
    size_t chunk_size, i;
    MemPool *pool;
    i8 *data, **chunk;

    /* Test quad vertex */
    chunk_size = 8 * (sizeof(i8 *) + GUARD_SIZE);
    atra_mempool_init(chunk_size);
    pool = atra_mempool_getpool();
    chunk = atra_list_get(&pool->chunks, 0);

    mempool_loadtestdata_vec3();
    data = *chunk;
    data += GUARD_SIZE;

    for (i = 0; i < 4; i++) {
        ck_assert_mem_eq(data, quad[i], 2 * sizeof(float));
        data += 2 * (GUARD_SIZE + sizeof(float));
    }
    data = *chunk;
    data += GUARD_SIZE;

    for (i = 0; i < 4; i++) {
        atra_mempool_free(data);
        data += 2 * (GUARD_SIZE + sizeof(float));
    }
    data = *chunk;

    ck_assert_msg(*(size_t *)data == subguardsize(chunk_size),
            "Chunk guard (%lu) -> size (%lu) mismatch",
            *(size_t *)data, subguardsize(chunk_size));
    ck_assert_msg(pool->free_space == subguardsize(chunk_size),
            "Free space (%lu) -> size (%lu) mismatch",
            pool->free_space, subguardsize(chunk_size));

    atra_mempool_stop();
}
END_TEST

START_TEST(mempool_malloc_advanced)
{

}
END_TEST

START_TEST(mempool_malloc_speedcomp)
{
    const size_t malloc_count = 2000000;
    const size_t malloc_size = 4 * sizeof(int);
    const int malloc_data[4] = {1, 2, 3, 4};

    clock_t time_start, time_end;
    float os_malloc_time, atra_malloc_time;

    size_t i;
    float **data;

    data = malloc(malloc_count * sizeof(float *));
    atra_mempool_init(256 * 1024 * 1024);

    /* Atra Malloc */
    time_start = clock();

    for (i = 0; i < malloc_count; i++) {
        data[i] = atra_mempool_malloc(malloc_size);
        memcpy(data[i], malloc_data, malloc_size);
    }

    for (i = 0; i < malloc_count; i++) {
        atra_mempool_free(data[i]);
    }
    time_end = clock();
    atra_malloc_time = (time_end - time_start) / (float)CLOCKS_PER_SEC;

    /* OS Malloc */
    time_start = clock();

    for (i = 0; i < malloc_count; i++) {
        data[i] = malloc(malloc_size);
        memcpy(data[i], malloc_data, malloc_size);
    }
    
    for (i = 0; i < malloc_count; i++) {
        free(data[i]);
    }
    time_end = clock();
    os_malloc_time = (time_end - time_start) / (float)CLOCKS_PER_SEC;

    atra_log_print(LOG_INFO, "Atra Engine: %.3fs", atra_malloc_time);
    atra_log_print(LOG_INFO, "Operating System: %.3fs", os_malloc_time);
    atra_log_print(LOG_INFO, "Atra malloc is approx. %.3fx faster",
            os_malloc_time / atra_malloc_time);

    atra_mempool_stop();
}
END_TEST

START_TEST(mempool_free)
{

}
END_TEST


/* Suites and Runners */

Suite *
list_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("List");

    tc_core = tcase_create("Core");
    tcase_add_test(tc_core, list_create);
    tcase_add_test(tc_core, list_create_zero);
    tcase_add_test(tc_core, list_create_bigelem);
    tcase_add_test(tc_core, list_addandget);
    tcase_add_test(tc_core, list_topandpop);
    tcase_add_test(tc_core, list_emptiness);
    tcase_add_test(tc_core, list_remove);
    tcase_add_test(tc_core, list_removerange);
    tcase_add_test(tc_core, list_move);

    suite_add_tcase(s, tc_core);

    return s;
}

Suite *
queue_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Queue");

    tc_core = tcase_create("Core");
    tcase_add_test(tc_core, queue_create);
    tcase_add_test(tc_core, queue_create_zero);
    tcase_add_test(tc_core, queue_create_bigelem);
    tcase_add_test(tc_core, queue_pushandpop_back);
    tcase_add_test(tc_core, queue_pushandpop_front);
    tcase_add_test(tc_core, queue_pushfront_popback);
    tcase_add_test(tc_core, queue_pushback_popfront);
    tcase_add_test(tc_core, queue_circular_pushpop);
    tcase_add_test(tc_core, queue_emptiness);

    suite_add_tcase(s, tc_core);

    return s;
}

Suite *
hashmap_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Hashmap");

    tc_core = tcase_create("Core");

    suite_add_tcase(s, tc_core);

    return s;
}

Suite *
mempool_suite(void)
{
    Suite *s;
    TCase *tc_core;

    s = suite_create("Memory Pool");

    tc_core = tcase_create("Core");
    tcase_add_test(tc_core, mempool_create);
    tcase_add_test(tc_core, mempool_create_zerosize);
    tcase_add_test(tc_core, mempool_create_hugesize);
    tcase_add_test(tc_core, mempool_malloc_simple);
    tcase_add_test(tc_core, mempool_malloc_speedcomp);

    suite_add_tcase(s, tc_core);

    return s;
}

int
main(int argc, char **argv)
{
    int fail_count;
    Suite *s_list, *s_queue, *s_hashmap, *s_mempool;
    SRunner *sr;

    ATRA_UNUSED(argc);
    ATRA_UNUSED(argv);
    
    ATRA_UNUSED(s_hashmap);

    s_list = list_suite();
    s_queue = queue_suite();
    s_mempool = mempool_suite();

    sr = srunner_create(s_list);
    srunner_add_suite(sr, s_queue);
    srunner_add_suite(sr, s_mempool);

    srunner_set_fork_status(sr, CK_NOFORK);

    srunner_run_all(sr, CK_NORMAL);
    fail_count = srunner_ntests_failed(sr);

    srunner_free(sr);

    return (fail_count == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}
