/*
 * TFT-LCD.h
 *
 *  Created on: Nov 21, 2025
 *      Author: Atharv
 */

#ifndef INC_TFT_LCD_H_
#define INC_TFT_LCD_H_

#include "Fonts/fonts.h"
#include "stdint.h"

#define LCD_WIDTH	320
#define LCD_HEIGHT	240

#define LCD_array_size	(LCD_WIDTH * LCD_HEIGHT)

extern uint16_t LCD_screen_matrix[LCD_array_size];

void update_LCD_matrix(uint16_t x_pos, uint16_t y_pos, uint16_t color);
void LCD_Display_Text(uint16_t x_pos, uint16_t y_pos, char *text, uint16_t color, uint16_t bgcolor, sFONT* Font);
void LCD_clear_screen();


#endif /* INC_TFT_LCD_H_ */
