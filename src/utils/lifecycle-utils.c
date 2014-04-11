/* Copyright (c) 2006-2014, Universities Space Research Association (USRA).
*  All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above copyright
*       notice, this list of conditions and the following disclaimer in the
*       documentation and/or other materials provided with the distribution.
*     * Neither the name of the Universities Space Research Association nor the
*       names of its contributors may be used to endorse or promote products
*       derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY USRA ``AS IS'' AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL USRA BE LIABLE FOR ANY DIRECT, INDIRECT,
* INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
* BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
* OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
* ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR
* TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
* USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "lifecycle-utils.h"

#include <stddef.h> /* for NULL */
#include <stdlib.h> /* malloc(), free() */
#include <string.h> /* memset() */

/* #define LIFECYCLE_DEBUG 1 */

#ifdef LIFECYCLE_DEBUG
#include <stdio.h> /* puts(), printf() */
#endif


/*
 * An opstack is a stack, implemented as a linked list of buckets.
 * Buckets in the list are never empty.
 * insert_idx should only be 0 when list is empty or during allocation of a bucket. 
 * New buckets are allocated by opstack_push when the head has no slots free.
 */

/* Each bucket should fit in a CPU cache line */
#define OPSTACK_BUCKET_SIZE 7

struct opstack_bucket {
  struct opstack_bucket *prev;
  lc_operator ops[OPSTACK_BUCKET_SIZE];
};

struct opstack {
  struct opstack_bucket *head; /* pointer to most recent bucket */
  size_t insert_idx;           /* index of first open slot in bucket; range 1 - OPSTACK_BUCKET_SIZE */
#ifdef LIFECYCLE_DEBUG
  size_t n_buckets;           /* total # of buckets */
#endif
};

/* Allocate and initialize a bucket */
static struct opstack_bucket *new_opstack_bucket()
{
  struct opstack_bucket *result = 
    (struct opstack_bucket *) malloc(sizeof(struct opstack_bucket));
  memset((void *) result, 0, sizeof(struct opstack_bucket));
  return result;
}

static void opstack_push(struct opstack *list, lc_operator op)
{
  struct opstack_bucket *head;
  /* Ensure there is space for the new entry */
  if ((head = list->head) == NULL) {
    head = list->head = new_opstack_bucket();
    list->insert_idx = 0;
#ifdef LIFECYCLE_DEBUG
    list->n_buckets = 1;
#endif
  }
  else if (list->insert_idx == OPSTACK_BUCKET_SIZE) {
    /* Bucket full, allocate new bucket */
    struct opstack_bucket *old_head = list->head;
    head = new_opstack_bucket();
    head->prev = old_head;
    list->head = head;
    list->insert_idx = 0;
#ifdef LIFECYCLE_DEBUG
    list->n_buckets++;
#endif
  }

  /* Insert */
  head->ops[list->insert_idx++] = op;
}

static void opstack_run(struct opstack *list)
{
#ifdef LIFECYCLE_DEBUG
  if (!list->head)
    puts("opstack_run, empty list\n");
  else 
    printf("opstack_run, list has %u entries\n",
           list->n_buckets * OPSTACK_BUCKET_SIZE + list->insert_idx - 1);
#endif
  struct opstack_bucket *head = list->head;
  size_t i = list->insert_idx;
  while (head != NULL) {
#ifdef LIFECYCLE_DEBUG
    if (i == 0) {
      puts("opstack_run: ERROR: insert_idx is 0 at top of loop\n");
    }
#endif
    while (i > 0) {
      lc_operator op = head->ops[--i];
      (*op)();
    }
    /* Free this bucket and go to previous */
    list->head = head->prev;
    free(head);
    head = list->head;
    i = OPSTACK_BUCKET_SIZE;
  }
  /* At end */
  list->head = NULL;
  list->insert_idx = 0;
#ifdef LIFECYCLE_DEBUG
  list->n_buckets = 0;
  puts("opstack_run finished\n");
#endif
}

static struct opstack s_finalizers = {NULL,
                                      0
#ifdef LIFECYCLE_DEBUG
                                      , 0
#endif                             
};

void addFinalizer(lc_operator op)
{
  opstack_push(&s_finalizers, op);
}

void runFinalizers()
{
  opstack_run(&s_finalizers);
}
