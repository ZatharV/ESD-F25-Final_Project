/*
 * Snake_game.h
 *
 * Include this file to run Snake game on the GamePal.
 * It contains the main function of the game.
 * 
 *  Created on: Dec 5, 2025
 *      Author: Mayuresh Pitale
 *      Modified by: Atharv More
 */

#ifndef INC_GAMES_SNAKE_GAME_H_
#define INC_GAMES_SNAKE_GAME_H_

#include "main.h"
#include "TFT_LCD.h"

// Snake size
#define SNAKE_BLOCK_SIZE 10

// Can be configured based on LCD size
#define GRID_WIDTH  (LCD_WIDTH / SNAKE_BLOCK_SIZE)
#define GRID_HEIGHT (LCD_HEIGHT / SNAKE_BLOCK_SIZE)

#define C_BLACK   0x0000
#define C_WHITE   0xFFFF
#define C_RED     0xF800
#define C_GREEN   0x07E0
#define C_GREY    0x52AA

void Snake_Game_Init(void);
void Snake_Game_Loop(void);
void snake_gamePlay(void);

#endif /* INC_SNAKE_H_ */
