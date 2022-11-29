#include "radio.h"
#include <cstdint>
#include <cstdio>

#if defined(SX128x_H)
    #define BW_KHZ              200
    #define SPREADING_FACTOR    7
    #define CF_HZ               2487000000
    #define TX_DBM              6
#else
    #if defined(SX126x_H)
        #define TX_DBM              (Radio::chipType == CHIP_TYPE_SX1262 ? 20 : 14)
    #else
        #define TX_DBM              20
    #endif
    #define BW_KHZ              125
    #define SPREADING_FACTOR    12
    #define CF_HZ               868300000
#endif

/**********************************************************************/
EventQueue queue(4 * EVENTS_EVENT_SIZE);
#define DELAY_TIME 90000000
uint8_t seq = 95;

// Time-difference measure
uint32_t send_time = 0;
uint32_t receive_time = 0;
uint32_t mode_changing_time = 0;
uint32_t time_difference = 0;

uint8_t response_countdown = 30;
uint8_t send_countdown = 5;

TIM_HandleTypeDef htim2;
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 0;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 4294967295;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    printf("Error\r\n");
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    printf("Error\r\n");
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    printf("Error\r\n");
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

void rx_test()
{
}

void tx_test()
{

    Radio::radio.tx_buf[0] = seq++;  /* set payload */
    Radio::Send(1, 0, 0, 0);   /* begin transmission */
    // if (send_countdown == 5)
    // {
    //     printf("Start a phase\n");
    // }
    // printf("send_time: %" PRIu32 "\n", send_time);

/*    {
        mbed_stats_cpu_t stats;
        mbed_stats_cpu_get(&stats);
        printf("canDeep:%u ", sleep_manager_can_deep_sleep());
        printf("Uptime: %llu ", stats.uptime / 1000);
        printf("Sleep time: %llu ", stats.sleep_time / 1000);
        printf("Deep Sleep: %llu\r\n", stats.deep_sleep_time / 1000);
    }*/
    if (send_countdown == 0)
    {
        Radio::Rx(0);
        // uint32_t shift_time = __HAL_TIM_GET_COUNTER(&htim2);
        // printf("shift_time: %" PRIu32 "\n", shift_time);
        // mode_changing_time = shift_time - send_time;
        // printf("mode_changing_time: %" PRIu32 "\n", mode_changing_time);
    }
}

void txDoneCB()
{
    send_time = Radio::irqAt_ns;//__HAL_TIM_GET_COUNTER(&htim2);
    send_countdown--;
    queue.call_in(100, tx_test);

    //printf("Done sending time: %llu\n", Radio::irqAt);
}

void rxDoneCB(uint8_t size, float rssi, float snr)
{
    //printf("Done receiving time: %llu\n", Radio::irqAt);
    if (Radio::radio.rx_buf[0] == 100)
    {
        receive_time = Radio::irqAt_ns;//__HAL_TIM_GET_COUNTER(&htim2);
        response_countdown--;
        time_difference = (receive_time - send_time - DELAY_TIME)/2;
        printf("%u, ", time_difference);
    }
    
    if (response_countdown != 0)
    {
        Radio::radio.tx_buf[0] = 0;  /* set payload */
        Radio::Send(1, 0, 0, 0);
        send_countdown = 6;
        seq = 95;
    }
    if (response_countdown == 0)
    {
        printf("DONE RECORDING\n");
    }
    //queue.call_in(500, tx_test);
}


void radio_irq_callback()
{
    Radio::irqAt_ns = __HAL_TIM_GET_COUNTER(&htim2);
    queue.call(Radio::service);
}


const RadioEvents_t rev = {
    /* DioPin_top_half */   radio_irq_callback,
    /* TxDone_topHalf */    NULL,
    /* TxDone_botHalf */    txDoneCB,
    /* TxTimeout  */        NULL,
    /* RxDone  */           rxDoneCB,
    /* RxTimeout  */        NULL,
    /* RxError  */          NULL,
    /* FhssChangeChannel  */NULL,
    /* CadDone  */          NULL
};

int main()
{
    printf("\r\nreset-tx ");

    __HAL_RCC_TIM2_CLK_ENABLE(); // Enable clock for timer
    Radio::Init(&rev);

    Radio::Standby();
    Radio::LoRaModemConfig(BW_KHZ, SPREADING_FACTOR, 1);
    Radio::SetChannel(CF_HZ);

    Radio::set_tx_dbm(TX_DBM);

               // preambleLen, fixLen, crcOn, invIQ
    Radio::LoRaPacketConfig(8, false, true, false);
    MX_TIM2_Init();
    HAL_TIM_Base_Start_IT(&htim2);
    // uint16_t timer_val = __HAL_TIM_GET_COUNTER(&htim2); // get the counter value of timer
    // while (1) {
    //     HAL_Delay(500);
    //     timer_val = __HAL_TIM_GET_COUNTER(&htim2) - timer_val;
    //     printf("\r\ntimer value = %d", timer_val);
    // }

    queue.call_in(500, tx_test);

    queue.dispatch();
}