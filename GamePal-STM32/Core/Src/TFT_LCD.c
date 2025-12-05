/*
 * TFT-LCD.c
 *
 *  Created on: Dec 4, 2025
 *      Author: Atharv
 */

#include "TFT_LCD.h"
#include "Fonts/fonts.h"
#include "stdlib.h"

/**
 * @brief Dispays text on the LCD at specified position with given colors
 * 
 * @param x_pos     Pixel position in X axis
 * @param y_pos     Pixel position in Y axis
 * @param text      Text to display
 * @param color     Text color
 * @param bgcolor   Background color
 */
void LCD_Display_Text(uint8_t x_pos, uint8_t y_pos, char *text, uint16_t color, uint16_t bgcolor, sFONT* Font)
{
	uint16_t font_table_ind = 0;

    uint8_t font_bit_ind = 0;
	for(int i = 0; i < Font->Width; i++)
	{
        Font->table[font_table_ind];

        if((Font->table[font_table_ind] >> (7 - font_bit_ind)) & 0x01)
        {
            update_LCD_matrix(x_pos + i, y_pos, color);
        }
        else
        {
            update_LCD_matrix(x_pos + i, y_pos, bgcolor);
        }
        font_bit_ind++;
        if(font_bit_ind >= 8)
        {
            font_bit_ind = 0;
            font_table_ind++;
            x_pos++;
        }
	}
    font_bit_ind = 0;
    font_table_ind++;
    x_pos -= Font->Width;
    y_pos++;
}


/**
 * @brief Updates the LCD screen matrix at (x,y) position with color
 * 
 * @param x_pos Pixel position in X axis
 * @param y_pos Pixel position in Y axis
 * @param color Pixel color
 */
void update_LCD_matrix(uint8_t x_pos, uint8_t y_pos, uint16_t color)
{
    LCD_screen_matrix[y_pos * LCD_WIDTH + x_pos] = (uint8_t)color;
}



