/*
 * T_rex.h
 *
 *  Created on: Dec 5, 2025
 *      Author: Atharv
 */

#ifndef INC_GAMES_T_REX_H_
#define INC_GAMES_T_REX_H_

#include "stdint.h"
#include "Images.h"

void T_rex_main();
uint8_t get_input();
void menu_screen();
void clear_screen();
void dino_run();

#define Dino_Track_pos_x	0
#define Dino_Track_pos_y	150

#define Dino_run_img_update_time	10	//(in delay time)
#define Dino_game_delay_time		10

#define Dino_default_pos_x	40
#define Dino_default_pos_y	Dino_Track_pos_y - dino_run1_height

#define Dino_cactus_pos_y	Dino_Track_pos_y - dino_cactus_height
#define Dino_cactus_update_time	1

#endif /* INC_GAMES_T_REX_H_ */
