/*
 * T_rex.h
 *
 * Include this file to run T-rex game on the GamePal.
 * It contains the main function of the game.
 *
 *  Created on: Dec 5, 2025
 *      Author: Atharv More
 *      Modified by: Mayuresh Pitale
 */

#ifndef INC_GAMES_T_REX_H_
#define INC_GAMES_T_REX_H_

#include "stdint.h"
#include "Images.h"

void T_rex_main();

#define Dino_Track_pos_x	0
#define Dino_Track_pos_y	150

#define Dino_run_img_update_time	10	//(in delay time)
#define Dino_game_delay_time		10

#define Dino_default_pos_x	40
#define Dino_default_pos_y	Dino_Track_pos_y - dino_run1_height

#define Dino_cactus_pos_y	Dino_Track_pos_y - dino_cactus_height
#define Dino_cactus_update_time	1

#endif /* INC_GAMES_T_REX_H_ */
