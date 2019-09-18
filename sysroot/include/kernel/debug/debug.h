#pragma once

static inline breakpoint(){
	asm("int $3");
}
