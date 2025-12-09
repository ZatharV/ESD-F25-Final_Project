/*
 * T-rex_main.c
 *
 * This file contains the dino jump game.
 * It has functions for setting up the game, main game loop,
 * and collision detection.
 * 
 * The game is a simple obstacle avoidance game. 
 * The player controls a dinosaur that runs and must jump over cactus.
 * The speed of the game increases every 1000 points scored.
 * The collision of dino with any cactus ends the game.
 * 
 * Controls:
 * - SW_up: To jump
 * - SW_A: To restart the game after game over
 * - SW_B: To exit the game after game over
 * 
 * References:
 * https://github.com/MaxRohowsky/chrome-dinosaur.git
 * Thanks to MaxRohowsky for providing the images used in this game.
 * The images have been scaled down to 50% to fit in our LCD.
 *
 *  Created on: Nov 21, 2025
 *      Author: Atharv More
 *	    Modified by: Mayuresh Pitale
 */


#include "Games/T_rex/T_rex.h"
#include "TFT_LCD.h"
#include "Fonts/fonts.h"
#include "string.h"
#include "Games/T_rex/Images.h"
#include "User_Interface.h"
#include "stdio.h"
#include "stm32f4xx_hal.h"

#define Dino_Track_pos_y        150

#define Dino_Score_pos_x        0
#define Dino_Score_pos_y        200

#define Dino_High_Score_pos_x   0
#define Dino_High_Score_pos_y   220

#define Score_text_length      20
#define High_Score_text_length 20
#define Score_text_color       0x0000
#define Score_bg_color         0xFFFF

#define Dino_run_img_update_time    10
#define Dino_Jump_pos_y          	20

#define Cactus_out_of_screen_x   -50

#define Dino_Game_Over_delay_ms  2000

#define speed_increase_theshold 1000

#define Dino_jump_lowest_time 4
#define Dino_jump_decreement  2

#define Score_Text_font_size  &Font16


uint8_t dino_run_update_count = 0;
uint8_t dino_cactus_update_count = 0;
uint8_t dino_img_no = 1;

uint32_t dino_score = 0;
uint32_t dino_high_score = 0;

int16_t curr_cactus_pos_x = LCD_WIDTH;

// Dino position and size
uint16_t curr_dino_pos_x = Dino_default_pos_x;
uint16_t curr_dino_pos_y = Dino_default_pos_y;
uint16_t curr_dino_size_x = dino_run1_width;
uint16_t curr_dino_size_y = dino_run1_height;

// Obstacle position and size
uint16_t curr_obstacle_pos_x = 0;
uint16_t curr_obstacle_pos_y = 0;
uint16_t curr_obstacle_size_x = 0;
uint16_t curr_obstacle_size_y = 0;

// Jump control variables
uint16_t dino_jump_time = 0;
uint16_t dino_jump_time_threshold = 10;

uint8_t game_speed = 1;

static uint8_t exit_game = 0;

/**
 * @brief Clears the current dinosaur image on the LCD
 * 
 */
void clear_current_dino()
{
	for (int y = 0; y < dino_run1_height; y++)
	{
		for(int x = 0; x < dino_run1_width; x++)
		{
			update_LCD_matrix(x + curr_dino_pos_x, y + curr_dino_pos_y, 0xFFFF);
		}
	}
}

/**
 * @brief Detects collision between the dinosaur and the obstacle
 * 
 * @return uint8_t 	1 if collision detected
 * 					0 otherwise
 */
uint8_t collision_detection()
{
	// Check the border conditions for collision
	if(((curr_dino_pos_x + curr_dino_size_x) < curr_obstacle_pos_x))
		return 0;
	if(curr_dino_pos_x > (curr_obstacle_pos_x + curr_obstacle_size_x))
		return 0;
	if((curr_dino_pos_y + curr_dino_size_y) < curr_obstacle_pos_y)
		return 0;
	if(curr_dino_pos_y > (curr_obstacle_pos_y + curr_obstacle_size_y))
		return 0;

	return 1;
}

/**
 * @brief Clears current cactus image on the LCD
 * 
 */
void clear_cactus()
{
	for(int y = 0; y < dino_cactus_height; y++)
	{
		for(int x = 0; x < dino_cactus_width; x++)
		{
			// Avoid overflowing the LCD boundaries
			if(curr_cactus_pos_x + x < 320 && curr_cactus_pos_x + x >= 0)
			{
				update_LCD_matrix(curr_cactus_pos_x + x, Dino_cactus_pos_y + y, 0xFFFF);
			}
		}
	}
}

/**
 * @brief Displays the cactus image on the LCD
 * 
 */
void display_cactus_img()
{
	for(int y = 0; y < dino_cactus_height; y++)
	{
		for(int x = 0; x < dino_cactus_width; x++)
		{
			if(curr_cactus_pos_x + x < LCD_WIDTH && curr_cactus_pos_x + x >= 0)
			{
				update_LCD_matrix(curr_cactus_pos_x + x, Dino_cactus_pos_y + y, dino_cactus_img[y*dino_cactus_width + x]);
			}
		}
	}

	curr_obstacle_pos_x = curr_cactus_pos_x;
	curr_obstacle_pos_y = Dino_cactus_pos_y;
	curr_obstacle_size_x = dino_cactus_width;
	curr_obstacle_size_y = dino_cactus_height;
}

/**
 * @brief Displays the dinosaur running image no. 1 on the LCD
 * 
 */
void display_dino_run1_img()
{
	for (int y = 0; y < dino_run1_height; y++)
	{
		for(int x = 0; x < dino_run1_width; x++)
		{
			update_LCD_matrix(x + curr_dino_pos_x, y + curr_dino_pos_y, dino_run1[y*dino_run1_width + x]);
		}
	}
	dino_img_no = 1;

	// Update current dino position and size
	curr_dino_pos_x = Dino_default_pos_x;
	curr_dino_pos_y = Dino_default_pos_y;
	curr_dino_size_x = dino_run1_width;
	curr_dino_size_y = dino_run1_height;
}

/**
 * @brief Displays the dinosaur running image no. 2 on the LCD
 * 
 */
void display_dino_run2_img()
{
	for (int y = 0; y < dino_run2_height; y++)
	{
		for(int x = 0; x < dino_run2_width; x++)
		{
			update_LCD_matrix(x + curr_dino_pos_x, y + curr_dino_pos_y, dino_run2[y*dino_run2_width + x]);
		}
	}
	dino_img_no = 2;
}

/**
 * @brief Displays the track on the LCD
 * 
 */
void display_track()
{
	for(int i = 0; i < dino_track_height * dino_track_width; i++)
	{
		update_LCD_matrix(i, Dino_Track_pos_y, dino_track[i]);
	}

}

/**
 * @brief Game setup for T-Rex game
 * 
 */
void T_rex_setup()
{
	LCD_clear_screen();

	// Reset all game variables to initial state
	dino_run_update_count = 0;
	dino_cactus_update_count = 0;
	dino_img_no = 1;

	dino_score = 0;
	game_speed = 1;

	curr_cactus_pos_x = LCD_WIDTH;

	curr_dino_pos_x = Dino_default_pos_x;
	curr_dino_pos_y = Dino_default_pos_y;
	curr_dino_size_x = dino_run1_width;
	curr_dino_size_y = dino_run1_height;

	curr_obstacle_pos_x = 0;
	curr_obstacle_pos_y = 0;
	curr_obstacle_size_x = 0;
	curr_obstacle_size_y = 0;

	// Display Score on LCD display
	char Score_text[20];
	sprintf(Score_text, "Score: %ld", dino_score);
	LCD_Display_Text(Dino_Score_pos_x, Dino_Score_pos_y, Score_text, Score_text_color, Score_bg_color, Score_Text_font_size);

	// Display High Score on LCD display
	char High_Score_text[20];
	sprintf(High_Score_text, "High Score: %ld", dino_high_score);
	LCD_Display_Text(Dino_High_Score_pos_x, Dino_High_Score_pos_y, High_Score_text, Score_text_color, Score_bg_color, Score_Text_font_size);

	// Display initial screen
	display_track();
	display_dino_run1_img();
}

/**
 * @brief Main game loop for T-Rex game
 * 
 */
void T_rex_loop()
{
	while(1)
	{
		// Read user input at each iteration
		read_switch();

		if(SW_up == 1)
		{
			// If jump button is pressed, initiate jump
			dino_jump_time = dino_jump_time_threshold;
		}

		if(++dino_run_update_count >= Dino_run_img_update_time)
		{
			// Check if the dinosaur is in jump state
			dino_run_update_count = 0;
			if(dino_jump_time > 0)
			{
				// Make the dinosaur jump
				clear_current_dino();
				curr_dino_pos_y = Dino_Jump_pos_y;
				display_dino_run2_img();
				dino_jump_time--;
			}
			else
			{
				// Dino runs on the ground
				clear_current_dino();
				curr_dino_pos_x = Dino_default_pos_x;
				curr_dino_pos_y = Dino_default_pos_y;
				curr_dino_size_x = dino_run2_width;
				curr_dino_size_y = dino_run2_height;

				// Switch between running images
				if(dino_img_no == 1)
				{
					display_dino_run2_img();
				}
				else if(dino_img_no == 2)
				{
					display_dino_run1_img();
				}
			}
		}

		// Update cactus position based on game speed
		if(++dino_cactus_update_count >= Dino_cactus_update_time)
		{
			clear_cactus();
			curr_cactus_pos_x -= game_speed;
			if(curr_cactus_pos_x <= Cactus_out_of_screen_x)
			{
				curr_cactus_pos_x = LCD_WIDTH;
			}
			dino_cactus_update_count = 0;
			display_cactus_img();
		}

		// Detect collision
		if(collision_detection())
		{
			// Update high score if needed
			if(dino_score > dino_high_score)
			{
				dino_high_score = dino_score;
			}
			HAL_Delay(Dino_Game_Over_delay_ms);	// Halt for some time
			break;
		}

		dino_score++;
		char Score_text[Score_text_length];
		sprintf(Score_text, "Score: %ld", dino_score);
		LCD_Display_Text(0, Dino_Score_pos_y, Score_text, Score_text_color, Score_bg_color, &Font16);

		if(dino_score % speed_increase_theshold == 0)
		{
			// Increase game speed every 1000 points scored
			game_speed++;
			if(dino_jump_time >= Dino_jump_lowest_time)
			{
				dino_jump_time_threshold -= Dino_jump_decreement;
			}


		}
		HAL_Delay(Dino_run_img_update_time);
	}
}

/**
 * @brief T-rex game main function
 * 
 */
void T_rex_main()
{
	exit_game = 0;
	while(!exit_game)
	{
		// Setup the game and start the main loop
		T_rex_setup();

		T_rex_loop();

		// If game over, wait for user input to restart or exit
		while(1)
		{
			read_switch();
			if(SW_A)
			{
				break;
			}
			else if(SW_B)
			{
				exit_game = 1;
				break;
			}
			HAL_Delay(50);
		}
	}
}
