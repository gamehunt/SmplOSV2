#pragma once 

#include <stdint.h>

//Implementation of circle queue

typedef struct{
	int front, rear; 
    uint32_t size; 
    uint32_t* array;
}queue_t;

queue_t* queue_create(uint32_t capacity);

void queue_enque(queue_t* que,uint32_t val);
uint32_t queue_deque(queue_t* que);

void queue_free(queue_t* que);

uint8_t queue_isfull(queue_t* queue);
  
uint8_t queue_isempty(queue_t* queue); 

