/*
 * T-rex_main.c
 *
 *  Created on: Nov 21, 2025
 *      Author: Atharv
 */

#include "Games/T-rex/T-rex_main.h"
#include "TFT-LCD.h"
#include "stdlib.h"
#include "string.h"

#define RUNNING_DINO_IMG_WIDTH  50
#define RUNNING_DINO_IMG_HEIGHT 10

#define JUMPING_DINO_IMG_WIDTH  50
#define JUMPING_DINO_IMG_HEIGHT 10

#define DUCKING_DINO_IMG_WIDTH  50
#define DUCKING_DINO_IMG_HEIGHT 10

#define CACTUS_SMALL_IMG_WIDTH  50
#define CACTUS_SMALL_IMG_HEIGHT 10

#define CACTUS_LARGE_IMG_WIDTH  50
#define CACTUS_LARGE_IMG_HEIGHT 10

#define BIRD_IMG_WIDTH          50
#define BIRD_IMG_HEIGHT         10

#define CLOUD_IMG_WIDTH         50
#define CLOUD_IMG_HEIGHT        10


/*
 * Define all the images in a constant array
 *
 */

const uint16_t dino_running_0[RUNNING_DINO_IMG_WIDTH][RUNNING_DINO_IMG_HEIGHT] = {};
const uint16_t dino_running_1[RUNNING_DINO_IMG_WIDTH][RUNNING_DINO_IMG_HEIGHT] = {};
uint16_t (*dino_running)[RUNNING_DINO_IMG_HEIGHT] = dino_running_0;

const uint16_t jumping[JUMPING_DINO_IMG_WIDTH][JUMPING_DINO_IMG_HEIGHT] = {};

const uint16_t dino_ducking[DUCKING_DINO_IMG_WIDTH][DUCKING_DINO_IMG_HEIGHT] = {};


void T_rex_main()
{
    clear_screen();
    menu_screen();
    while(!get_input());

    dino_run();

}

uint8_t get_input()
{
    return 0;
}

void menu_screen()
{

}

void clear_screen()
{
    memset(LCD_screen_matrix, 0, sizeof(LCD_screen_matrix));
}


void dino_run()
{
}




// dino.c
#include "dino.h"
#include "images.h"
#include "gfx.h"

void dino_init(Dinosaur *d)
{
    d->x = 20;
    d->y_normal = 150;
    d->y_duck = 175;

    d->y = d->y_normal;
    d->jump_velocity = 12;
    d->jumping = 0;
    d->ducking = 0;
    d->frame = 0;
}

void dino_update(Dinosaur *d, uint8_t jumpBtn, uint8_t duckBtn)
{
    if (jumpBtn && !d->jumping) {
        d->jumping = 1;
        d->ducking = 0;
    }

    if (duckBtn && !d->jumping) {
        d->ducking = 1;
    } else {
        d->ducking = 0;
    }

    if (d->jumping) {
        d->y -= d->jump_velocity;
        d->jump_velocity -= 1;

        if (d->jump_velocity < -12) {
            d->jumping = 0;
            d->jump_velocity = 12;
            d->y = d->y_normal;
        }
    }
}

void dino_draw(Dinosaur *d)
{
    const uint16_t *img;

    if (d->jumping) {
        img = DINO_JUMP;
    } else if (d->ducking) {
        img = (d->frame < 5) ? DINO_DUCK_0 : DINO_DUCK_1;
    } else {
        img = (d->frame < 5) ? DINO_RUN_0 : DINO_RUN_1;
    }

    gfx_draw_image(d->x, d->y, 50, 50, img);

    d->frame++;
    if (d->frame >= 10) d->frame = 0;
}



// T_rex_main.c
#include "T_rex_main.h"
#include "gfx.h"
#include "dino.h"
#include "obstacle.h"
#include "cloud.h"
#include "images.h"

uint16_t points = 0;
uint8_t game_speed = 5;

extern uint8_t button_jump();
extern uint8_t button_duck();

void T_rex_main()
{
    Dinosaur dino;
    dino_init(&dino);

    obstacle_init();
    cloud_init();

    while (1)
    {
        uint8_t jump = button_jump();
        uint8_t duck = button_duck();

        dino_update(&dino, jump, duck);
        obstacle_update(game_speed);
        cloud_update(game_speed);

        gfx_clear(0xFFFF);

        cloud_draw();
        obstacle_draw();
        dino_draw(&dino);

        gfx_present();
    }
}


#include "Games/T-rex/T-rex_main.h"
#include "TFT-LCD.h"
#include "Fonts/fonts.h"


void T_rex_main()
{
    memset(LCD_screen_matrix, 0xFF, sizeof(LCD_screen_matrix));
}