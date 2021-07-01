/*
 *  ATRA ENGINE
 *
 *  Copyright (c) 2019-2021 Robert Saarloos, All rights reserved.
 *
 *  test_memory.h - Unit tests for Atra Engine's memory module.
 *  
 *  */

#ifndef ATRA_TEST_MEMORY_H
#define ATRA_TEST_MEMORY_H

#include <atra.h>
#include <check.h>

START_TEST(test_list_create_4byte)
{
    List list = atra_list_create(4);

    ck_assert_uint_eq(list.elem_size, 4);
    ck_assert_uint_eq(list.size, list.elem_size * ATRA_LIST_INIT_CAP);
    ck_assert_ptr_eq(list.head, list.start);

    atra_list_delete(&list);
}
END_TEST

START_TEST(test_list_create_16byte)
{
    List list = atra_list_create(16);

    ck_assert_uint_eq(list.elem_size, 16);
    ck_assert_uint_eq(list.size, list.elem_size * ATRA_LIST_INIT_CAP);
    ck_assert_ptr_eq(list.head, list.start);

    atra_list_delete(&list);
}
END_TEST

START_TEST(test_list_create_maxbyte)
{
    List list = atra_list_create(1);

    ck_assert_ptr_eq(list.start, NULL);
    ck_assert_ptr_eq(list.head, list.start);
}
END_TEST

START_TEST(test_list_create_zero)
{
    List list = atra_list_create(1);

    ck_assert_ptr_eq(list.start, NULL);
    ck_assert_ptr_eq(list.head, list.start);
}
END_TEST

#endif /* ATRA_TEST_MEMORY_H */
