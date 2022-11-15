#include "sync_mode.h"
#include "defs.h"
#include "time_info.h"

uint8_t transmitted_time[NUMANCHOR+1];

void cut_time(uint32_t time)
{
    transmitted_time[0] = 1;
    transmitted_time[1] = (time & 0xff000000UL) >> 24;
    transmitted_time[2] = (time & 0x00ff0000UL) >> 16;
    transmitted_time[3] = (time & 0x0000ff00UL) >>  8;
    transmitted_time[4] = (time & 0x000000ffUL)      ;
}

void sync_send(uint8_t whichNode)
{
    uint32_t sent_time = return_counter();
    //send_time = send_time + time_distance[whichNode]; // remember to measure this one
    cut_time(sent_time); // remember to measure this one

    // put data into buffer
    Radio::radio.tx_buf[0] = transmitted_time[0];  /* set payload */
    Radio::radio.tx_buf[1] = transmitted_time[1];
    Radio::radio.tx_buf[2] = transmitted_time[2];
    Radio::radio.tx_buf[3] = transmitted_time[3];
    Radio::radio.tx_buf[4] = transmitted_time[4];
    Radio::Send(5, 0, 0, 0);   /* begin transmission */
}

void sync_listen(void)
{
    Radio::Rx(10000);
}

void sync_CBSend(void)
{

}

void sync_CBSend(uint8_t size, float rssi, float snr)
{
    if (Radio::radio.rx_buf[0] == SYNCHRONIZED)
    {
        // trigger flag to change to go back to Call to Sync state or if all node synchronized, change to Normal state
    }
}
