#include <kbd.h>
//Types  
#define common         0x01
#define special        0x02
#define functional     0x03

#define KEY_UP_MASK    0x80
#define KEY_CTRL_MASK  0x40

//keyboard state flags

static char key_type[] = {
	0,    special, common, common, common, common, common, common,
	common, common, common, common, common, common, common, common,
	common, common, common, common, common, common, common, common,
	common, common, common, common, common, special, common, common,
	common, common, common, common, common, common, common, common,
	common, common, special, common, common, common, common, common,
	common, common, common, common, common, common, special, common,
	special, common, special, functional, functional, functional, functional, functional,
	functional, functional, functional, functional, functional, special, special, special,
	special, special, special, special, special, special, special, special,
	special, special, special, special, special, special, special, functional,
	functional, special, special, special, special, special, special, special,
	special, special, special, special, special, special, special, special,
	special, special, special, special, special, special, special, special,
	special, special, special, special, special, special, special, special,
	special, special, special, special, special, special, special, special,
};

//Scancode sets
static char kbd_us_l1[128] = {
	0, 27,
	'1','2','3','4','5','6','7','8','9','0',
	'-','=','\b',
	'\t', /* tab */
	'q','w','e','r','t','y','u','i','o','p','[',']','\n',
	0, /* control */
	'a','s','d','f','g','h','j','k','l',';','\'', '`',
	0, /* left shift */
	'\\','z','x','c','v','b','n','m',',','.','/',
	0, /* right shift */
	'*',
	0, /* alt */
	' ', /* space */
	0, /* caps lock */
	0, /* F1 [59] */
	0, 0, 0, 0, 0, 0, 0, 0,
	0, /* ... F10 */
	0, /* 69 num lock */
	0, /* scroll lock */
	0, /* home */
	0, /* up */
	0, /* page up */
	'-',
	0, /* left arrow */
	0,
	0, /* right arrow */
	'+',
	0, /* 79 end */
	0, /* down */
	0, /* page down */
	0, /* insert */
	0, /* delete */
	0, 0, 0,
	0, /* F11 */
	0, /* F12 */
	0, /* everything else */
};

static char kbd_us_l2[128] = {
	0, 27,
	'!','@','#','$','%','^','&','*','(',')',
	'_','+','\b',
	'\t', /* tab */
	'Q','W','E','R','T','Y','U','I','O','P','{','}','\n',
	0, /* control */
	'A','S','D','F','G','H','J','K','L',':','"', '~',
	0, /* left shift */
	'|','Z','X','C','V','B','N','M','<','>','?',
	0, /* right shift */
	'*',
	0, /* alt */
	' ', /* space */
	0, /* caps lock */
	0, /* F1 [59] */
	0, 0, 0, 0, 0, 0, 0, 0,
	0, /* ... F10 */
	0, /* 69 num lock */
	0, /* scroll lock */
	0, /* home */
	0, /* up */
	0, /* page up */
	'-',
	0, /* left arrow */
	0,
	0, /* right arrow */
	'+',
	0, /* 79 end */
	0, /* down */
	0, /* page down */
	0, /* insert */
	0, /* delete */
	0, 0, 0,
	0, /* F11 */
	0, /* F12 */
	0, /* everything else */
};

static uint8_t keyboard_state_ctrl = 0;
static uint8_t keyboard_state_shift = 0;
//TODO modifier handlers
void kbd_key_event(key_t* key,unsigned char scancode){
	//printf("Processing scancode: 0x%x\n",scancode);
	if (scancode == 0xE0) {
		return;
	}
	if(scancode & KEY_UP_MASK){
		scancode ^= KEY_UP_MASK;
		key->state = KEY_ACTION_UP;
	}else{
		key->state = KEY_ACTION_DOWN;
	}
	switch (key_type[scancode]) {
		case common:
			key->keycode = kbd_us_l1[scancode];
			if(keyboard_state_ctrl){
				int keyc = kbd_us_l1[scancode];
				if(keyc <= 'z' && keyc >= 'a'){
					keyc -= ('a' - 'A');
				}
				if(keyc == '-'){
					keyc = '_';
				}
				if(keyc == '`'){
					keyc = '@';
				}
				keyc -= KEY_CTRL_MASK;
				if(keyc < 0 || keyc > 0x1F){
					key->key = kbd_us_l1[scancode];
				}else{
					key->key = keyc;
				}
			}else{
				key->key     = (keyboard_state_shift)?kbd_us_l2[scancode]:kbd_us_l1[scancode];
			}
		break;
		case special:
			switch(scancode){
				case 0x1D: //lctrl
					keyboard_state_ctrl = key->state;
				break;
				case 0x2A: //lshift
					keyboard_state_shift = key->state;
				break;
				case 0x36: //rshift
					keyboard_state_shift = key->state;
				break;
			}
		break;
		case functional:
		
		break;
	}
}
