#include "mbed.h"
#include "defs.h"
#include <cstdint>

// Check if all nodes are synchronized in time.
//  * input: the synchronizing status of nodes
bool areSync(uint8_t*);

// Synchronize a node.
//  * input: the index of a node in the synchronizing status array
void syncNode(uint8_t*);

// Check which node is the first node that is not in sync in the synchronizing status array
//  * input: the synchronizing status of node
uint8_t nodeNotSync(uint8_t*);