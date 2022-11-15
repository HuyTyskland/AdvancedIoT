#include <stdint.h>
#define NUMANCHOR 4
#define SYNCMODE 11111
#define RECEIVEMODE 22222
#define TRANSMITMODE 33333
#define ALLSYNC 1
#define ALLNOTSYNC 0
#define READY 2
#define SYNCHRONIZED 3

typedef enum {
  call_to_sync_state,
  sync_state,
  normal_state
} states;