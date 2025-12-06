/*
 * T-rex_main.c
 *
 *  Created on: Nov 21, 2025
 *      Author: Atharv
 */


#include "Games/T_rex/T_rex.h"
#include "TFT_LCD.h"
#include "Fonts/fonts.h"
#include "string.h"
#include "Games/T_rex/Images.h"
#include "User_Interface.h"
#include "stdio.h"
#include "stm32f4xx_hal.h"

uint8_t dino_run_update_count = 0;
uint8_t dino_cactus_update_count = 0;
uint8_t dino_img_no = 1;

uint32_t dino_score = 0;
uint32_t dino_high_score = 0;

int16_t curr_cactus_pos_x = LCD_WIDTH;

uint16_t curr_dino_pos_x = Dino_default_pos_x;
uint16_t curr_dino_pos_y = Dino_default_pos_y;
uint16_t curr_dino_size_x = dino_run1_width;
uint16_t curr_dino_size_y = dino_run1_height;

uint16_t curr_obstacle_pos_x = 0;
uint16_t curr_obstacle_pos_y = 0;
uint16_t curr_obstacle_size_x = 0;
uint16_t curr_obstacle_size_y = 0;

uint16_t dino_jump_time = 0;
uint16_t dino_jump_time_threshold = 10;

uint8_t game_speed = 1;

static uint8_t exit_game = 0;

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

uint8_t collision_detection()
{
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

void clear_cactus()
{
	for(int y = 0; y < dino_cactus_height; y++)
	{
		for(int x = 0; x < dino_cactus_width; x++)
		{
			if(curr_cactus_pos_x + x < 320 && curr_cactus_pos_x + x >= 0)
			{
				update_LCD_matrix(curr_cactus_pos_x + x, Dino_cactus_pos_y + y, 0xFFFF);
			}
		}
	}
}

void display_cactus_img()
{
	for(int y = 0; y < dino_cactus_height; y++)
	{
		for(int x = 0; x < dino_cactus_width; x++)
		{
			if(curr_cactus_pos_x + x < 320 && curr_cactus_pos_x + x >= 0)
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

	curr_dino_pos_x = Dino_default_pos_x;
	curr_dino_pos_y = Dino_default_pos_y;
	curr_dino_size_x = dino_run1_width;
	curr_dino_size_y = dino_run1_height;
}

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

void display_track()
{
	for(int i = 0; i < dino_track_height * dino_track_width; i++)
	{
		update_LCD_matrix(i, 150, dino_track[i]);
	}

}

void T_rex_setup()
{
	LCD_clear_screen();
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

	char Score_text[20];
	sprintf(Score_text, "Score: %ld", dino_score);
	LCD_Display_Text(0, 200, Score_text, 0x0000, 0xFFFF, &Font16);

	char High_Score_text[20];
	sprintf(High_Score_text, "High Score: %ld", dino_high_score);
	LCD_Display_Text(0, 220, High_Score_text, 0x0000, 0xFFFF, &Font16);

	display_track();
}

void T_rex_loop()
{
	while(1)
	{
		read_switch();

		if(SW_up == 1)
		{
			dino_jump_time = dino_jump_time_threshold;
		}

		if(++dino_run_update_count >= Dino_run_img_update_time)
		{
			dino_run_update_count = 0;
			if(dino_jump_time > 0)
			{
				clear_current_dino();
				curr_dino_pos_y = 20;
				display_dino_run2_img();
				dino_jump_time--;
			}
			else
			{
				clear_current_dino();
				curr_dino_pos_x = Dino_default_pos_x;
				curr_dino_pos_y = Dino_default_pos_y;
				curr_dino_size_x = dino_run2_width;
				curr_dino_size_y = dino_run2_height;
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

//		LCD_Display_Text(0, 0, "Score", 0x0000, 0xFFFF, &Font24);

		if(++dino_cactus_update_count >= Dino_cactus_update_time)
		{
			clear_cactus();
			curr_cactus_pos_x -= game_speed;
			if(curr_cactus_pos_x <= -50)
			{
				curr_cactus_pos_x = LCD_WIDTH;
			}
			dino_cactus_update_count = 0;
			display_cactus_img();
		}

		if(collision_detection())
		{
			if(dino_score > dino_high_score)
			{
				dino_high_score = dino_score;
			}
			HAL_Delay(2000);
			break;
		}

		dino_score += 1;
		char Score_text[20];
		sprintf(Score_text, "Score: %ld", dino_score);
		LCD_Display_Text(0, 200, Score_text, 0x0000, 0xFFFF, &Font16);

		if(dino_score % 1000 == 0)
		{
			game_speed += 1;
			if(dino_jump_time >= 4)
			{
				dino_jump_time_threshold -= 2;
			}


		}
		HAL_Delay(Dino_run_img_update_time);
	}
}

void T_rex_main()
{
	exit_game = 0;
	while(!exit_game)
	{
		T_rex_setup();

		T_rex_loop();

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
