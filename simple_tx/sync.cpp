#include "sync.h"
#include "radio.h"
#include <cstdint>

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

void syncNode(uint8_t *ptrSyncNode)
{
    // Send signal to ask if the node is ready for synchronization process
    uint8_t whichNode = nodeNotSync(ptrSyncNode);
    if (whichNode < NUMANCHOR)
    {
        Radio::radio.tx_buf[0] = whichNode + 1;  /* set payload */
        Radio::Send(1, 0, 0, 0);   /* begin transmission */
        printf("Sending synchronization request to node %d\r\n", whichNode);
    }

    // After sending synchronization request, listen
    Radio::Rx(10000);
}