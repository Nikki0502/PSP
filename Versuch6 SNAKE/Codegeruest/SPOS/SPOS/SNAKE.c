#include "defines.h"
#include "joystick.h"
#include "lcd.h"
#include "led_draw.h"
#include "led_paneldriver.h"
#include "led_patterns.h"
#include "os_core.h"
#include "os_process.h"
#include "os_scheduler.h"
#include "util.h"

#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <math.h>
#include <stdlib.h>
#include <util/atomic.h>


/************************************************************************/
/*                             SNAKE                                    */
/************************************************************************/

/*  globale Var. */

// Einfachere Darstellung der Position von Stuff
typedef struct{
	uint8_t cord_X;
	uint8_t cord_Y;
}Position;

// SnakeBit Array fuer die Richtungen des Snakebits verwenden wir als RINGBUFFER
// 32*32=1024 =>256 Byte max. mit 2Bit pro SnakeBit => 1 Eintrag enthaelt die Directions von 4 SnakeBit
// erstes Snake bit bei Head und ganz rechts im Eintrag 
uint8_t snakeBitDirections[256];
// mabye use overflow
uint8_t snakeBitHead = 0;
uint8_t snakeBitTail = 0;
uint16_t numberOfSnakeBits = 0;

Position pos_SnakeHead;
Direction oldDirection = JS_NEUTRAL;
Direction currentDirection = JS_NEUTRAL;

Position pos_Food;
uint8_t momScore;
uint8_t highScore;

/*  funktionen fuer Snake */

// funktion um die Direction von einen SnakeBit zu bekommen 
Direction directionOfSnakeBit(uint16_t snakeBit){
	
}
// ringbuffer veraendern passend zu bewegung
// heisst tail und head vergroessern(ausser haben nahrung gegessen dann nur head)
// an neuer head pos im array alte currentDirection einfueghen
void snakeMove(){
	if(pos_SnakeHead.cord_X==0||pos_SnakeHead.cord_Y==0||pos_SnakeHead.cord_X==31||pos_SnakeHead.cord_Y==31||){
		return;
	}
	switch (currentDirection){
		case JS_NEUTRAL: currentDirection = oldDirection; snakeMove();break;
		case JS_DOWN: draw_setPixel(pos_SnakeHead.cord_X,pos_SnakeHead.cord_Y-1, COLOR_WHITE);break;
		case JS_UP: draw_setPixel(pos_SnakeHead.cord_X,pos_SnakeHead.cord_Y+1, COLOR_WHITE);break;
		case JS_LEFT: draw_setPixel(pos_SnakeHead.cord_X-1,pos_SnakeHead.cord_Y, COLOR_WHITE);break;
		case JS_RIGHT: draw_setPixel(pos_SnakeHead.cord_X+1,pos_SnakeHead.cord_Y, COLOR_WHITE);break;
	}
	// ringbuffer stuff
}
// Anhand des Ringbuffers die Positionen der SnakeBits und Kopf berechnen und Drawen  
void drawSnakeBits(){
	
}
Position posOfSnakeBits(uint16_t snakeBit, Position prevSnakeBit){

}
// kuck ob Snake Head gegen Wand oder sich selbst(jede Pos der SnakeBits checkne?)
void checkGameOver(){
	
}

/*  SNAKE */
// muss als ausfuehrbares program im Schedukler sein glaub ich
REGISTER_AUTOSTART(SNAKE)
void SNAKE(void) {
	// Fastest strategy idk
	os_setSchedulingStrategy(OS_SS_RUN_TO_COMPLETION);
	// Init panel and timer nur zur sicherheit
	panel_init();
	panel_initTimer();
	panel_startTimer();
	// init JS
	js_init();
	// Snake
	pos_SnakeHead.cord_X = 0;
	pos_SnakeHead.cord_Y = 0;
	draw_setPixel(0,0, COLOR_WHITE);
	while(true){
		oldDirection = currentDirection;
		currentDirection = js_getDirection();
		snakeMove();
	}
}