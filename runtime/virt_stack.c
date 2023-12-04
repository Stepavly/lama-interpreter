#include "virt_stack.h"

#include <malloc.h>

virt_stack *vstack_create (size_t size, size_t **cur) {
  virt_stack *st = (virt_stack*) malloc(sizeof(virt_stack));
  if (st == NULL) {
    return NULL;
  }

  size_t *stack = (size_t*) malloc(sizeof(size_t) * size);
  if (stack == NULL) {
    free(st);
    return NULL;
  }

  st->bot = stack;
  st->top = stack + size;
  st->cur = cur;
  *cur = st->top;
  return st;
}

void vstack_destruct (virt_stack *st) { 
  free(st->bot);
  free(st); 
}

size_t vstack_size (virt_stack *st) {
  return st->top - *st->cur;
}

void vstack_push (virt_stack *st, size_t value) {
  if (*st->cur == st->bot) {
    failure("Stack overflow");
  }
  --(*st->cur);
  **st->cur = value;
}

void vstack_reverse (virt_stack *st, size_t size) {
  if (vstack_size(st) < size) {
    failure("Stack size less than reversing size");
  }
  for (size_t i = 0; i * 2 < size; i++) {
    size_t tmp = *(*st->cur + i);
    *(*st->cur + i) = *(*st->cur + (size - i - 1));
    *(*st->cur + (size - i - 1)) = tmp;
  }
}

size_t vstack_pop (virt_stack *st) {
  if (*st->cur == st->top) {
    failure("Pop from empty stack");
  }
  size_t value = **st->cur;
  ++(*st->cur);
  return value;
}

size_t *vstack_top (virt_stack *st) { 
  return *st->cur;
}

size_t vstack_kth_from_cur (virt_stack *st, size_t k) {
  if (vstack_size(st) < k) {
    failure("Stack size less than index");
  }
  return *(*st->cur + k);
}

size_t *vstack_access(virt_stack *st, size_t *addr) {
  if (!(*st->cur <= addr && addr < st->top)) {
    failure("Accessing stack by removed address");
  }
  return addr;
}
