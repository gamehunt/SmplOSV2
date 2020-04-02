#include <kernel/misc/types/queue.h>

queue_t* queue_create(uint32_t capacity){
	queue_t* queue = kmalloc(sizeof(queue_t));
	queue->front = queue->rear = -1;
	queue->size = capacity;
	queue->array = kmalloc(sizeof(uint32_t)*capacity);
}

void queue_free(queue_t* que){
	kfree(que->array);
	kfree(que);
}

uint8_t queue_isfull(queue_t* queue) 
{  
	return ((queue->front == 0 && queue->rear == queue->size-1) || 
            (queue->rear == (queue->front-1)%(queue->size-1)));  
} 
  
uint8_t queue_isempty(queue_t* queue) 
{  
	return (queue->front == -1); 
}

void queue_enque(queue_t* queue,uint32_t val){
	if (queue_isfull(queue)) 
        return; 
  
    else if (queue->front == -1) /* Insert First Element */
    { 
        queue->front = queue->rear = 0; 
        queue->array[queue->rear] = val; 
    } 
  
    else if (queue->rear == queue->size-1 && queue->front != 0) 
    { 
        queue->rear = 0; 
        queue->array[queue->rear] = val; 
    } 
  
    else
    { 
        queue->rear++; 
        queue->array[queue->rear] = val; 
    } 
}

uint32_t queue_deque(queue_t* queue){
	if (queue_isempty(queue)) 
        return 0; 
    uint32_t data = queue->array[queue->front]; 
    queue->array[queue->front] = -1; 
    if (queue->front == queue->rear) 
    { 
        queue->front = -1; 
        queue->rear = -1; 
    } 
    else if (queue->front == queue->size-1) 
        queue->front = 0; 
    else
        queue->front++; 
  
    return data; 
}


