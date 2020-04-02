#include <kernel/misc/types/dyn_array.h>

#define NULL 0

array_t* array_create(uint32_t capacity)
{
	array_t* v = kmalloc(sizeof(array_t));
    v->capacity = capacity;
    v->total = 0;
    v->items = kmalloc(sizeof(uint32_t) * v->capacity);
    return v;
}

uint32_t array_add(array_t *v, uint32_t item)
{
	if(v->total >= v->capacity-1){
		return;
	}
    v->items[v->total++] = item;
    return v->total;
}

void array_set(array_t  *v, int index, uint32_t item)
{
    if (index >= 0 && index < v->total)
        v->items[index] = item;
}

uint32_t array_get(array_t  *v, int index)
{
    if (index >= 0 && index < v->total)
        return v->items[index];
    return NULL;
}

void array_delete(array_t  *v, int index)
{
    if (index < 0 || index >= v->total)
        return;

    v->items[index] = NULL;

    for (int i = index; i < v->total - 1; i++) {
        v->items[i] = v->items[i + 1];
        v->items[i + 1] = NULL;
    }

    v->total--;
}

void array_free(array_t  *v)
{
    kfree(v->items);
    kfree(v);
}
