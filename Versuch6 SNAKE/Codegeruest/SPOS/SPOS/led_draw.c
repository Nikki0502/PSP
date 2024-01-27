/*! \file
 *  \brief High level functions to draw premade things on the LED Panel
 *  \author   Lehrstuhl Informatik 11 - RWTH Aachen
 *  \date     2019
 */
#include "led_draw.h"
#include "led_paneldriver.h"
#include "led_patterns.h"
#include "util.h"

uint8_t frambuffer[2][16][32];

uint8_t colorToRGB(Color color,uint8_t Ebene){
	uint8_t rgbValue = 0;
	if(((color.r>>(7-Ebene))&0b00000001)==0b00000001){
		rgbValue |= 0b0000001;
	}
	if(((color.g>>(7-Ebene))&0b00000001)==0b00000001){
		rgbValue |= 0b0000010;
	}
	if(((color.b>>(7-Ebene))&0b00000001)==0b00000001){
		rgbValue |= 0b0000100;
	}
	return rgbValue;
		
}
Color rgbToColor(uint8_t rgbValue,uint8_t Ebene){
	Color color = (Color){.r = 0x00, .g = 0x00, .b = 0x00};;
	if((rgbValue&0b00000001)==1){
		color.r=0b00000001<<(7-Ebene);
	}
	if(((rgbValue&0b00000010)>>1)==1){
		color.g=0b00000001<<(7-Ebene);
	}
	if(((rgbValue&0b00000100)>>2)==1){
		color.b=0b00000001<<(7-Ebene);
	}
	return color;
}


//! \brief Distributes bits of given color's channels r, g and b on layers of framebuffer
void draw_setPixel(uint8_t x, uint8_t y, Color color) {
	for(uint8_t i=0;i<2;i++){
		uint8_t rgbValue = colorToRGB(color,i);
		if(y<16){
			frambuffer[i][y][x]=(frambuffer[i][y][x]&0b00111000)+(rgbValue);
		}
		else{
			frambuffer[i][y-16][x]=(frambuffer[i][y-16][x]&0b00000111)+(rgbValue<<3);
		}
	}
}

//! \brief Reconstructs RGB-Color from layers of framebuffer
Color draw_getPixel(uint8_t x, uint8_t y) {
	volatile Color momColor = (Color){.r = 0x00, .g = 0x00, .b = 0x00};
	volatile Color totColor = (Color){.r = 0x00, .g = 0x00, .b = 0x00};
	for(uint8_t i=0;i<2;i++){
		if(y<16){
			momColor = rgbToColor(frambuffer[i][y][x],i);
		}
		else{
			momColor = rgbToColor((frambuffer[i][y-16][x])>>3,i);
		}
		totColor.r += momColor.r;
		totColor.g += momColor.g;
		totColor.b += momColor.b;
	}
	return totColor;
}

//! \brief Fills whole panel with given color
void draw_fillPanel(Color color) {
	for(uint8_t y=0;y<32;y++){
		for(uint8_t x=0;x<32;x++){
			for(uint8_t i=0;i<2;i++){
				uint8_t rgbValue = colorToRGB(color,i);
				if(y<16){
					frambuffer[i][y][x]=(frambuffer[i][y][x]&0b00111000)+(rgbValue);
				}
				else{
					frambuffer[i][y-16][x]=(frambuffer[i][y-16][x]&0b00000111)+(rgbValue<<3);
				}
			}
		}
	}
}

//! \brief Sets every pixel's color to black
void draw_clearDisplay() {
	for(uint8_t i = 0; i < 2; i++){
		for(uint8_t j = 0; j < 16; j++){
			for(uint8_t k = 0; k < 32; k++){
				frambuffer[i][j][k]=0;
			}
		}
	}
}

//! \brief Draws Rectangle
void draw_filledRectangle(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, Color color) {
	for(uint8_t y=y1;y<y2;y++){
		for(uint8_t x=x1;x<x2;x++){
			for(uint8_t i=0;i<2;i++){
				uint8_t rgbValue = colorToRGB(color,i);
				if(y<16){
					frambuffer[i][y][x]=(frambuffer[i][y][x]&0b00111000)+(rgbValue);
				}
				else{
					frambuffer[i][y-16][x]=(frambuffer[i][y-16][x]&0b00000111)+(rgbValue<<3);
				}
			}
		}
	}
}


/*! \brief Draws pattern
 * \param x			Column of left upper corner
 * \param y			Row of left upper corner
 * \param height	Height to be used for the pattern
 * \param width		Width to be used for the pattern
 * \param pattern	The given pattern. Has a maximum span of 8x8 pixel
 * \param color		RGB color used to draw the pattern with
 * \param overwrite Delete pixels in picture that are black in the pattern if set to true
 */
void draw_pattern(uint8_t x, uint8_t y, uint8_t height, uint8_t width, uint64_t pattern, Color color, bool overwrite) {
    uint64_t temprow = 0;
    for (uint8_t i = 0; i < height; i++) {
        // Select row
        temprow = pattern >> i * 8;

        for (uint8_t j = 0; j < width; j++) {
            if ((x + j < 32) && (y + i < 32)) {
                if (temprow & (1 << (7 - j))) {
                    draw_setPixel(x + j, y + i, color);
                } else {
                    if (overwrite) {
                        draw_setPixel(x + j, y + i, (Color){.r = 0, .g = 0, .b = 0});
                    }
                }
            }
        }
    }
}

/*! \brief Draws a character on the panel.
 * \param letter    A character of [0-9A-Za-z]. Note: small letters cannot be drawn, the corresponding capital letter will be drawn instead.
 * \param x         Column of left upper corner
 * \param y         Row of left upper corner
 * \param color     RGB color to draw letter with
 * \param overwrite Delete pixels in picture that are black in the pattern if set to true
 */
void draw_letter(char letter, uint8_t x, uint8_t y, Color color, bool overwrite, bool large) {
    const uint64_t *pattern_table;
    uint8_t idx;

    // the type of character determines how we get our lookup table and index
    if (letter >= '0' && letter <= '9') {
        pattern_table = large ? led_large_numbers : led_small_numbers;
        idx = letter - '0';
    } else if (letter >= 'A' && letter <= 'Z') {
        pattern_table = large ? led_large_letters : led_small_letters;
        idx = letter - 'A';
    }

    else if (letter >= 'a' && letter <= 'z') {
        pattern_table = large ? led_large_letters : led_small_letters;
        idx = letter - 'a';
    } else {
        return;
    }

    // in all cases, we have to fetch our pattern from a progmem array of patterns
    // prepare a SRAM uint64_t value to store the pattern
    uint64_t pattern;

    // Since there is no load function for uint64_ts, we will just use the Flash-to-SRAM version of memcpy
    memcpy_P(&pattern, pattern_table + idx, sizeof(uint64_t));
    draw_pattern(x, y, large ? LED_CHAR_HEIGHT_LARGE : LED_CHAR_HEIGHT_SMALL, large ? LED_CHAR_WIDTH_LARGE : LED_CHAR_WIDTH_SMALL, pattern, color, overwrite);
}


/*! \brief Draws Decimal (0..9) on panel
 * \param dec       Decimal number (from 0 to 9)
 * \param x         Row of left upper corner
 * \param y         Column of left upper corner
 * \param color     Color to draw number with
 * \param overwrite Delete pixels in picture that are black in the pattern if set to true
 * \param large     Draws large numbers when set to true, otherwise small (small: 5x3 px, large: 7x5 px)
 */
void draw_decimal(uint8_t dec, uint8_t x, uint8_t y, Color color, bool overwrite, bool large) {
    draw_letter(dec + '0', x, y, color, overwrite, large);
}

/*! \brief Draw an integer on the panel
 * \param number The number to draw
 * \param right_align if true, the least-significant digit of the number will be drawn at x,y; otherwise, the number will start at this position
 * \param x		   Column of the top-left corner of either the first or the last digit, depending on right_align
 * \param y		   Row of the top-left corner of either the first or the large digit, depending on right_align
 * \param overwrite Delete pixels in picture that are black in the pattern if set to true
 * \param large     Draws large numbers when set to true, otherwise small (small: 5x3 px, large: 7x5 px)
 */
void draw_number(uint32_t number, bool right_align, uint8_t x, uint8_t y, Color color, bool overwrite, bool large) {
    char number_chars[10];
    uint8_t len = 0;

    uint32_t temp = number;

    while (temp >= 10) {
        number_chars[len++] = '0' + (temp % 10);
        temp /= 10;
    }
    number_chars[len++] = '0' + temp;

    const uint8_t diff = (large ? LED_CHAR_WIDTH_LARGE : LED_CHAR_WIDTH_SMALL) + 1;

    // this could potentially be <0 but unsigned integer underflow is defined
    // so that should not be a problem
    // we will simply land somewhere below 256 which will be safely cut off
    // by drawPattern, so the number is just truncated at the left
    if (right_align) x -= diff * (len - 1);
    for (uint8_t i = 0; i < len; i++) {
        draw_letter(number_chars[len - 1 - i], x + i * diff, y, color, overwrite, large);
    }
}
