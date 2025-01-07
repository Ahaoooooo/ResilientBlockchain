#ifndef MINER_H
#define MINER_H


#include "../../data/Blockchain.hpp"

uint32_t mine_new_block( Blockchain *bc);

uint32_t mine_weak_block( Blockchain *bc);

void miner( Blockchain *bc );


#endif
