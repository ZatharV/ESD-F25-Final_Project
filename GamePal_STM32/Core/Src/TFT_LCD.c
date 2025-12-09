/*
 * TFT-LCD.c
 *
 * This file contains functions to interface with the TFT LCD.
 * It contains functions to initialize, write commands and data to LCD.
 * 
 * The font file was provided by STMicroelectronics.
 * It supports Font sizes: 8, 12, 16, 20, 24
 *
 *  Created on: Dec 4, 2025
 *      Author: Atharv More
 * 		Modified by: Mayuresh Pitale
 */

#include "TFT_LCD.h"
#include "Fonts/fonts.h"
#include "stdlib.h"
#include "main.h"

#define LCD_CS_LOW()       HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)
#define LCD_CS_HIGH()      HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)

extern SPI_HandleTypeDef hspi3;

// LCD screen matrix for LTDC controller to write to the LCD
uint16_t LCD_screen_matrix[LCD_array_size];

/**
 * @brief Dispays text on the LCD at specified position with given colors
 *
 * @param x_pos     Pixel position in X axis
 * @param y_pos     Pixel position in Y axis
 * @param text      Text to display
 * @param color     Text color
 * @param bgcolor   Background color
 */
void LCD_Display_Text(uint16_t x_pos, uint16_t y_pos, char *text, uint16_t color, uint16_t bgcolor, sFONT* Font)
{
	while(*text)
	{
		// Font array starts from ASCII 32 (' ')
		uint16_t font_table_ind = (*text - 32) * (Font->Height) * (Font->Width / 8 + ((Font->Width % 8) > 0));

		// Adjust according to font size
		for(int h = 0; h < Font->Height; h++)
		{
			uint8_t font_bit_ind = 0;
			for(int i = 0; i < Font->Width; i++)
			{
				// 1 bit per pixel font
				// Map the font bits to LCD pixels with color and bgcolor
				if((Font->table[font_table_ind] >> (7 - font_bit_ind)) & 0x01)
				{
					update_LCD_matrix(x_pos + i, y_pos + h, color);
				}
				else
				{
					update_LCD_matrix(x_pos + i, y_pos + h, bgcolor);
				}
				font_bit_ind++;
				if((font_bit_ind >= 8) || (i == Font->Width - 1))
				{
					font_bit_ind = 0;
					font_table_ind++;
				}
			}
		}
		text++;
		x_pos += Font->Width;
	}
}

/**
 * @brief Clears LCD screen by filling the white color
 * 
 */
void LCD_clear_screen()
{
	for(int i = 0; i < LCD_array_size; i++)
	{
		LCD_screen_matrix[i] = RGB565_WHITE_COLOR;
	}

}


/**
 * @brief Updates the LCD screen matrix at (x,y) position with color
 *
 * @param x_pos Pixel position in X axis
 * @param y_pos Pixel position in Y axis
 * @param color Pixel color
 */
void update_LCD_matrix(uint16_t x_pos, uint16_t y_pos, uint16_t color)
{
    LCD_screen_matrix[y_pos * LCD_WIDTH + x_pos] = color;
}

/**
 * @brief Delay function for LCD setup
 * 
 * @param Delay delay time in ms
 */
void LCD_IO_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}

/**
 * @brief 	Initializes the LCD IO pins and resets the LCD
 * 			Reset sequence as per LCD datasheet
 * 
 */
void LCD_IO_Init()
{
	LCD_CS_HIGH();
	LCD_IO_Delay(10);

	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
	LCD_IO_Delay(1);
	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_RESET);
	LCD_IO_Delay(10);
	HAL_GPIO_WritePin(LCD_RESET_GPIO_Port, LCD_RESET_Pin, GPIO_PIN_SET);
	LCD_IO_Delay(120);

}

/**
 * @brief 	Writes command to the LCD via SPI interface
 * 			LCD expects 9-bit data
 * 			STM32 only supports 8-bit data over SPI
 * 			Padded to 16-bits and sent over SPI
 * 
 * @param command command byte
 */
void LCD_IO_WriteCommand(uint8_t command)
{
	// First bit is 0 to indicate command
	uint8_t SPI_data[2];
	SPI_data[0] = command >> 1;
	SPI_data[1] = command << 7;
	
	LCD_CS_LOW();
	HAL_SPI_Transmit(&hspi3, SPI_data, 2, 100);
	LCD_CS_HIGH();
}

/**
 * @brief 	Writes data to the LCD via SPI interface
 * 			LCD expects 9-bit data
 * 			STM32 only supports 8-bit data over SPI
 * 			Padded to 16-bits and sent over SPI
 * 
 * @param data data byte
 */
void LCD_IO_WriteData(uint8_t data)
{
	// First bit is 1 to indicate data
	uint8_t SPI_msg[2];
	SPI_msg[0] = 0x80 | (data >> 1);
	SPI_msg[1] = data << 7;

	LCD_CS_LOW();
	HAL_SPI_Transmit(&hspi3, SPI_msg, 2, 100);
	LCD_CS_HIGH();
}


