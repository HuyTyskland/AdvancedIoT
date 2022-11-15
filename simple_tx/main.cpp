#include "radio.h"
#include <cstdint>
#include "defs.h"
#include "sync.h"

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

uint8_t syncStatus[NUMANCHOR] = {0,0,0,0};

void tx_test()
{
    // static uint8_t seq = 0;

    // Radio::radio.tx_buf[0] = seq++;  /* set payload */
    // Radio::Send(1, 0, 0, 0);   /* begin transmission */
    // printf("sent\r\n");
    if (areSync(syncStatus))
    {
        syncNode(syncStatus);
    }

    // check if the condition is met => change to receive mode
}

void txDoneCB()
{
    if (areSync(syncStatus))
    {
        syncNode(syncStatus);
    } else {
        
    }
    //printf("got-tx-done\r\n");
    //queue.call_in(500, tx_test);
}

void rxDoneCB(uint8_t size, float rssi, float snr)
{
    // do something with received data
    if (Radio::radio.rx_buf[0] == READY)
    {
        
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
