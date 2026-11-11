/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
 * All rights reserved.</center></h2>
 *
 * This software component is licensed by ST under BSD 3-Clause license,
 * the "License"; You may not use this file except in compliance with the
 * License. You may obtain a copy of the License at:
 *                        opensource.org/licenses/BSD-3-Clause
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "dma.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static volatile uint8_t  result_ready = 0;
static volatile uint16_t result_lower = 0, result_upper = 0;
static char result_str[40];
static char tx_msg[2][96];
volatile uint32_t ms_ticks = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
void proccesDmaData(const uint8_t* data, uint16_t len);
static void send_text(char *buf, int len);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void)
{
	/* USER CODE BEGIN 1 */

	/* USER CODE END 1 */


	/* MCU Configuration--------------------------------------------------------*/

	/* Reset of all peripherals, Initializes the Flash interface and the Systick. */

	LL_APB2_GRP1_EnableClock(LL_APB2_GRP1_PERIPH_SYSCFG);
	LL_APB1_GRP1_EnableClock(LL_APB1_GRP1_PERIPH_PWR);

	NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_4);

	/* System interrupt init*/

	/* USER CODE BEGIN Init */

	/* USER CODE END Init */

	/* Configure the system clock */
	SystemClock_Config();

	/* USER CODE BEGIN SysInit */

	/* USER CODE END SysInit */

	/* Initialize all configured peripherals */
	MX_GPIO_Init();
	MX_DMA_Init();
	MX_USART2_UART_Init();
	/* USER CODE BEGIN 2 */
	USART2_RegisterCallback(proccesDmaData);
	LL_SYSTICK_EnableIT();

	/* USER CODE END 2 */

	/* Infinite loop */
	/* USER CODE BEGIN WHILE */
	uint32_t last_report = 0;
	while (1)
	{
		/* USER CODE END WHILE */
#if POLLING
		USART2_CheckDmaReception();
#endif

		if (result_ready)
		{
			result_ready = 0;
			int n = snprintf(tx_msg[0], sizeof(tx_msg[0]),
					"Valid string: %s, Capital letters: %u, Small letters: %u\r\n",
					result_str, result_upper, result_lower);
			send_text(tx_msg[0], n);
		}

		if ((ms_ticks - last_report) >= 1000)
		{
			last_report += 1000;
			uint16_t used = DMA_USART2_BUFFER_SIZE - LL_DMA_GetDataLength(DMA1, LL_DMA_CHANNEL_6);
			int n = snprintf(tx_msg[1], sizeof(tx_msg[1]),
					"Buffer capacity: %u bytes, occupied memory: %u bytes, load: %u%%\r\n",
					(unsigned)DMA_USART2_BUFFER_SIZE, (unsigned)used,
					(unsigned)((used * 100) / DMA_USART2_BUFFER_SIZE));
			send_text(tx_msg[1], n);
		}

		/* USER CODE BEGIN 3 */
	}
	/* USER CODE END 3 */
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
void SystemClock_Config(void)
{
	LL_FLASH_SetLatency(LL_FLASH_LATENCY_0);

	if(LL_FLASH_GetLatency() != LL_FLASH_LATENCY_0)
	{
		Error_Handler();
	}
	LL_RCC_HSI_Enable();

	/* Wait till HSI is ready */
	while(LL_RCC_HSI_IsReady() != 1)
	{

	}
	LL_RCC_HSI_SetCalibTrimming(16);
	LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
	LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_1);
	LL_RCC_SetAPB2Prescaler(LL_RCC_APB1_DIV_1);
	LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);

	/* Wait till System clock is ready */
	while(LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI)
	{

	}
	LL_Init1msTick(8000000);
	LL_SYSTICK_SetClkSource(LL_SYSTICK_CLKSOURCE_HCLK);
	LL_SetSystemCoreClock(8000000);
}

/* USER CODE BEGIN 4 */
void proccesDmaData(const uint8_t* data, uint16_t len)
{
	static uint8_t  active = 0;
	static uint8_t  after = 0;
	static uint8_t  idx = 0;
	static uint16_t lower = 0, upper = 0;

	for (uint16_t i = 0; i < len; i++)
	{
		uint8_t c = data[i];

		if (!active)
		{
			if (c == '#')
			{
				active = 1; after = 0; lower = 0; upper = 0;
				idx = 0; result_str[idx++] = '#';
			}
			continue;
		}

		if (c == '$')
		{
			active = 0;
			result_str[idx++] = '$';
			result_str[idx] = '\0';
			result_lower = lower;
			result_upper = upper;
			result_ready = 1;
		}
		else if (after >= 35)
		{
			active = 0;
			if (c == '#')
			{
				active = 1; after = 0; lower = 0; upper = 0;
				idx = 0; result_str[idx++] = '#';
			}
		}
		else
		{
			after++;
			if (c != '\r' && c != '\n')
				result_str[idx++] = c;
			if (c >= 'a' && c <= 'z')      lower++;
			else if (c >= 'A' && c <= 'Z') upper++;
		}
	}
}

static void send_text(char *buf, int len)
{
	while (LL_DMA_IsEnabledChannel(DMA1, LL_DMA_CHANNEL_7)) {}
	USART2_PutBuffer((uint8_t*)buf, len);
}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */
void Error_Handler(void)
{
	/* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */

	/* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
 * @brief  Reports the name of the source file and the source line number
 *         where the assert_param error has occurred.
 * @param  file: pointer to the source file name
 * @param  line: assert_param error line source number
 * @retval None
 */
void assert_failed(char *file, uint32_t line)
{ 
	/* USER CODE BEGIN 6 */
	/* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
	/* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
