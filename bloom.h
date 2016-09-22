/**
   @file    bloom.h
   @brief   Bloom filter
*/
#ifndef _BLOOM_H_
#define _BLOOM_H_

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef struct _bloom_t_ {
    unsigned char * bits ;
    int             sz ; /* Size of bit array in bytes */
    int             nh ; /* Number of hashes */
} bloom_t ;

/** Instantiate new bloom filter meant to store n items with probability p
    of false positive */
bloom_t * bloom_new(int n, double p);

/** Free bloom filter */
void bloom_del(bloom_t * bf);

/** Add data to a bloom filter */
void bloom_add(bloom_t * bf, unsigned char * data, int len);

/** Check if provided data has already been seen by filter */    
int bloom_check(bloom_t * bf, unsigned char * data, int len);

#endif
