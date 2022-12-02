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
#define BEGIN_NUMBER 90 // substract by 5 every anchor node, start from 95
#define ANCHOR_NUM 2 // the number of anchor node
#define ANCHOR1_RESPONSE 11
#define ANCHOR2_RESPONSE 22
#define SENT_MESSAGE_NUM 2
uint32_t time_difference[ANCHOR_NUM] = {0,0}; // number of 0 equal to number of anchor nodes - ANCHOR_NUM
EventQueue queue(4 * EVENTS_EVENT_SIZE);
uint8_t seq = BEGIN_NUMBER;

// Time-difference measure
uint32_t send_time = 0;
uint32_t receive_time = 0;
uint32_t mode_changing_time = 0;

uint8_t response_countdown = 10; // the number of sample * the number of anchor nodes. 2 anchor nodes * 15 samples = 30
uint8_t send_countdown = SENT_MESSAGE_NUM*ANCHOR_NUM; // 5*n with n = the number of anchor nodes

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
  htim2.Init.Prescaler = 0;//0x40;
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

void tx_test()
{

    Radio::radio.tx_buf[0] = seq++;  /* set payload */
    Radio::Send(1, 0, 0, 0);   /* begin transmission */
    printf("sent %d\n", Radio::radio.tx_buf[0]);
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
}

void txDoneCB()
{
    send_time = Radio::irqAt_ns;//__HAL_TIM_GET_COUNTER(&htim2);
    send_countdown--;

    printf("send_countdown: %d\n", send_countdown);
    if ((send_countdown == 0) || (send_countdown == 2)) // add more expression if there are more anchor node
    {
        Radio::Rx(0);
        // uint32_t shift_time = __HAL_TIM_GET_COUNTER(&htim2);
        // printf("shift_time: %" PRIu32 "\n", shift_time);
        // mode_changing_time = shift_time - send_time;
        // printf("mode_changing_time: %" PRIu32 "\n", mode_changing_time);
    } else {
        queue.call_in(100, tx_test);
    }
}

void rxDoneCB(uint8_t size, float rssi, float snr)
{
    //printf("Done receiving time: %llu\n", Radio::irqAt);
    if (Radio::radio.rx_buf[0] == ANCHOR1_RESPONSE) // anchor node 1
    {
        receive_time = Radio::irqAt_ns;//__HAL_TIM_GET_COUNTER(&htim2);
        time_difference[0] = (receive_time - send_time)/2;
        printf("received from anchor1 \n");
    }

    if (Radio::radio.rx_buf[0] == ANCHOR2_RESPONSE) // anchor node 2
    {
        receive_time = Radio::irqAt_ns;//__HAL_TIM_GET_COUNTER(&htim2);
        time_difference[1] = (receive_time - send_time)/2;
        printf("received from anchor2 \n");
    }

    if(send_countdown == 0)
    {
        printf(" ,");
        for (int i = 0; i < ANCHOR_NUM; i++)
        {
            printf("%u,", time_difference[i]);
            time_difference[i] = 0;
        }
        response_countdown--;
    } else {
        queue.call_in(100, tx_test);
    }
    
    if ((response_countdown != 0) && (send_countdown == 0))
    {
        Radio::radio.tx_buf[0] = 0;  /* set payload */
        Radio::Send(1, 0, 0, 0);
        send_countdown = SENT_MESSAGE_NUM*ANCHOR_NUM + 1;
        seq = BEGIN_NUMBER;
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