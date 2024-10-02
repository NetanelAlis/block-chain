#ifndef UTILS_HPP
#define UTILS_HPP

#include <iostream>
#include <zlib.h>
#include <math.h>
#include <string>
#include <list>
#include <sstream>
#include <fstream>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#define SIZE_OF_CRC_RESULT 32
#define DIFFICULTY_POS 11
#define MINER_ID_POS 19
#define CONF_FILE_PATH "mnt/mta/mtacoin.conf"
#define SERVER_PIPE_PATH "mnt/mta/server_pipe"
#define LOGS_FILE_PATH "/var/log/mtacoin.log"

typedef struct {
    int         	  height;        // Incrementeal ID of the block in the chain
    int         	  timestamp;     // Time of the mine in seconds since epoch
    unsigned int    hash;          // Current block hash value
    unsigned int    prev_hash;     // Hash value of the previous block
    int        	    difficulty;    // Amount of preceding zeros in the hash
    int         	  nonce;         // Incremental integer to change the hash value
    int         	  relayed_by;    // Miner ID
} BLOCK_T;

const typedef enum {
  SUBSCRIBE_OP_CODE = 1,
  BLOCK_OP_CODE = 2
} TLV_TYPE;

struct VALUE
{
    char* pipeName;
    BLOCK_T* block;
}; 

struct TLV
{
    int type;
    int length;
    VALUE value;
};

#endif // UTILS_HPP
