/*
 * User_interface.c
 *
 * This file reads the user input from the switches.
 * The switches are arranged in a 2x4 matrix form.
 * D-Pad switches are 1st row and ABXY switches are 2nd row.
 * 
 * Matrix arrangement:
 *  SW_ARR_1:
 *    SW_UP    SW_DOWN   SW_LEFT   SW_RIGHT
 *  SW_ARR_2:
 * 	  SW_Y     SW_A      SW_X      SW_B
 *
 *  Created on: Dec 5, 2025
 *      Author: Atharv More
 *	    Modified by: Mayuresh Pitale
 */

#include "User_Interface.h"
#include "main.h"

uint8_t SW_up = 0;
uint8_t SW_down = 0;
uint8_t SW_left = 0;
uint8_t SW_right = 0;

uint8_t SW_Y = 0;
uint8_t SW_X = 0;
uint8_t SW_A = 0;
uint8_t SW_B = 0;


/**
 * @brief 	Reads the user input from switches
 * 			Stores in global variables
 * 
 */
void read_switch()
{
	// Activate and read the D-Pad switches
	HAL_GPIO_WritePin(SW_ARR_2_GPIO_Port, SW_ARR_2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SW_ARR_1_GPIO_Port, SW_ARR_1_Pin, GPIO_PIN_SET);
	HAL_Delay(0);

	SW_up = HAL_GPIO_ReadPin(SW_UP_GPIO_Port, SW_UP_Pin);
	SW_down = HAL_GPIO_ReadPin(SW_DOWN_GPIO_Port, SW_DOWN_Pin);
	SW_left = HAL_GPIO_ReadPin(SW_LEFT_GPIO_Port, SW_LEFT_Pin);
	SW_right = HAL_GPIO_ReadPin(SW_RIGHT_GPIO_Port, SW_RIGHT_Pin);


	// Activate and read the ABXY switches
	HAL_GPIO_WritePin(SW_ARR_1_GPIO_Port, SW_ARR_1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SW_ARR_2_GPIO_Port, SW_ARR_2_Pin, GPIO_PIN_SET);
	HAL_Delay(0);

	SW_Y = HAL_GPIO_ReadPin(SW_UP_GPIO_Port, SW_UP_Pin);
	SW_A = HAL_GPIO_ReadPin(SW_DOWN_GPIO_Port, SW_DOWN_Pin);
	SW_X = HAL_GPIO_ReadPin(SW_LEFT_GPIO_Port, SW_LEFT_Pin);
	SW_B = HAL_GPIO_ReadPin(SW_RIGHT_GPIO_Port, SW_RIGHT_Pin);

	HAL_Delay(0);

	// Reset all switch rows
	HAL_GPIO_WritePin(SW_ARR_1_GPIO_Port, SW_ARR_1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SW_ARR_2_GPIO_Port, SW_ARR_2_Pin, GPIO_PIN_RESET);

}
