#include "radio.h"
#include <cstdint>

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
    #define SPREADING_FACTOR    7
    #define CF_HZ               868300000
#endif

/**********************************************************************/
EventQueue queue(4 * EVENTS_EVENT_SIZE);
uint8_t countSend = 0;
uint8_t countReceive = 0;

void tx_test()
{
    static uint8_t seq = 0;

    Radio::radio.tx_buf[0] = seq++;  /* set payload */
    Radio::Send(1, 0, 0, 0);   /* begin transmission */
    printf("sent\r\n");

/*    {
        mbed_stats_cpu_t stats;
        mbed_stats_cpu_get(&stats);
        printf("canDeep:%u ", sleep_manager_can_deep_sleep());
        printf("Uptime: %llu ", stats.uptime / 1000);
        printf("Sleep time: %llu ", stats.sleep_time / 1000);
        printf("Deep Sleep: %llu\r\n", stats.deep_sleep_time / 1000);
    }*/

    // check if the condition is met => change to receive mode
    countSend++;
    if (countSend == 10)
    {
        countSend = 0;
        Radio::Rx(0);
    }
}

void txDoneCB()
{
    printf("got-tx-done\r\n");
    queue.call_in(500, tx_test);
}

void rxDoneCB(uint8_t size, float rssi, float snr)
{
    // do something with received data
    printf("Receive something \r\n");
    countReceive++;
    printf("countReceive: %d\r\n", countReceive);
    // check if the condition is met => change to transmit mode
    if (countReceive == 10)
    {
        countReceive = 0;
        //Send stuff to change to sending mode
        Radio::radio.tx_buf[0] = 99;
        Radio::Send(1, 0, 0, 0);
        printf("sent\r\n");
    }
}


void radio_irq_callback()
{
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

    Radio::Init(&rev);

    Radio::Standby();
    Radio::LoRaModemConfig(BW_KHZ, SPREADING_FACTOR, 1);
    Radio::SetChannel(CF_HZ);

    Radio::set_tx_dbm(TX_DBM);

               // preambleLen, fixLen, crcOn, invIQ
    Radio::LoRaPacketConfig(8, false, true, false);

    queue.call_in(500, tx_test);

    queue.dispatch();
}
