/**
   @file    bloom.c
   @brief   Bloom filter
   See: https://en.wikipedia.org/wiki/Bloom_filter
*/
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#include "bloom.h"

#define DEBUG 0

/** 1/log(2)^2 */
#define INVSQ_LOG2  (-2.0813689810056077)

/** New bloom filter meant to store n items with proba p of false positive */
bloom_t * bloom_new(int n, double p)
{
    bloom_t * bf ;
    double    dbits ;
    int       sz ;

    /* Kick out nonsense values */
    if (n<=0) return NULL ;
    if (p<=0.0 || p>=1.0) return NULL ;

    bf = malloc(sizeof(bloom_t));

    /* Number of bits in filter */
    dbits = (double)n * log(p) * INVSQ_LOG2 ;
    /* Round up to next power of 8 */
    sz = (int)dbits;
    bf->sz = (sz % 8) ? (1+sz/8) : sz/8 ;
    /* Number of hash functions */
    bf->nh = (int)(dbits * log(2.0) / (double)n) ;
#if DEBUG>0
    printf("items %d - bits %g - bytes %d - nh %d\n", n, dbits, bf->sz, bf->nh);
#endif
    /* Allocate bit array */
    bf->bits = calloc(bf->sz, 1);
    return bf ;
}

/** Free bloom filter */
void bloom_del(bloom_t * bf)
{
    if (!bf) return ;
    if (bf->bits)
        free(bf->bits);
    free(bf);
    return ;
}

/** Private: general-purpose hash function on 32 bits */
#define FNV_PRIME 0x811c9dc5
static uint32_t fnv_hash(unsigned char *str, int len)
{
   uint32_t hash =0;
   int i;

   for(i=0; i<len; str++, i++) {
      hash *= FNV_PRIME;
      hash ^= (*str);
   }
   return hash;
}

/** Add data to a bloom filter */
void bloom_add(bloom_t * bf, unsigned char * data, int len)
{
	uint32_t        hash ;
	int             i;
    int             pos, bytepos, bitpos ;
    unsigned char * p_data ;
    int             p_len ;

    /* First hash computed on real data */
    p_data = data ;
    p_len  = len ;
    for (i=0 ; i<bf->nh ; i++) {
        hash = fnv_hash(p_data, p_len) ;
        /* Light up bit at hashed position */
        pos     = (hash % (8*bf->sz));
        bytepos = pos / 8 ;
        bitpos  = pos - bytepos*8 ;
        bf->bits[bytepos] |= (1<<bitpos);
        /* Consecutive hashes are hash of previous hash */
        p_data = (unsigned char *)&hash ;
        p_len  = sizeof(uint32_t) ;
    }
    return ;
}

/** Check if provided data has already been seen by filter */    
int bloom_check(bloom_t * bf, unsigned char * data, int len)
{
	uint32_t hash ;
	int i;
    unsigned char * p_data ;
    int             p_len ;
    int             pos, bytepos, bitpos ;

    /* First hash computed on real data */
    p_data = data ;
    p_len  = len ;
    for (i=0 ; i<bf->nh ; i++) {
        hash = fnv_hash(p_data, p_len) ;
        /* Check up bit at hashed position */
        pos     = (hash % (8*bf->sz));
        bytepos = pos / 8 ;
        bitpos  = pos - bytepos*8 ;
        if (!((bf->bits[bytepos]>>bitpos)&1)) {
            return 0 ;
        }
        /* Consecutive hashes are hash of previous hash */
        p_data = (unsigned char *)&hash ;
        p_len  = sizeof(uint32_t) ;
    }
    return 1 ;
}

#ifdef MAIN

#include <time.h>
#include <sys/time.h>

#define ALIGN   "%15s: %6.4f\n"
#define NKEYS   1024*1024

#if DEBUG>2
static void bloom_dump(bloom_t * bf)
{
    int i ;
    printf("---\n");
    for (i=0 ; i<bf->sz ; i++) {
        printf("%02x", bf->bits[i]);
    }
    printf("\n");
}
#endif 

double epoch_double()
{
    struct timeval t;
    gettimeofday(&t, NULL);
    return t.tv_sec + (t.tv_usec * 1.0) / 1000000.0;
}

int main(int argc, char * argv[])
{
    bloom_t * bf ;
    double t1, t2 ;
    int    nkeys ;
    char * buffer ;
    int    check ;
    int    i ;
    int    miss ;
    double p ;

    nkeys = (argc>1) ? (int)atoi(argv[1]) : NKEYS ;
    p     = (argc>2) ? (double)atof(argv[2]) : 0.01 ;

    printf("%15s: %d\n", "values", nkeys);
    buffer = malloc(9 * nkeys);

    bf = bloom_new(nkeys, p);
#if DEBUG>2
    bloom_dump(bf);
#endif

    t1 = epoch_double();
    for (i=0; i<nkeys; i++) {
        sprintf(buffer + i * 9, "%08x", i);
    }
    t2 = epoch_double();
    printf(ALIGN, "initialization", t2 - t1);

    t1 = epoch_double();
    for (i=0; i<nkeys; i++) {
        bloom_add(bf, (unsigned char *)buffer + i*9, 9);
    }
    t2 = epoch_double();
    printf(ALIGN, "adding", t2 - t1);
#if DEBUG>2
    bloom_dump(bf);
#endif

    t1 = epoch_double();
    for (i=0; i<nkeys; i++) {
        check = bloom_check(bf, (unsigned char *)buffer + i*9, 9);
        if (!check) {
            printf("-> WRONG [%s] not found\n", buffer+i*9);
        }
    }
    t2 = epoch_double();
    printf(ALIGN, "lookup", t2 - t1);

#if DEBUG>2
    bloom_dump(bf);
#endif
    /* Mess up keys */
    for (i=0 ; i<nkeys ; i++) {
        buffer[i*9] = 'Z';
    }

    miss=0;
    t1 = epoch_double();
    for (i=0; i<nkeys; i++) {
        check = bloom_check(bf, (unsigned char *)buffer + i*9, 9);
        if (check) {
            miss++;
#if DEBUG>2
            printf("-> false [%s]\n", buffer+i*9);
#endif
        }
    }
    t2 = epoch_double();
    printf(ALIGN, "lookup", t2 - t1);

    printf("miss %d nkeys %d rate %g\n", miss, nkeys, (double)miss/(double)nkeys);

    bloom_del(bf);
    free(buffer);
    return 0 ;
}
#endif
