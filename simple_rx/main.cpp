#include "radio.h"
#include <cstdint>

#if defined(SX128x_H)
    #define BW_KHZ              200
    #define SPREADING_FACTOR    7
    #define CF_HZ               2487000000
    #define TX_DBM              6
#else
    #if defined(SX128x_H)
        #define TX_DBM              (Radio::chipType == CHIP_TYPE_SX1262 ? 20 : 14)
    #else
        #define TX_DBM              20
    #endif
    #define BW_KHZ              125
    #define SPREADING_FACTOR    12
    #define CF_HZ               868300000
#endif

DigitalOut myled(LED1);
uint32_t receive_time = 0;
uint32_t send_time = 0;

/**********************************************************************/
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

void txDoneCB_Top()
{
    //printf("tx done\r\n");
    send_time = __HAL_TIM_GET_COUNTER(&htim2);
    //printf("%u, ", send_time - receive_time);
}


void txDoneCB()
{
    //printf("tx done\r\n");
    //send_time = __HAL_TIM_GET_COUNTER(&htim2);
    printf(" %u,%u, ", send_time, receive_time);
    Radio::Rx(0);
}

void first_send()
{
    //HAL_Delay(1000);
    Radio::radio.tx_buf[0] = 100;  /* set payload */
    Radio::Send(1, 0, 0, 0);   /* begin transmission */
    //printf("send first message\r\n");
}

void rxDoneCB(uint8_t size, float rssi, float snr)
{
    // unsigned i;
    // printf("%.1fdBm  snr:%.1fdB\t", rssi, snr);

    // myled.write(!myled.read()); // toggle LED

    // for (i = 0; i < size; i++) {
    //     printf("%02x ", Radio::radio.rx_buf[i]);
    // }
    // printf("\r\n");4
    receive_time = __HAL_TIM_GET_COUNTER(&htim2);
    //printf("received data: %d\r\n", Radio::radio.rx_buf[0]);
    if (Radio::radio.rx_buf[0] == 99)
    {first_send();}
}

const RadioEvents_t rev = {
    /* Dio0_top_half */     NULL,
    /* TxDone_topHalf */    txDoneCB_Top,
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
    printf("\r\nreset-rx\r\n");
    __HAL_RCC_TIM2_CLK_ENABLE();
    MX_TIM2_Init();
    HAL_TIM_Base_Start_IT(&htim2);
    Radio::Init(&rev);

    Radio::Standby();
    Radio::LoRaModemConfig(BW_KHZ, SPREADING_FACTOR, 1);
    Radio::SetChannel(CF_HZ);

               // preambleLen, fixLen, crcOn, invIQ
    Radio::LoRaPacketConfig(8, false, true, false);

    Radio::Rx(0);
    
    for (;;) {     
        Radio::service();
    }
}

