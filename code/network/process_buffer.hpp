#ifndef POW_PROCESS_BUFFER_H
#define POW_PROCESS_BUFFER_H

#include <iostream>
#include <string>
#include <vector>
#include <map>


//#include "../consensus/raft/raft.hpp"
#include "../consensus/pow/pow.hpp"
#include "../data/Blockchain.hpp"
#include "./requests.hpp"
#include "../utils/crypto_stuff.h"
#include "../utils/misc.h"

using namespace std;

void process_buffer( string &m, tcp_server *ser, Blockchain *bc );



#endif