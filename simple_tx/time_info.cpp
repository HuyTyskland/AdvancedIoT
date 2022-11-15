#include "time_info.h"

TIM_HandleTypeDef htim10;

void MX_TIM10_Init(void)
{

  /* USER CODE BEGIN TIM10_Init 0 */

  /* USER CODE END TIM10_Init 0 */

  /* USER CODE BEGIN TIM10_Init 1 */

  /* USER CODE END TIM10_Init 1 */
  htim10.Instance = TIM10;
  htim10.Init.Prescaler = 0;
  htim10.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim10.Init.Period = 65535;
  htim10.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  //htim10.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim10) != HAL_OK)
  {
    printf("Error Error\r\n");
  }
  /* USER CODE BEGIN TIM10_Init 2 */

  /* USER CODE END TIM10_Init 2 */

}

void counter_init(void)
{
    __HAL_RCC_TIM10_CLK_ENABLE(); // Enable clock for timer
    HAL_TIM_Base_Start_IT(&htim10);
}

uint32_t return_counter(void)
{
    return __HAL_TIM_GET_COUNTER(&htim10);
}
