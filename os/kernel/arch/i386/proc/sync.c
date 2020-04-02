#include <kernel/proc/proc.h>
#include <kernel/proc/sync.h>


void lock_spin(spinlock_t* mutex){
	while(!__sync_bool_compare_and_swap(mutex, 0, 1))
	{
		asm("pause");
	}
}
void unlock_spin(spinlock_t* mutex){
	__sync_lock_release(mutex);
}
