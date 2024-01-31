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
uint8_t snakeRingbuffer[256];
// mabye use overflow
uint16_t snakeBitHead = 0;
uint16_t snakeBitTail = 0;

Position pos_SnakeHead;
Position pos_OldSnakeHead;
Direction oldDirection = JS_RIGHT;
Direction currentDirection = JS_RIGHT;

Position pos_Food;
uint16_t momScore=0;
uint16_t highScore=0;

/*  funktionen fuer Snake */

void checkDircetion(){
	oldDirection = currentDirection;
	if(js_getDirection()!= JS_NEUTRAL){
		currentDirection = js_getDirection();
	}
}

void setDirection(uint16_t snakeBit, Direction direction){
	uint8_t directionInBin=0;
	switch(direction){
		case JS_NEUTRAL: break;
		case JS_DOWN: directionInBin = 0;break;
		case JS_UP: directionInBin = 1;break;
		case JS_LEFT: directionInBin = 2;break;
		case JS_RIGHT: directionInBin = 3;break;
	}
	snakeRingbuffer[snakeBit/4] = snakeRingbuffer[snakeBit/4] + (directionInBin << (snakeBit%4)*2);
}
Direction getDirection(uint16_t snakeBit){
	uint8_t directionInBin=0;
	directionInBin= (snakeRingbuffer[snakeBit/4]>>(snakeBit%4)*2)& 0b00000011;
	switch(directionInBin){
		case 0: return JS_DOWN;
		case 1: return JS_UP;
		case 2: return JS_LEFT;
		case 3: return JS_RIGHT;
		default: draw_setPixel(0,0,COLOR_YELLOW); break;
	}
	return JS_NEUTRAL;
}

void drawSnake(){
	pos_SnakeHead.cord_X=16;
	pos_SnakeHead.cord_Y=16;
	snakeBitHead =5;
	for(uint8_t i =0; i<=snakeBitHead; i++){
		setDirection(i,JS_RIGHT);
		draw_setPixel(pos_SnakeHead.cord_X-i,pos_SnakeHead.cord_Y,COLOR_GREEN);
	}
	
}

void spawnFood(){
	pos_Food.cord_X = rand() % 32;
	pos_Food.cord_Y = (rand() % 25)+7;
	draw_setPixel(pos_Food.cord_X,pos_Food.cord_Y,COLOR_RED);
}

void deathMessage(){
	draw_letter('Y',7,5,COLOR_RED,true,true);
	draw_letter('O',13,5,COLOR_RED,true,true);
	draw_letter('U',19,5,COLOR_RED,true,true);
	draw_letter('D',5,14,COLOR_RED,true,true);
	draw_letter('I',10,14,COLOR_RED,true,true);
	draw_letter('E',15,14,COLOR_RED,true,true);
	draw_letter('D',22,14,COLOR_RED,true,true);
	delayMs(700);
}

void resetGame(){
	draw_clearDisplay();
	snakeBitTail = 0;
	momScore=0;
	oldDirection = JS_RIGHT;
	currentDirection = JS_RIGHT;
	for(uint8_t i=0;i<255;i++){
		snakeRingbuffer[i]=0;
	}
	deathMessage();
	draw_clearDisplay();
	drawSnake();
	spawnFood();
}

bool checkGameOver(){
	Position posSnakeBit;
	posSnakeBit.cord_X = pos_SnakeHead.cord_X;
	posSnakeBit.cord_Y = pos_SnakeHead.cord_Y;
	for(uint16_t i = snakeBitHead; i>snakeBitTail-1; i--){
		switch (getDirection(i)){
			case JS_DOWN:posSnakeBit.cord_Y--; break;
			case JS_UP:posSnakeBit.cord_Y++; break;
			case JS_LEFT:posSnakeBit.cord_X++; break;
			case JS_RIGHT:posSnakeBit.cord_X--; break;
			default:return false;
		}
		if(posSnakeBit.cord_X == pos_SnakeHead.cord_X&&posSnakeBit.cord_Y == pos_SnakeHead.cord_Y){
			return true;
		}
	}
	return false;
	
}

void checkEatenFood(){
	if(pos_SnakeHead.cord_X==pos_Food.cord_X&&pos_SnakeHead.cord_Y==pos_Food.cord_Y){
		snakeBitHead=(snakeBitHead+1)%1024;
		momScore ++;
		spawnFood();
		if(momScore>highScore){
			highScore=momScore;
		}
	}
	else{
		snakeBitHead=(snakeBitHead+1)%1024;
		snakeBitTail=(snakeBitTail+1)%1024;
	}
	setDirection(snakeBitHead,currentDirection);
}

Position posOfLastSnakeBit(){
	Position posLastSnakeBit;
	posLastSnakeBit.cord_X = pos_SnakeHead.cord_X;
	posLastSnakeBit.cord_Y = pos_SnakeHead.cord_Y;
	/*
	for(uint16_t i = snakeBitHead; i>snakeBitTail-1; i--){
		switch (getDirection(i)){
			case JS_DOWN:posLastSnakeBit.cord_Y--; break;
			case JS_UP:posLastSnakeBit.cord_Y++; break;
			case JS_LEFT:posLastSnakeBit.cord_X++; break;
			case JS_RIGHT:posLastSnakeBit.cord_X--; break;
			default:draw_setPixel(0,0,COLOR_YELLOW);break;
		}
	}
	*/
	uint16_t testHead = snakeBitHead;
	uint16_t testTail = snakeBitTail;
	while(testHead!=testTail-1){
		switch (getDirection(testHead)){
			case JS_DOWN:posLastSnakeBit.cord_Y--; break;
			case JS_UP:posLastSnakeBit.cord_Y++; break;
			case JS_LEFT:posLastSnakeBit.cord_X++; break;
			case JS_RIGHT:posLastSnakeBit.cord_X--; break;
			default:draw_setPixel(0,0,COLOR_YELLOW);break;
		}
		if(testHead!=0){
			testHead = (testHead-1)%1024;
		}
		else{
			testHead = 1024;
		}

	}
	return posLastSnakeBit;
}

void drawSnakeBits(){
	if(snakeBitHead==snakeBitTail){
		draw_setPixel(pos_OldSnakeHead.cord_X,pos_OldSnakeHead.cord_Y,COLOR_BLACK);
	}
	else{
		draw_setPixel(posOfLastSnakeBit().cord_X,posOfLastSnakeBit().cord_Y,COLOR_BLACK);
	}
	delayMs(100);
	draw_setPixel(pos_SnakeHead.cord_X,pos_SnakeHead.cord_Y,COLOR_GREEN);
}

void snakeMove(){
	pos_OldSnakeHead = pos_SnakeHead;
	switch (currentDirection){
		case JS_NEUTRAL: currentDirection = oldDirection; snakeMove();return;
		case JS_DOWN:if(pos_SnakeHead.cord_Y==31){resetGame();return;}
			if(oldDirection==JS_UP){
				currentDirection = oldDirection; snakeMove();return;
			}
			else{pos_SnakeHead.cord_Y++;} break;
		case JS_UP:if(pos_SnakeHead.cord_Y==7){resetGame();return;}
			if(oldDirection==JS_DOWN){
				currentDirection = oldDirection; snakeMove();return;
			}
			else{pos_SnakeHead.cord_Y--;} break;
		case JS_LEFT:if(pos_SnakeHead.cord_X==0){resetGame();return;}
			if(oldDirection==JS_RIGHT){
				currentDirection = oldDirection; snakeMove();return;
			}
			else{pos_SnakeHead.cord_X--;} break;
		case JS_RIGHT:if(pos_SnakeHead.cord_X==31){resetGame();return;}
			if(oldDirection==JS_LEFT){
				currentDirection = oldDirection; snakeMove();return;
			}
			else{pos_SnakeHead.cord_X++;} break;
	}
	if(checkGameOver()){
		resetGame();
		return;
	}
	checkEatenFood();
	drawSnakeBits();
	
}
void drawScores(){
	draw_number(momScore,false,1,1,COLOR_TURQUOISE,true,false);
	draw_number(highScore,false,16,1,COLOR_DARKBLUE,true,false);
	draw_filledRectangle(0,6,32,7, COLOR_PINK);
}

void drawPauseMenu(){
	draw_clearDisplay();
	drawScores();
	draw_letter('P',3,10,COLOR_TURQUOISE,true,true);
	draw_letter('A',8,10,COLOR_DARKGREEN,true,true);
	draw_letter('U',13,10,COLOR_DARKRED,true,true);
	draw_letter('S',18,10,COLOR_PINK,true,true);
	draw_letter('E',23,10,COLOR_YELLOW,true,true);
}
void drawSnakeAfterMenu(){
	draw_clearDisplay();
	Position posSnakeBit;
	posSnakeBit.cord_X = pos_SnakeHead.cord_X;
	posSnakeBit.cord_Y = pos_SnakeHead.cord_Y;
	draw_setPixel(pos_SnakeHead.cord_X,pos_SnakeHead.cord_Y,COLOR_GREEN);
	uint16_t testHead = snakeBitHead;
	uint16_t testTail = snakeBitTail;
	while(testHead!=testTail){
		switch (getDirection(testHead)){
			case JS_DOWN:posSnakeBit.cord_Y--; break;
			case JS_UP:posSnakeBit.cord_Y++; break;
			case JS_LEFT:posSnakeBit.cord_X++; break;
			case JS_RIGHT:posSnakeBit.cord_X--; break;
			default:draw_setPixel(0,0,COLOR_YELLOW);break;
		}
		if(testHead!=0){
			testHead = (testHead-1)%1024;
		}
		else{
			testHead = 1024;
		}
		draw_setPixel(posSnakeBit.cord_X,posSnakeBit.cord_Y,COLOR_GREEN);
	}
	draw_setPixel(pos_Food.cord_X,pos_Food.cord_Y,COLOR_RED);
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
	spawnFood();
	drawSnake();
	while(true){
		while(!js_getButton()){
			drawScores();
			checkDircetion();
			delayMs(100);
			snakeMove();
			delayMs(100);
		}
		delayMs(500);
		drawPauseMenu();
		while(!js_getButton()){}
		drawSnakeAfterMenu();
		delayMs(1000);
	}
}
