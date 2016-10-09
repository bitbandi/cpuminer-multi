#include "cpuminer-config.h"
#include "miner.h"

#include <string.h>
#include <stdint.h>

#include "sha3/sph_luffa.h"
#include "sha3/sph_cubehash.h"
#include "sha3/sph_shavite.h"
#include "sha3/sph_simd.h"
#include "sha3/sph_echo.h"


/* Move init out of loop, so init once externally, and then use one single memcpy with that bigger memory block */
typedef struct {
	sph_luffa512_context	luffa;
	sph_cubehash512_context	cubehash;
	sph_shavite512_context	shavite;
	sph_simd512_context	simd;
	sph_echo512_context	echo;
} qubithash_context_holder;

/* no need to copy, because close reinit the context */
static THREADLOCAL qubithash_context_holder ctx;

void init_qubit_contexts()
{
	sph_luffa512_init(&ctx.luffa);
	sph_cubehash512_init(&ctx.cubehash);
	sph_shavite512_init(&ctx.shavite);
	sph_simd512_init (&ctx.simd);
	sph_echo512_init (&ctx.echo);
}

void qubithash(void *state, const void *input)
{
	uint32_t hash[16];
	uint32_t mask = 8;
	uint32_t zero = 0;

	memset(hash, 0, 16 * sizeof(uint32_t));

	sph_luffa512(&ctx.luffa, input, 80);
	sph_luffa512_close (&ctx.luffa, hash);


	sph_cubehash512(&ctx.cubehash, hash, 64);
	sph_cubehash512_close(&ctx.cubehash, hash);

	sph_shavite512(&ctx.shavite, hash, 64);
	sph_shavite512_close(&ctx.shavite, hash);

	sph_simd512(&ctx.simd, hash, 64);
	sph_simd512_close(&ctx.simd, hash);

	sph_echo512(&ctx.echo, hash, 64);
	sph_echo512_close(&ctx.echo, hash);

	memcpy(state, hash, 32);
}

int scanhash_qubit(int thr_id, uint32_t *pdata, const uint32_t *ptarget,
	uint32_t max_nonce, uint64_t *hashes_done)
{
	uint32_t n = pdata[19] - 1;
	const uint32_t first_nonce = pdata[19];
	const uint32_t Htarg = ptarget[7];

	uint32_t hash64[8] __attribute__((aligned(32)));
	uint32_t endiandata[32];

	uint64_t htmax[] = {
		0,
		0xF,
		0xFF,
		0xFFF,
		0xFFFF,
		0x10000000
	};
	uint32_t masks[] = {
		0xFFFFFFFF,
		0xFFFFFFF0,
		0xFFFFFF00,
		0xFFFFF000,
		0xFFFF0000,
		0
	};

	// we need bigendian data...
	for (int kk=0; kk < 32; kk++) {
		be32enc(&endiandata[kk], ((uint32_t*)pdata)[kk]);
	};
#ifdef DEBUG_ALGO
	printf("[%d] Htarg=%X\n", thr_id, Htarg);
#endif
	for (int m=0; m < sizeof(masks); m++) {
		if (Htarg <= htmax[m]) {
			uint32_t mask = masks[m];
			do {
				pdata[19] = ++n;
				be32enc(&endiandata[19], n);
				qubithash(hash64, &endiandata);
#ifndef DEBUG_ALGO
				if ((!(hash64[7] & mask)) && fulltest(hash64, ptarget)) {
					*hashes_done = n - first_nonce + 1;
					return true;
				}
#else
				if (!(n % 0x1000) && !thr_id) printf(".");
				if (!(hash64[7] & mask)) {
					printf("[%d]",thr_id);
					if (fulltest(hash64, ptarget)) {
						*hashes_done = n - first_nonce + 1;
						return true;
					}
				}
#endif
			} while (n < max_nonce && !work_restart[thr_id].restart);
			// see blake.c if else to understand the loop on htmax => mask
			break;
		}
	}

	*hashes_done = n - first_nonce + 1;
	pdata[19] = n;
	return 0;
}
