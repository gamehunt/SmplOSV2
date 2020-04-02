#pragma once

#include <stdint.h>

typedef struct array {
    uint32_t* items;
    int capacity;
    int total;
} array_t;

array_t* array_create(uint32_t capacity);
uint32_t array_add(array_t *, uint32_t);
void array_set(array_t *, int, uint32_t);
uint32_t array_get(array_t *, int);
void array_delete(array_t *, int);
void array_free(array_t *);
