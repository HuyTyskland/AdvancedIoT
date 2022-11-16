#include "radio.h"

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
    #define SPREADING_FACTOR    7
    #define CF_HZ               868300000
#endif

DigitalOut myled(LED1);

/**********************************************************************/

void txDoneCB()
{
}

void rxDoneCB(uint8_t size, float rssi, float snr)
{
    unsigned i;
    printf("%.1fdBm  snr:%.1fdB\t", rssi, snr);

    myled.write(!myled.read()); // toggle LED

    for (i = 0; i < size; i++) {
        printf("%02x ", Radio::radio.rx_buf[i]);
    }
    printf("\r\n");
}

const RadioEvents_t rev = {
    /* Dio0_top_half */     NULL,
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
    printf("\r\nreset-rx\r\n");
    
    Radio::Init(&rev);

    Radio::Standby();
    Radio::LoRaModemConfig(BW_KHZ, SPREADING_FACTOR, 1);
    Radio::SetChannel(CF_HZ);

               // preambleLen, fixLen, crcOn, invIQ
    Radio::LoRaPacketConfig(8, false, true, false);

    
    printf("start receiving\r\n");
    Radio::Rx(10000000);
    printf("nothing to be received\r\n");
    
    for (;;) {     
        Radio::service();
    }
}

