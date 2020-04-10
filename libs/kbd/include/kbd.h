#pragma once

#define KEY_ACTION_UP   0x00
#define KEY_ACTION_DOWN 0x01

#define MOD_LCTRL  0x01
#define MOD_LCTRL  0x02
#define MOD_LSHIFT 0x04
#define MOD_RSHIFT 0x08

#include <stdint.h>
#include <cheader.h>

CH_START

typedef struct{
	uint32_t keycode; //Keycode
	uint16_t modifiers; //modifiers (shift, ctrl, caps-lock e.t.c)
	uint8_t  state; //Pressed or released
	
	uint8_t key; //Raw key
}key_t;


void kbd_key_event(key_t* key,unsigned char scancode); //fills up key structure

CH_END
