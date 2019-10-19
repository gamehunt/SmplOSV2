#include <kernel/dev/pit.h>

#include <kernel/interrupts/irq.h>


static uint16_t last_id = 0;
static uint16_t max_id = 0;
static uint32_t __sys_ticks = 0;
uint8_t in_call = 0;
pit_listener_t* pit_listeners[MAX_PIT_LISTENERS];



uint32_t pit_add_listener(pit_listener_t* listener){
	if((uint32_t)pit_listeners[last_id]){
		return 0;
	}
	pit_listeners[last_id] = listener;
	uint32_t id = last_id;
	last_id++;
	while((uint32_t)pit_listeners[last_id] && last_id<MAX_PIT_LISTENERS){
		last_id++;	
	}
	if(last_id > max_id){
		max_id = last_id;
	}
	return id;
}

void pit_remove_listener(uint32_t listener){
	if(listener > MAX_PIT_LISTENERS){
		return;
	}
	kfree(pit_listeners[listener]);
	pit_listeners[listener] = 0;
	last_id = listener;
}

void pit_phase(int hz) {
	int divisor = PIT_SCALE / hz;
	outb(PIT_CONTROL, PIT_SET);
	outb(PIT_A, divisor & PIT_MASK);
	outb(PIT_A, (divisor >> 8) & PIT_MASK);
}

void pit_tick(regs_t r){
		irq_end(0);
		__sys_ticks++;
		for(uint16_t i=0;i<max_id;i++){
			if((uint32_t)pit_listeners[i] && !(__sys_ticks % pit_listeners[i]->time)){
				pit_listeners[i]->handler(r);
			}
		}

	
}

uint32_t pit_system_ticks(){
	
	return __sys_ticks;
}

void  init_pit(){
	irq_set_handler(TIMER_IRQ,pit_tick);
	pit_phase(100);
	
}
