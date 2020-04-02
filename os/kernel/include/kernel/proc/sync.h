#pragma once

#include <stdint.h>
#include <kernel/misc/types/queue.h>

typedef uint8_t spinlock_t;

void lock_spin(spinlock_t* mutex);
void unlock_spin(spinlock_t* mutex);

