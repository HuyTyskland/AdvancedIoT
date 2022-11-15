#include "call_to_sync_mode.h"
#include <cstdint>
#include "time_info.h"


uint32_t transmit_time = 0;
uint32_t receive_time = 0;

bool areSync(uint8_t *ptrSyncNode)
{
    int *end = ((int*)ptrSyncNode + NUMANCHOR);
    for (int* i = (int*)ptrSyncNode; i != end; ++i)
    {
        if (*i == 0)
        {
            return ALLNOTSYNC;
        }
    }
    return ALLSYNC;
}

uint8_t nodeNotSync(uint8_t *ptrSyncNode)
{
    int *end = ((int*)ptrSyncNode + NUMANCHOR);
    uint8_t index = 0;
    for (int* i = (int*)ptrSyncNode; i != end; ++i)
    {
        if (*i == 0)
        {
            return index;
        }
        index++;
    }
    return NUMANCHOR; // check condition before using this function
}

void call_to_sync_listen(void)
{
    Radio::Rx(10000);
}

void call_to_sync_send(uint8_t *ptrSyncNode)
{
    uint8_t whichNode = nodeNotSync(ptrSyncNode);
    if (whichNode < NUMANCHOR)
    {
        Radio::radio.tx_buf[0] = whichNode + 1;  /* set payload */
        Radio::Send(1, 0, 0, 0);   /* begin transmission */
        transmit_time = return_counter();
        printf("Sending synchronization request to node %d\r\n", whichNode);
    }
}

void call_to_sync_CBListen(uint8_t size, float rssi, float snr)
{
    if (Radio::radio.rx_buf[0] == READY)
    {
        // trigger flag to change to Sync state
    }
}


void call_to_sync_CBSend(void)
{
    receive_time = return_counter();
    uint16_t time_difference = (receive_time - transmit_time)/2;
}