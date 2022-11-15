#include <cstdint>
#include <stdint.h>
#include "defs.h"

void call_to_sync_listen(void);
void call_to_sync_send(uint8_t*);
void call_to_sync_CBListen(uint8_t , float , float );
void call_to_sync_CBSend(void);
uint16_t calculate_distance();