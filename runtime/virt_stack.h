//
// Created by egor on 24.04.23.
//

#ifndef LAMA_RUNTIME_VIRT_STACK_H
#define LAMA_RUNTIME_VIRT_STACK_H
#define RUNTIME_VSTACK_SIZE 100000

#include <assert.h>
#include <stddef.h>
#include "runtime.h"

struct {
  size_t *bot;
  size_t *top;
  size_t **cur;
} typedef virt_stack;

virt_stack *vstack_create (size_t size, size_t **cur);

void vstack_destruct (virt_stack *st);

void vstack_init (virt_stack *st);

void vstack_push (virt_stack *st, size_t value);

void vstack_reverse (virt_stack *st, size_t size);

size_t vstack_pop (virt_stack *st);

size_t *vstack_top (virt_stack *st);

size_t vstack_kth_from_cur (virt_stack *st, size_t k);

size_t *vstack_access(virt_stack *st, size_t *addr);

#endif   //LAMA_RUNTIME_VIRT_STACK_H
