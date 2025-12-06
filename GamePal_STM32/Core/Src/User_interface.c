/*
 * User_interface.c
 *
 *  Created on: Dec 5, 2025
 *      Author: Atharv
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



void read_switch()
{
	HAL_GPIO_WritePin(SW_ARR_2_GPIO_Port, SW_ARR_2_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SW_ARR_1_GPIO_Port, SW_ARR_1_Pin, GPIO_PIN_SET);
	HAL_Delay(0);

	SW_up = HAL_GPIO_ReadPin(SW_UP_GPIO_Port, SW_UP_Pin);

	SW_down = HAL_GPIO_ReadPin(SW_DOWN_GPIO_Port, SW_DOWN_Pin);

	SW_left = HAL_GPIO_ReadPin(SW_LEFT_GPIO_Port, SW_LEFT_Pin);

	SW_right = HAL_GPIO_ReadPin(SW_RIGHT_GPIO_Port, SW_RIGHT_Pin);



	HAL_GPIO_WritePin(SW_ARR_1_GPIO_Port, SW_ARR_1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SW_ARR_2_GPIO_Port, SW_ARR_2_Pin, GPIO_PIN_SET);
	HAL_Delay(0);

	SW_Y = HAL_GPIO_ReadPin(SW_UP_GPIO_Port, SW_UP_Pin);

	SW_A = HAL_GPIO_ReadPin(SW_DOWN_GPIO_Port, SW_DOWN_Pin);

	SW_X = HAL_GPIO_ReadPin(SW_LEFT_GPIO_Port, SW_LEFT_Pin);

	SW_B = HAL_GPIO_ReadPin(SW_RIGHT_GPIO_Port, SW_RIGHT_Pin);

	HAL_Delay(0);


	HAL_GPIO_WritePin(SW_ARR_1_GPIO_Port, SW_ARR_1_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(SW_ARR_2_GPIO_Port, SW_ARR_2_Pin, GPIO_PIN_RESET);

}
