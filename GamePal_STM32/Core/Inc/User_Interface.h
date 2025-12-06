/*
 * User_Interface.h
 *
 *  Created on: Dec 5, 2025
 *      Author: Atharv
 */

#ifndef INC_USER_INTERFACE_H_
#define INC_USER_INTERFACE_H_

#include "stdint.h"

extern uint8_t SW_up;
extern uint8_t SW_down;
extern uint8_t SW_left;
extern uint8_t SW_right;

extern uint8_t SW_Y;
extern uint8_t SW_X;
extern uint8_t SW_A;
extern uint8_t SW_B;


void read_switch();

#endif /* INC_USER_INTERFACE_H_ */
