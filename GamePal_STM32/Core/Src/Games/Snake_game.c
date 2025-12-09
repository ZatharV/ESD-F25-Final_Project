/*
 * Snake_game.c
 *
 * This file contains the Snake game.
 * It has functions for setting up the game, main game loop and snake logic.
 * 
 * The game is a simple snake growth game.
 * The player controls a snake that needs to eat apples.
 * Each apple increases the length of the snake by 1 block.
 * The game ends if the snake collides with the walls or itself.
 * Each apple eaten increases the score by 10 points.
 * The high score is stored in flash.
 * 
 * Controls:
 * - D-pad to move the snake
 * - SW_A: To restart the game after game over
 * - SW_B: To exit the game after game over
 *
 *  Created on: Nov 21, 2025
 *      Author: Mayuresh Pitale
 *	    Modified by: Atharv More
 */

#include "Games/Snake_game/Snake_game.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "TFT_LCD.h"
#include "User_Interface.h"

// Rotates the Screen by 90 degrees clockwise
#define HUD_LCD_ROT_90_CW   0

// Flips the text the display in the right orientation
#define FONT_BIT0_IS_TOP    1


// Address space to score high score in flash
#define HIGHSCORE_FLASH_ADDR   ((uint32_t)0x080E0000)
#define HIGHSCORE_FLASH_SECTOR (FLASH_SECTOR_11)
#define HIGHSCORE_MAGIC        ((uint32_t)0x534E4B45)

// snakeX and snakeY store the positions of the snake
int snakeX[200], snakeY[200];
int snakeLen;
int headX, headY;
int fruitX, fruitY;
int dir; // 0=Stop, 1=Up, 2=Down, 3=Left, 4=Right
int gameOver;
int score;
uint8_t delay_count = 0;

uint32_t high_score = 0;
static uint8_t gameover_saved = 0;
static uint8_t exit_game = 0;


// Links the game pixels to the LCD matrix
static inline void DrawPixelRaw(int x, int y, uint16_t color)
{
    if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT) return;
    update_LCD_matrix(x, y, color);
}

// Handles the display rotation functions
#if HUD_LCD_ROT_90_CW
static inline void DrawPixelHUD(int u, int v, uint16_t color)
{
    // Map visual(u,v) -> raw(x,y) for 90° CW rotation
    // raw x = v
    // raw y = (LCD_HEIGHT - 1) - u
    int x = v;
    int y = (LCD_HEIGHT - 1) - u;
    DrawPixelRaw(x, y, color);
}
#else
static inline void DrawPixelHUD(int u, int v, uint16_t color)
{
    DrawPixelRaw(u, v, color);
}
#endif

/**
 * @brief Draws a filled rectangle on the LCD screen
 * 
 * @param u Starting x-position
 * @param v Starting y-position
 * @param w Width of the rectangle
 * @param h Height of the rectangle
 * @param color Color of the rectangle
 */
static void FillRectHUD(int u, int v, int w, int h, uint16_t color)
{
    if (w <= 0 || h <= 0) return;
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            DrawPixelHUD(u + i, v + j, color);
}


/**
 * @brief Font data for all numbers of size 5x7 pixels
 * 
 */
static const uint8_t font5x7_digits[10][5] = {
    {0x3E,0x51,0x49,0x45,0x3E}, // 0
    {0x00,0x42,0x7F,0x40,0x00}, // 1
    {0x42,0x61,0x51,0x49,0x46}, // 2
    {0x21,0x41,0x45,0x4B,0x31}, // 3
    {0x18,0x14,0x12,0x7F,0x10}, // 4
    {0x27,0x45,0x45,0x45,0x39}, // 5
    {0x3C,0x4A,0x49,0x49,0x30}, // 6
    {0x01,0x71,0x09,0x05,0x03}, // 7
    {0x36,0x49,0x49,0x49,0x36}, // 8
    {0x06,0x49,0x49,0x29,0x1E}, // 9
};


// Font data for letters S, H, colon & space of size 5x7 pixels
static const uint8_t font5x7_S[5]     = {0x46,0x49,0x49,0x49,0x31};
static const uint8_t font5x7_H[5]     = {0x7F,0x08,0x08,0x08,0x7F};
static const uint8_t font5x7_colon[5] = {0x00,0x36,0x36,0x00,0x00};
static const uint8_t font5x7_space[5] = {0x00,0x00,0x00,0x00,0x00};

/**
 * @brief Writes the character c at position (u,v) on the LCD
 * 
 * @param u X-position of the character
 * @param v Y-position of the character
 * @param c character to be drawn
 * @param fg font color
 * @param bg background color
 * @param scale Scale factor of the character
 */
static void DrawChar5x7(int u, int v, char c, uint16_t fg, uint16_t bg, int scale)
{
    // Store the glyph data for the character
    const uint8_t *glyph = font5x7_space;

    if (c >= '0' && c <= '9') glyph = font5x7_digits[c - '0'];
    else if (c == 'S') glyph = font5x7_S;
    else if (c == 'H') glyph = font5x7_H;
    else if (c == ':') glyph = font5x7_colon;
    else if (c == ' ') glyph = font5x7_space;

    // 5 columns x 7 rows
    for (int col = 0; col < 5; col++) {
        uint8_t bits = glyph[col];
        for (int row = 0; row < 7; row++) {

            // Flips the character vertically if specified
#if FONT_BIT0_IS_TOP
            uint8_t on = (bits & (1U << row)) ? 1U : 0U;
#else
            uint8_t on = (bits & (1U << (6 - row))) ? 1U : 0U;
#endif
            // Get color of the pixel
            uint16_t color = on ? fg : bg;

            // Scale the character if required
            for (int dy = 0; dy < scale; dy++)
                for (int dx = 0; dx < scale; dx++)
                    DrawPixelHUD(u + col*scale + dx, v + row*scale + dy, color);
        }
    }
}

/**
 * @brief Displays the string at position (u,v) on the LCD
 * 
 * @param u X-position of the string
 * @param v Y-position of the string
 * @param s string pointer
 * @param fg font color
 * @param bg background color
 * @param scale Scale factor of the string
 */
static void DrawString5x7(int u, int v, const char *s, uint16_t fg, uint16_t bg, int scale)
{
    while (*s) {
        DrawChar5x7(u, v, *s++, fg, bg, scale);
        u += (5 + 1) * scale;   // Adjust start position for next character
    }
}

/**
 * @brief Displays interger at position (u,v) on the LCD
 * 
 * @param u X-position of the string
 * @param v Y-position of the string
 * @param val Integer value to be displayed
 * @param fg font color
 * @param bg background color
 * @param scale Scale factor of the text
 */
static void DrawUInt(int u, int v, uint32_t val, uint16_t fg, uint16_t bg, int scale)
{
    char buf[11];
    int i = 0;

    // Convert integer to string and store in buf
    if (val == 0) {
        buf[i++] = '0';
    } else {
        char tmp[11];
        int t = 0;
        while (val && t < 10) { tmp[t++] = (char)('0' + (val % 10)); val /= 10; }
        while (t--) buf[i++] = tmp[t];
    }

    // Null terminate the string
    buf[i] = 0;
    DrawString5x7(u, v, buf, fg, bg, scale);    // Draw the integer string
}

/**
 * @brief Draws the game frame (walls and Scores) on LCD
 * 
 * @param s score
 * @param h high score
 */
static void DrawHUD(uint32_t s, uint32_t h)
{
    const int scale = 1;
    const int hudU  = 6;
    const int hudV  = 2;
    const int hudW  = 140;
    const int hudH  = (7 * scale) + 4;

    const uint16_t hud_bg = C_GREY;   // match border

    // Walls
    FillRectHUD(hudU - 2, hudV - 1, hudW, hudH, hud_bg);

    // Display Score and High Score
    DrawString5x7(hudU,        hudV, "S:", C_WHITE, hud_bg, scale);
    DrawUInt     (hudU + 12,   hudV, s,    C_WHITE, hud_bg, scale);

    DrawString5x7(hudU + 70,   hudV, "H:", C_WHITE, hud_bg, scale);
    DrawUInt     (hudU + 82,   hudV, h,    C_WHITE, hud_bg, scale);
}

/**
 * @brief Reads the high score from flash
 * 
 * @return uint32_t High score
 */
static uint32_t Flash_ReadHighScore(void)
{
    volatile uint32_t *p = (volatile uint32_t*)HIGHSCORE_FLASH_ADDR;
    if (p[0] != HIGHSCORE_MAGIC) return 0;
    return p[1];
}

/**
 * @brief Writes the high score to flash
 * 
 * @param hs high score
 */
static void Flash_WriteHighScore(uint32_t hs)
{
    // Enable flash write
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase = {0};
    uint32_t sectorError = 0;

    erase.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector       = HIGHSCORE_FLASH_SECTOR;
    erase.NbSectors    = 1;

    // To write on flash, need to erase the whole sector first
    if (HAL_FLASHEx_Erase(&erase, &sectorError) == HAL_OK) {
        (void)HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, HIGHSCORE_FLASH_ADDR + 0, HIGHSCORE_MAGIC);
        (void)HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, HIGHSCORE_FLASH_ADDR + 4, hs);
    }

    // Disable flash write
    HAL_FLASH_Lock();
}

/**
 * @brief Compare current score and high score and flash if needed
 * 
 */
static void CommitHighScoreIfNeeded(void)
{
    if (gameover_saved) return;
    gameover_saved = 1;

    // Update high score if needed
    if ((uint32_t)score > high_score) {
        high_score = (uint32_t)score;
        Flash_WriteHighScore(high_score);
    }
}


/**
 * @brief Draws one block of snake or fruit on the LCD
 * 
 * @param x x-pos of the block in the matrix
 * @param y y-pos of the block in the matrix
 * @param color color of the block
 */
void DrawBlock(int x, int y, uint16_t color)
{
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return;

    // Map block (x,y) to LCD pixels
    int startX = x * SNAKE_BLOCK_SIZE;
    int startY = y * SNAKE_BLOCK_SIZE;

    // Draw the block
    for (int i = 0; i < SNAKE_BLOCK_SIZE; i++) {
        for (int j = 0; j < SNAKE_BLOCK_SIZE; j++) {
            DrawPixelRaw(startX + j, startY + i, color);
        }
    }
}

/**
 * @brief Gets user input and maps to the direction variable
 * 
 */
void Poll_Input(void)
{
	// 0=Stop, 1=Up, 2=Down, 3=Left, 4=Right
	read_switch();

	if(SW_up)
	{
		if (dir != 2) dir = 1;
	}
	else if(SW_down)
	{
		if (dir != 1) dir = 2;
	}
	else if(SW_left)
	{
		if (dir != 4) dir = 3;
	}
	else if(SW_right)
	{
		if (dir != 3) dir = 4;
	}
}

/**
 * @brief Randomly generates fruit position not overlapping with the snake
 * 
 */
void Generate_Fruit(void)
{
    int ok = 0;

    while (!ok) {
        // Random position within the grid
        fruitX = (rand() % (GRID_WIDTH  - 2)) + 1;
        fruitY = (rand() % (GRID_HEIGHT - 2)) + 1;

        ok = 1;
        // Check for overlap
        for (int i = 0; i < snakeLen; i++) {
            if (snakeX[i] == fruitX && snakeY[i] == fruitY) {
                ok = 0;
                break;
            }
        }
    }
}

/**
 * @brief Sets up the initial game state
 * 
 */
void Snake_Setup(void)
{
    // Reset all game variables to initial state
    snakeLen = 3;
    headX = GRID_WIDTH / 2;
    headY = GRID_HEIGHT / 2;

    dir = 0;
    gameOver = 0;
    score = 0;
    gameover_saved = 0;

    high_score = Flash_ReadHighScore(); // Load high score from flash

    snakeX[0] = headX; snakeY[0] = headY;
    snakeX[1] = headX; snakeY[1] = headY + 1;
    snakeX[2] = headX; snakeY[2] = headY + 2;

    LCD_clear_screen(); // Clear the LCD screen

    Generate_Fruit();
}

/**
 * @brief Entire game logic for snake movement, growth, and collision detection
 * 
 */
void Snake_Logic(void)
{
    if (dir == 0 || gameOver) return;

    int oldTailX = snakeX[snakeLen - 1];
    int oldTailY = snakeY[snakeLen - 1];

    // Move snake by shifting the array
    for (int i = snakeLen - 1; i > 0; i--) {
        snakeX[i] = snakeX[i - 1];
        snakeY[i] = snakeY[i - 1];
    }

    // Move head
    switch (dir) {
        case 1: headY--; break;
        case 2: headY++; break;
        case 3: headX--; break;
        case 4: headX++; break;
        default: break;
    }
    // New head position
    snakeX[0] = headX;
    snakeY[0] = headY;

    // Check for Wall collision
    if (headX <= 0 || headX >= GRID_WIDTH - 1 || headY <= 0 || headY >= GRID_HEIGHT - 1) {
        gameOver = 1;

        CommitHighScoreIfNeeded();
        return;
    }

    // Check for Self collision
    for (int i = 1; i < snakeLen; i++) {
        if (snakeX[i] == headX && snakeY[i] == headY) {
            gameOver = 1;
            CommitHighScoreIfNeeded();
            return;
        }
    }

    // Check if fruit is eaten
    if (headX == fruitX && headY == fruitY) {
        score += 10;

        // If not max length, grow the snake
        if (snakeLen < 199) {
            snakeX[snakeLen] = oldTailX;
            snakeY[snakeLen] = oldTailY;
            snakeLen++;
        }
        // Doest not clear tail if fruit eaten

        Generate_Fruit();
    } else {
        // Moves the tail if fruit not eaten
        DrawBlock(oldTailX, oldTailY, 0x0000);
    }
}

/**
 * @brief Draw snake and fruit on the LCD
 * 
 */
void Snake_Draw(void)
{
    // Draw Walls
    for (int x = 0; x < GRID_WIDTH; x++) {
        DrawBlock(x, 0, C_GREY);
        DrawBlock(x, GRID_HEIGHT - 1, C_GREY);
    }
    for (int y = 0; y < GRID_HEIGHT; y++) {
        DrawBlock(0, y, C_GREY);
        DrawBlock(GRID_WIDTH - 1, y, C_GREY);
    }

    // Draw Fruit
    DrawBlock(fruitX, fruitY, C_RED);

    // Draw Snake
    for (int i = 0; i < snakeLen; i++) {
        if (i == 0) DrawBlock(snakeX[i], snakeY[i], C_GREEN);
        else        DrawBlock(snakeX[i], snakeY[i], C_WHITE);
    }

    // Indicate Game Over by changing wall color
    if (gameOver) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            DrawBlock(x, 0, C_RED);
            DrawBlock(x, GRID_HEIGHT - 1, C_RED);
        }
    }

    // Display Scores
    DrawHUD((uint32_t)score, high_score);
}

/**
 * @brief Initializes the Snake game
 * 
 */
void Snake_Game_Init(void)
{
    Snake_Setup();
}

/**
 * @brief Main game loop for Snake game
 * 
 */
void Snake_Game_Loop(void)
{
    if (!gameOver) {
        // Polls user input every 8 ms
        // Updates game logic and display every 80 ms
        delay_count++;
        Poll_Input();

        if (delay_count == 10) {
            delay_count = 0;
            Snake_Logic();
            Snake_Draw();
        }

        HAL_Delay(8);
    } 
    else 
    {
        // If game over, commit high score
        CommitHighScoreIfNeeded();

        HAL_Delay(200); // Wait for some time

        // Restart on button press
        if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2) || HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_5)) {
            Snake_Setup();
        }
    }
}

/**
 * @brief   Main function to play the Snake game
 *          Call this function to start the game
 * 
 */
void snake_gamePlay(void)
{
	exit_game = 0;
	while(!exit_game)
	{
        // Game setup
		Snake_Setup();

		while (gameOver == 0) 
        {
            // Main game loop
			Snake_Game_Loop();
		}
        // Exits loop if game is over

		dir = 0;
		while (dir == 0) {
            // Waits for user input to restart or exit
			Poll_Input();
			if(SW_B == 1)
			{
				exit_game = 1;
				break;
			}
			else if(SW_A != 1)
			{
				dir = 0;
			}
			else
			{
				dir = 1;
			}
		}
		HAL_Delay(1000);
	}
}
