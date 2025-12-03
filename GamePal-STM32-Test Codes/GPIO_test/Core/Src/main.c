/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Non-Blocking State Machine for Buttons + ADC Filter
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private variables ---------------------------------------------------------*/
ADC_HandleTypeDef hadc1;

/* USER CODE BEGIN PV */
/* Button State Variables */
uint8_t btn_up = 0, btn_down = 0, btn_left = 0, btn_right = 0;
uint8_t btn_y = 0, btn_a = 0, btn_x = 0, btn_b = 0;

/* Battery Variables */
float battery_voltage = 0.0f;
uint8_t battery_percentage = 0;

/* State Machine Definitions */
typedef enum {
  BTN_STATE_IDLE,
  BTN_STATE_SET_BANK1,
  BTN_STATE_WAIT_BANK1,
  BTN_STATE_READ_BANK1,
  BTN_STATE_INTER_DELAY_1,
  BTN_STATE_SET_BANK2,
  BTN_STATE_WAIT_BANK2,
  BTN_STATE_READ_BANK2,
  BTN_STATE_INTER_DELAY_2
} ButtonState_t;

ButtonState_t current_btn_state = BTN_STATE_IDLE;
uint32_t btn_timer = 0;

/* Timing Constants (Adjust based on 0.1uF Capacitor behavior) */
#define CAP_CHARGE_MS      15  // Time for 0.1uF to charge/discharge (3x - 4x RC)
#define BANK_SWITCH_MS     2   // Safety delay between banks floating high
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_ADC1_Init(void);
void Poll_Buttons_NonBlocking(void);
void Poll_Battery_Filtered(void);

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_ADC1_Init();

  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    // This function checks time and returns immediately - NO BLOCKING
    Poll_Buttons_NonBlocking();

    // Check battery occasionally
    Poll_Battery_Filtered();

    // INSERT GAME LOGIC / DISPLAY UPDATE HERE
    // The CPU is now free to run graphics code while waiting for caps to charge.
  }
  /* USER CODE END 3 */
}

/* USER CODE BEGIN 4 */

/**
  * @brief Non-Blocking State Machine to handle Multiplexed Buttons with Large Caps
  */
void Poll_Buttons_NonBlocking(void)
{
    uint32_t current_time = HAL_GetTick();

    switch (current_btn_state)
    {
        case BTN_STATE_IDLE:
        case BTN_STATE_SET_BANK1:
            // Float Bank 2 (Inactive)
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_SET);
            // Drive Bank 1 Low (Active)
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_RESET);

            btn_timer = current_time;
            current_btn_state = BTN_STATE_WAIT_BANK1;
            break;

        case BTN_STATE_WAIT_BANK1:
            // Wait for 0.1uF caps to discharge/charge
            if ((current_time - btn_timer) >= CAP_CHARGE_MS) {
                current_btn_state = BTN_STATE_READ_BANK1;
            }
            break;

        case BTN_STATE_READ_BANK1:
            // Read Inputs (Active Low)
            // Using simple assignment here; strictly, you could add hysteresis
            btn_up    = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET);
            btn_down  = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET);
            btn_left  = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == GPIO_PIN_RESET);
            btn_right = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == GPIO_PIN_RESET);

            // Disable Bank 1 (Float High)
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_SET);

            btn_timer = current_time;
            current_btn_state = BTN_STATE_INTER_DELAY_1;
            break;

        case BTN_STATE_INTER_DELAY_1:
            // SAFETY DELAY: Allow lines to float up before activating Bank 2
            if ((current_time - btn_timer) >= BANK_SWITCH_MS) {
                current_btn_state = BTN_STATE_SET_BANK2;
            }
            break;

        case BTN_STATE_SET_BANK2:
            // Float Bank 1 (Already done, but ensure)
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8, GPIO_PIN_SET);
            // Drive Bank 2 Low (Active)
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_RESET);

            btn_timer = current_time;
            current_btn_state = BTN_STATE_WAIT_BANK2;
            break;

        case BTN_STATE_WAIT_BANK2:
            if ((current_time - btn_timer) >= CAP_CHARGE_MS) {
                current_btn_state = BTN_STATE_READ_BANK2;
            }
            break;

        case BTN_STATE_READ_BANK2:
            btn_y = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET);
            btn_a = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET);
            btn_x = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_14) == GPIO_PIN_RESET);
            btn_b = (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_15) == GPIO_PIN_RESET);

            // Disable Bank 2 (Float High)
            HAL_GPIO_WritePin(GPIOD, GPIO_PIN_9, GPIO_PIN_SET);

            btn_timer = current_time;
            current_btn_state = BTN_STATE_INTER_DELAY_2;
            break;

        case BTN_STATE_INTER_DELAY_2:
            // Loop back to start
            if ((current_time - btn_timer) >= BANK_SWITCH_MS) {
                current_btn_state = BTN_STATE_SET_BANK1;
            }
            break;
    }
}

/**
  * @brief Reads Battery ADC and applies Low Pass Filter
  */
void Poll_Battery_Filtered(void)
{
    static uint32_t last_bat_check = 0;

    // Only check battery every 500ms (It doesn't change fast)
    if ((HAL_GetTick() - last_bat_check) < 500) return;
    last_bat_check = HAL_GetTick();

    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 10) == HAL_OK)
    {
        uint32_t adc_val = HAL_ADC_GetValue(&hadc1);

        // Raw Voltage Calculation
        // Divider R2(33k) / R3(100k) -> Multiplier 1.33
        float new_voltage = ((float)adc_val * (3.3f / 4095.0f)) * 1.33f;

        // IIR Filter (Low Pass): 90% Old value, 10% New value
        // This removes jitter from the ADC reading
        if (battery_voltage == 0.0f) {
            battery_voltage = new_voltage; // Initialize instantly on first run
        } else {
            battery_voltage = (0.9f * battery_voltage) + (0.1f * new_voltage);
        }

        // Update Percentage
        if (battery_voltage >= 4.2f) battery_percentage = 100;
        else if (battery_voltage <= 3.0f) battery_percentage = 0;
        else {
            battery_percentage = (uint8_t)((battery_voltage - 3.0f) / 1.2f * 100.0f);
        }
    }
    HAL_ADC_Stop(&hadc1);
}

static void MX_ADC1_Init(void)
{
  ADC_ChannelConfTypeDef sConfig = {0};

  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = DISABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK) Error_Handler();

  /* Configure Channel 10 (PC0) */
  sConfig.Channel = ADC_CHANNEL_10;
  sConfig.Rank = 1;
  /* Reduced Sample Time as requested - 144 Cycles is plenty for 100k impedance source */
  sConfig.SamplingTime = ADC_SAMPLETIME_144CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK) Error_Handler();
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /* Select Lines: Open Drain + High Initial State */
  HAL_GPIO_WritePin(GPIOD, GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_SET);

  /* Button Inputs: Pull-Up is REQUIRED for Open Drain logic */
  GPIO_InitStruct.Pin = GPIO_PIN_12|GPIO_PIN_13|GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* Select Lines: Open Drain to prevent short circuits */
  GPIO_InitStruct.Pin = GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* Battery ADC Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_0;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);
}
/* USER CODE END 4 */

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
