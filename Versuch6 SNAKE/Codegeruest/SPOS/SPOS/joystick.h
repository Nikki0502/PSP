#ifndef _JOYSTICK_H
#define _JOYSTICK_H

#include <stdbool.h>

typedef enum Direction {
	JS_LEFT,
	JS_RIGHT,
	JS_UP,
	JS_DOWN,
	JS_NEUTRAL
} Direction;

void js_init();
uint16_t js_getHorizontal();
uint16_t js_getVertical();
Direction js_getDirection();
bool os_getButton();

#endif