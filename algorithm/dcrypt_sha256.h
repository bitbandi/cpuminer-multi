// Copyright (c) 2013-2014 The Slimcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef SHA256_H
#define SHA256_H

#include <openssl/sha.h>
#include "miner.h"
 
#define SHA256_LEN           64 //64 bytes the for the hash itself

//I need these
typedef unsigned long long  uint64;
typedef          long long  int64;
typedef unsigned int        u32int;
typedef          int        s32int;
typedef unsigned short      u16int;
typedef          short      s16int;
typedef unsigned char       u8int;
typedef          char       s8int;

//the intermediate sha256 hashing algoritm
void sha256_to_str(const u8int *data, size_t data_sz, u8int *outputBuffer, u8int *hash_digest);
u32int *sha256_(const u8int *data, size_t data_sz, u32int *hash_digest);

void sha256_salt_to_str(const u8int *data, size_t data_sz, u8int *salt, size_t salt_sz, 
                        u8int *outputBuffer, u8int *hash_digest);

void digest_to_string(u8int *hash_digest, u8int *string);

#endif
