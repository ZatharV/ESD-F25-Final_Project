/*
 * TFT-LCD.h
 *
 *  Created on: Nov 21, 2025
 *      Author: Atharv
 */

#ifndef INC_TFT_LCD_H_
#define INC_TFT_LCD_H_

#include "stdint.h"

#define LCD_WIDTH	320
#define LCD_HEIGHT	240

#define LCD_array_size	(LCD_WIDTH * LCD_HEIGHT)

const uint16_t LCD_screen_matrix[LCD_array_size];



#endif /* INC_TFT_LCD_H_ */