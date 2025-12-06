/*
 * TFT-LCD.c
 *
 *  Created on: Dec 4, 2025
 *      Author: Atharv
 */

#include "TFT_LCD.h"
#include "Fonts/fonts.h"
#include "stdlib.h"
#include "main.h"

#define LCD_CS_LOW()       HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)
#define LCD_CS_HIGH()      HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)

extern SPI_HandleTypeDef hspi3;

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
		uint16_t font_table_ind = (*text - 32) * (Font->Height) * (Font->Width / 8 + ((Font->Width % 8) > 0));
		for(int h = 0; h < Font->Height; h++)
		{
			uint8_t font_bit_ind = 0;
			for(int i = 0; i < Font->Width; i++)
			{
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

void LCD_clear_screen()
{
	for(int i = 0; i < 76800; i++)
	{
	  LCD_screen_matrix[i] = 0xFFFF;
	//		  HAL_Delay(0);
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


void LCD_IO_Delay(uint32_t Delay)
{
  HAL_Delay(Delay);
}

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

void LCD_IO_WriteCommand(uint8_t command)
{
	uint8_t SPI_data[2];
	SPI_data[0] = command >> 1;
	SPI_data[1] = command << 7;
	LCD_CS_LOW();
	HAL_SPI_Transmit(&hspi3, SPI_data, 2, 100);
	LCD_CS_HIGH();

//	uint16_t SPI_msg = (command << 7);
//	LCD_CS_LOW();
//	HAL_SPI_Transmit(&hspi3, (uint8_t*)&SPI_msg, 1, 100);
//	LCD_CS_HIGH();
}

void LCD_IO_WriteData(uint8_t data)
{
	uint8_t SPI_msg[2];
	SPI_msg[0] = 0x80 | (data >> 1);
	SPI_msg[1] = data << 7;
	LCD_CS_LOW();
	HAL_SPI_Transmit(&hspi3, SPI_msg, 2, 100);
	LCD_CS_HIGH();

//	uint16_t SPI_msg = 0x8000 | (data << 7);
//	LCD_CS_LOW();
//	HAL_SPI_Transmit(&hspi3, (uint8_t*)&SPI_msg, 1, 100);
//	LCD_CS_HIGH();
}


