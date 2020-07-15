/**
 * Copyright Amazon.com, Inc. or its affiliates. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0.
 */

#include <aws/common/array_list.h>
#include <aws/common/private/array_list.h>

#include <stdlib.h> /* qsort */

#include <seahorn/seahorn.h>
#include <stdio.h>

size_t size_t_nondet();
char char_nondet();

int aws_array_list_calc_necessary_size(struct aws_array_list *AWS_RESTRICT list, size_t index, size_t *necessary_size) {
    AWS_PRECONDITION(aws_array_list_is_valid(list));
    size_t index_inc;
    if (aws_add_size_checked(index, 1, &index_inc)) {
        AWS_POSTCONDITION(aws_array_list_is_valid(list));
        return AWS_OP_ERR;
    }

    if (aws_mul_size_checked(index_inc, list->item_size, necessary_size)) {
        AWS_POSTCONDITION(aws_array_list_is_valid(list));
        return AWS_OP_ERR;
    }
    AWS_POSTCONDITION(aws_array_list_is_valid(list));
    
    return AWS_OP_SUCCESS;
}

void alloc_buffer(struct aws_array_list *list) {
    // create harness with nondeterministic values for all fields and memory
    // for data.
    list->current_size = size_t_nondet();
    list->length = size_t_nondet();
    //AWS_PRECONDITION(list->current_size > 0);
    list->item_size = size_t_nondet();
    list->data = malloc(list->current_size);
    list->alloc = NULL;
    printf("List allocated.  current_size: %lu length = %lu item_size = %lu data=%p\n", list->current_size, list->length, list->item_size, list->data);
    int i;
    for (i=0; i<list->current_size; i++) {
        ((char *)list->data)[i] = char_nondet();
    }
    printf("Filled buffer.\n");
    AWS_PRECONDITION(aws_array_list_is_valid(list));
    printf("Assumed precondition.\n");
}

int aws_array_list_calc_necessary_size_harness(void) {
    printf("At harness.\n");
    size_t necessary_size = size_t_nondet();
    struct aws_array_list list;
    size_t index = size_t_nondet();
    int ret_val;
    printf("At alloc buffer.\n");
    alloc_buffer(&list);
    ret_val = aws_array_list_calc_necessary_size(&list, index, &necessary_size);
    AWS_POSTCONDITION(aws_array_list_is_valid(&list));
    printf("All postconditions passed.\n");
    return ret_val;
}


int main() {
    printf("Started.\n");
    aws_array_list_copy_harness();
    return 0;
}

int aws_array_list_shrink_to_fit(struct aws_array_list *AWS_RESTRICT list) {
    AWS_PRECONDITION(aws_array_list_is_valid(list));
    if (list->alloc) {
        size_t ideal_size;
        if (aws_mul_size_checked(list->length, list->item_size, &ideal_size)) {
            AWS_POSTCONDITION(aws_array_list_is_valid(list));
            return AWS_OP_ERR;
        }

        if (ideal_size < list->current_size) {
            void *raw_data = NULL;

            if (ideal_size > 0) {
                raw_data = aws_mem_acquire(list->alloc, ideal_size);
                if (!raw_data) {
                    AWS_POSTCONDITION(aws_array_list_is_valid(list));
                    return AWS_OP_ERR;
                }

                memcpy(raw_data, list->data, ideal_size);
                aws_mem_release(list->alloc, list->data);
            }
            list->data = raw_data;
            list->current_size = ideal_size;
        }
        AWS_POSTCONDITION(aws_array_list_is_valid(list));
        return AWS_OP_SUCCESS;
    }

    AWS_POSTCONDITION(aws_array_list_is_valid(list));
    return aws_raise_error(AWS_ERROR_LIST_STATIC_MODE_CANT_SHRINK);
}

int aws_array_list_shrink_to_fit_harness(void) {
    printf("At harness.\n");
    struct aws_array_list list;
    int ret_val;
    printf("At alloc buffer.\n");
    alloc_buffer(&list);
    ret_val = aws_array_list_shrink_to_fit(&list);
    printf("All postconditions passed.\n");
    return ret_val;
}

int aws_array_list_copy(const struct aws_array_list *AWS_RESTRICT from, struct aws_array_list *AWS_RESTRICT to) {
    AWS_FATAL_PRECONDITION(from->item_size == to->item_size);
    AWS_FATAL_PRECONDITION(from->data);
    AWS_PRECONDITION(aws_array_list_is_valid(from));
    AWS_PRECONDITION(aws_array_list_is_valid(to));

    size_t copy_size;
    if (aws_mul_size_checked(from->length, from->item_size, &copy_size)) {
        AWS_POSTCONDITION(aws_array_list_is_valid(from));
        AWS_POSTCONDITION(aws_array_list_is_valid(to));
        return AWS_OP_ERR;
    }

    if (to->current_size >= copy_size) {
        if (copy_size > 0) {
            memcpy(to->data, from->data, copy_size);
        }
        to->length = from->length;
        AWS_POSTCONDITION(aws_array_list_is_valid(from));
        AWS_POSTCONDITION(aws_array_list_is_valid(to));
        return AWS_OP_SUCCESS;
    }
    /* if to is in dynamic mode, we can just reallocate it and copy */
    if (to->alloc != NULL) {
        void *tmp = aws_mem_acquire(to->alloc, copy_size);

        if (!tmp) {
            AWS_POSTCONDITION(aws_array_list_is_valid(from));
            AWS_POSTCONDITION(aws_array_list_is_valid(to));
            return AWS_OP_ERR;
        }

        memcpy(tmp, from->data, copy_size);
        if (to->data) {
            aws_mem_release(to->alloc, to->data);
        }

        to->data = tmp;
        to->length = from->length;
        to->current_size = copy_size;
        AWS_POSTCONDITION(aws_array_list_is_valid(from));
        AWS_POSTCONDITION(aws_array_list_is_valid(to));
        return AWS_OP_SUCCESS;
    }

    return aws_raise_error(AWS_ERROR_DEST_COPY_TOO_SMALL);
}

int aws_array_list_copy_harness(void) {
    printf("At harness.\n");
    struct aws_array_list list1;
    struct aws_array_list list2;
    int ret_val;
    printf("At alloc buffer.\n");
    alloc_buffer(&list1);
    alloc_buffer(&list2);
    ret_val = aws_array_list_copy(&list1, &list2);
    printf("All postconditions passed.\n");
    return ret_val;
}


int aws_array_list_ensure_capacity(struct aws_array_list *AWS_RESTRICT list, size_t index) {
    AWS_PRECONDITION(aws_array_list_is_valid(list));
    size_t necessary_size;
    if (aws_array_list_calc_necessary_size(list, index, &necessary_size)) {
        AWS_POSTCONDITION(aws_array_list_is_valid(list));
        return AWS_OP_ERR;
    }

    if (list->current_size < necessary_size) {
        if (!list->alloc) {
            AWS_POSTCONDITION(aws_array_list_is_valid(list));
            return aws_raise_error(AWS_ERROR_INVALID_INDEX);
        }

        /* this will double capacity if the index isn't bigger than what the
         * next allocation would be, but allocates the exact requested size if
         * it is. This is largely because we don't have a good way to predict
         * the usage pattern to make a smart decision about it. However, if the
         * user
         * is doing this in an iterative fashion, necessary_size will never be
         * used.*/
        size_t next_allocation_size = list->current_size << 1;
        size_t new_size = next_allocation_size > necessary_size ? next_allocation_size : necessary_size;

        if (new_size < list->current_size) {
            /* this means new_size overflowed. The only way this happens is on a
             * 32-bit system where size_t is 32 bits, in which case we're out of
             * addressable memory anyways, or we're on a 64 bit system and we're
             * most certainly out of addressable memory. But since we're simply
             * going to fail fast and say, sorry can't do it, we'll just tell
             * the user they can't grow the list anymore. */
            AWS_POSTCONDITION(aws_array_list_is_valid(list));
            return aws_raise_error(AWS_ERROR_LIST_EXCEEDS_MAX_SIZE);
        }

        void *temp = aws_mem_acquire(list->alloc, new_size);

        if (!temp) {
            AWS_POSTCONDITION(aws_array_list_is_valid(list));
            return AWS_OP_ERR;
        }

        if (list->data) {
            memcpy(temp, list->data, list->current_size);

#ifdef DEBUG_BUILD
            memset(
                (void *)((uint8_t *)temp + list->current_size),
                AWS_ARRAY_LIST_DEBUG_FILL,
                new_size - list->current_size);
#endif
            aws_mem_release(list->alloc, list->data);
        }
        list->data = temp;
        list->current_size = new_size;
    }

    AWS_POSTCONDITION(aws_array_list_is_valid(list));
    return AWS_OP_SUCCESS;
}

static void aws_array_list_mem_swap(void *AWS_RESTRICT item1, void *AWS_RESTRICT item2, size_t item_size) {
    enum { SLICE = 128 };

    AWS_FATAL_PRECONDITION(item1);
    AWS_FATAL_PRECONDITION(item2);

    /* copy SLICE sized bytes at a time */
    size_t slice_count = item_size / SLICE;
    uint8_t temp[SLICE];
    for (size_t i = 0; i < slice_count; i++) {
        memcpy((void *)temp, (void *)item1, SLICE);
        memcpy((void *)item1, (void *)item2, SLICE);
        memcpy((void *)item2, (void *)temp, SLICE);
        item1 = (uint8_t *)item1 + SLICE;
        item2 = (uint8_t *)item2 + SLICE;
    }

    size_t remainder = item_size & (SLICE - 1); /* item_size % SLICE */
    memcpy((void *)temp, (void *)item1, remainder);
    memcpy((void *)item1, (void *)item2, remainder);
    memcpy((void *)item2, (void *)temp, remainder);
}

void aws_array_list_swap(struct aws_array_list *AWS_RESTRICT list, size_t a, size_t b) {
    AWS_FATAL_PRECONDITION(a < list->length);
    AWS_FATAL_PRECONDITION(b < list->length);
    AWS_PRECONDITION(aws_array_list_is_valid(list));

    if (a == b) {
        AWS_POSTCONDITION(aws_array_list_is_valid(list));
        return;
    }

    void *item1 = NULL, *item2 = NULL;
    aws_array_list_get_at_ptr(list, &item1, a);
    aws_array_list_get_at_ptr(list, &item2, b);
    aws_array_list_mem_swap(item1, item2, list->item_size);
    AWS_POSTCONDITION(aws_array_list_is_valid(list));
}
