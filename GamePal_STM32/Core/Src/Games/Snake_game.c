#include "Games/Snake_game/Snake_game.h"
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_flash.h"
#include "stm32f4xx_hal_flash_ex.h"
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "TFT_LCD.h"
#include "User_Interface.h"

// ======================================================
//                 FIX THESE TWO IF NEEDED
// ======================================================
// 1) If your LCD appears rotated (HUD goes to a side), set correctly:
//    1 = LCD is rotated 90° clockwise relative to framebuffer memory
//    0 = no rotation (HUD uses framebuffer coords directly)
#define HUD_LCD_ROT_90_CW   0

// 2) If digits look vertically inverted, flip this:
//    1 = bit0 in glyph column represents TOP pixel (common)
//    0 = bit0 represents BOTTOM pixel
#define FONT_BIT0_IS_TOP    1

// ======================================================
// Flash high score storage (VERIFY these for your MCU/linker!)
// Common for STM32F429 2MB parts: Sector 11 starts at 0x080E0000
#define HIGHSCORE_FLASH_ADDR   ((uint32_t)0x080E0000)
#define HIGHSCORE_FLASH_SECTOR (FLASH_SECTOR_11)
#define HIGHSCORE_MAGIC        ((uint32_t)0x534E4B45) // 'SNKE'
// ======================================================

// --- Global Variables ---
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


// ======================================================
//                 RAW FRAMEBUFFER PIXEL
// ======================================================
static inline void DrawPixelRaw(int x, int y, uint16_t color)
{
    if (x < 0 || x >= LCD_WIDTH || y < 0 || y >= LCD_HEIGHT) return;
    update_LCD_matrix(x, y, color);
}

// ======================================================
// HUD PIXEL IN "VISUAL" COORDS (so it stays at TOP)
// Visual space:
//   if rotated 90CW: VIS_W = LCD_HEIGHT, VIS_H = LCD_WIDTH
//   else:            VIS_W = LCD_WIDTH,  VIS_H = LCD_HEIGHT
// ======================================================
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

static void FillRectHUD(int u, int v, int w, int h, uint16_t color)
{
    if (w <= 0 || h <= 0) return;
    for (int j = 0; j < h; j++)
        for (int i = 0; i < w; i++)
            DrawPixelHUD(u + i, v + j, color);
}

// ======================================================
//                 5x7 FONT (digits + S H : space)
// ======================================================
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

static const uint8_t font5x7_S[5]     = {0x46,0x49,0x49,0x49,0x31};
static const uint8_t font5x7_H[5]     = {0x7F,0x08,0x08,0x08,0x7F};
static const uint8_t font5x7_colon[5] = {0x00,0x36,0x36,0x00,0x00};
static const uint8_t font5x7_space[5] = {0x00,0x00,0x00,0x00,0x00};

static void DrawChar5x7(int u, int v, char c, uint16_t fg, uint16_t bg, int scale)
{
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

#if FONT_BIT0_IS_TOP
            uint8_t on = (bits & (1U << row)) ? 1U : 0U;
#else
            uint8_t on = (bits & (1U << (6 - row))) ? 1U : 0U;
#endif
            uint16_t color = on ? fg : bg;

            for (int dy = 0; dy < scale; dy++)
                for (int dx = 0; dx < scale; dx++)
                    DrawPixelHUD(u + col*scale + dx, v + row*scale + dy, color);
        }
    }
}

static void DrawString5x7(int u, int v, const char *s, uint16_t fg, uint16_t bg, int scale)
{
    while (*s) {
        DrawChar5x7(u, v, *s++, fg, bg, scale);
        u += (5 + 1) * scale;
    }
}

static void DrawUInt(int u, int v, uint32_t val, uint16_t fg, uint16_t bg, int scale)
{
    char buf[11];
    int i = 0;

    if (val == 0) {
        buf[i++] = '0';
    } else {
        char tmp[11];
        int t = 0;
        while (val && t < 10) { tmp[t++] = (char)('0' + (val % 10)); val /= 10; }
        while (t--) buf[i++] = tmp[t];
    }
    buf[i] = 0;
    DrawString5x7(u, v, buf, fg, bg, scale);
}

static void DrawHUD(uint32_t s, uint32_t h)
{
    const int scale = 1;
    const int hudU  = 6;
    const int hudV  = 2;
    const int hudW  = 140;
    const int hudH  = (7 * scale) + 4;

    const uint16_t hud_bg = C_GREY;   // match border

    // Background bar
    FillRectHUD(hudU - 2, hudV - 1, hudW, hudH, hud_bg);

    // IMPORTANT: pass hud_bg as BG for the glyphs too
    DrawString5x7(hudU,        hudV, "S:", C_WHITE, hud_bg, scale);
    DrawUInt     (hudU + 12,   hudV, s,    C_WHITE, hud_bg, scale);

    DrawString5x7(hudU + 70,   hudV, "H:", C_WHITE, hud_bg, scale);
    DrawUInt     (hudU + 82,   hudV, h,    C_WHITE, hud_bg, scale);
}

// ======================================================
//                 FLASH HIGH SCORE
// ======================================================
static uint32_t Flash_ReadHighScore(void)
{
    volatile uint32_t *p = (volatile uint32_t*)HIGHSCORE_FLASH_ADDR;
    if (p[0] != HIGHSCORE_MAGIC) return 0;
    return p[1];
}

static void Flash_WriteHighScore(uint32_t hs)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef erase = {0};
    uint32_t sectorError = 0;

    erase.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    erase.Sector       = HIGHSCORE_FLASH_SECTOR;
    erase.NbSectors    = 1;

    if (HAL_FLASHEx_Erase(&erase, &sectorError) == HAL_OK) {
        (void)HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, HIGHSCORE_FLASH_ADDR + 0, HIGHSCORE_MAGIC);
        (void)HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, HIGHSCORE_FLASH_ADDR + 4, hs);
    }

    HAL_FLASH_Lock();
}

static void CommitHighScoreIfNeeded(void)
{
    if (gameover_saved) return;
    gameover_saved = 1;

    if ((uint32_t)score > high_score) {
        high_score = (uint32_t)score;
        Flash_WriteHighScore(high_score);
    }
}

// ======================================================
//                 GAME DRAW (RAW GRID)
// ======================================================
void DrawBlock(int x, int y, uint16_t color)
{
    if (x < 0 || x >= GRID_WIDTH || y < 0 || y >= GRID_HEIGHT) return;

    int startX = x * SNAKE_BLOCK_SIZE;
    int startY = y * SNAKE_BLOCK_SIZE;

    for (int i = 0; i < SNAKE_BLOCK_SIZE; i++) {
        for (int j = 0; j < SNAKE_BLOCK_SIZE; j++) {
            DrawPixelRaw(startX + j, startY + i, color);
        }
    }
}

// --- Buttons ---
void Init_Buttons_GPIO(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_GPIOE_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitStruct.Pin  = GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin   = GPIO_PIN_6;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_14;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
}

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

//    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6,  GPIO_PIN_SET);
//    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);
//    HAL_Delay(1);
//
//    // Your corrected mapping (keep as-is)
//    if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2)) {
//        if (dir != 4) dir = 3;
//    } else if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_5)) {
//        if (dir != 3) dir = 4;
//    } else if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_3)) {
//        if (dir != 1) dir = 2;
//    } else if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_4)) {
//        if (dir != 2) dir = 1;
//    }
//
//    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6,  GPIO_PIN_RESET);
//    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_RESET);
}

// --- Fruit: never spawn on the snake ---
void Generate_Fruit(void)
{
    int ok = 0;

    while (!ok) {
        fruitX = (rand() % (GRID_WIDTH  - 2)) + 1;
        fruitY = (rand() % (GRID_HEIGHT - 2)) + 1;

        ok = 1;
        for (int i = 0; i < snakeLen; i++) {
            if (snakeX[i] == fruitX && snakeY[i] == fruitY) {
                ok = 0;
                break;
            }
        }
    }
}

void Snake_Setup(void)
{
    snakeLen = 3;
    headX = GRID_WIDTH / 2;
    headY = GRID_HEIGHT / 2;

    dir = 0;
    gameOver = 0;
    score = 0;
    gameover_saved = 0;

    high_score = Flash_ReadHighScore();

    snakeX[0] = headX; snakeY[0] = headY;
    snakeX[1] = headX; snakeY[1] = headY + 1;
    snakeX[2] = headX; snakeY[2] = headY + 2;

    memset(LCD_screen_matrix, 0x00, sizeof(uint16_t) * LCD_WIDTH * LCD_HEIGHT);

    Generate_Fruit();
}

void Snake_Logic(void)
{
    if (dir == 0 || gameOver) return;

    int oldTailX = snakeX[snakeLen - 1];
    int oldTailY = snakeY[snakeLen - 1];

    // Move body
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
    snakeX[0] = headX;
    snakeY[0] = headY;

    // Wall collision
    if (headX <= 0 || headX >= GRID_WIDTH - 1 || headY <= 0 || headY >= GRID_HEIGHT - 1) {
        gameOver = 1;

        CommitHighScoreIfNeeded();
        return;
    }

    // Self collision
    for (int i = 1; i < snakeLen; i++) {
        if (snakeX[i] == headX && snakeY[i] == headY) {
            gameOver = 1;
            CommitHighScoreIfNeeded();
            return;
        }
    }

    // Fruit
    if (headX == fruitX && headY == fruitY) {
        score += 10;

        // Clean growth: append old tail to new segment
        if (snakeLen < 199) {
            snakeX[snakeLen] = oldTailX;
            snakeY[snakeLen] = oldTailY;
            snakeLen++;
        }

        Generate_Fruit();
        // Don't clear tail this frame
    } else {
        // Normal move: clear old tail block
        DrawBlock(oldTailX, oldTailY, 0x0000);
    }
}

void Snake_Draw(void)
{
    // Walls
    for (int x = 0; x < GRID_WIDTH; x++) {
        DrawBlock(x, 0, C_GREY);
        DrawBlock(x, GRID_HEIGHT - 1, C_GREY);
    }
    for (int y = 0; y < GRID_HEIGHT; y++) {
        DrawBlock(0, y, C_GREY);
        DrawBlock(GRID_WIDTH - 1, y, C_GREY);
    }

    // Fruit
    DrawBlock(fruitX, fruitY, C_RED);

    // Snake
    for (int i = 0; i < snakeLen; i++) {
        if (i == 0) DrawBlock(snakeX[i], snakeY[i], C_GREEN);
        else        DrawBlock(snakeX[i], snakeY[i], C_WHITE);
    }

    // Game over border
    if (gameOver) {
        for (int x = 0; x < GRID_WIDTH; x++) {
            DrawBlock(x, 0, C_RED);
            DrawBlock(x, GRID_HEIGHT - 1, C_RED);
        }
    }

    // HUD at VISUAL TOP
    DrawHUD((uint32_t)score, high_score);
}

// --- Public ---
void Snake_Game_Init(void)
{
    Init_Buttons_GPIO();
    Snake_Setup();
}

void Snake_Game_Loop(void)
{
    if (!gameOver) {
        delay_count++;
        Poll_Input();

        if (delay_count == 10) {
            delay_count = 0;
            Snake_Logic();
            Snake_Draw();
        }

        HAL_Delay(8);
    } else {
        CommitHighScoreIfNeeded();

        HAL_Delay(200);

        // Restart on button press
        if (HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_2) || HAL_GPIO_ReadPin(GPIOE, GPIO_PIN_5)) {
            Snake_Setup();
        }
    }
}

void snake_gamePlay(void)
{
	exit_game = 0;
	while(!exit_game)
	{
		Snake_Setup();

		while (gameOver == 0) {
			Snake_Game_Loop();
		}

		dir = 0;
		while (dir == 0) {
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
