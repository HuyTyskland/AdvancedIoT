#include "main_controller.h"
#include "time_info.h"
#include "normal_mode.h"
#include "call_to_sync_mode.h"
#include "sync_mode.h"
#include <cstdint>
#include <iterator>

states current_state;
states next_state;

// Function pointer
void (*state_listen)(void) = NULL;
void (*state_send)(uint8_t*) = NULL;
void (*state_CBListen)(uint8_t , float , float ) = NULL;
void (*state_CBSend)(void) = NULL;


// Set the next state of the tracking node
void fsm_set_state(states newState)
{
    next_state = newState;
}

// Update the function pointer
void fsm_update_state(void)
{
    state_listen = &call_to_sync_listen;
    state_send = &call_to_sync_send;
    state_CBListen = &call_to_sync_CBListen;
    state_CBSend = &call_to_sync_CBSend;

    current_state = next_state;

    switch (current_state) 
    {
        case call_to_sync_state:
        {
            state_listen = &call_to_sync_listen;
            state_send = &call_to_sync_send;
            state_CBListen = &call_to_sync_CBListen;
            state_CBSend = &call_to_sync_CBSend;
            break;
        }
        case sync_state:
        {
            state_listen = &sync_listen;
            state_send = &sync_send;

        }
    }
}

// Update state automatically
void fsm_step(void)
{
    if (current_state != next_state)
    {

    }
}