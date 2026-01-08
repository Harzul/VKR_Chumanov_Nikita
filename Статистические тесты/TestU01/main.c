#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "TestU01.h"
#include "unif01.h"
#include "bbattery.h"
#include "swrite.h"
#include <math.h>


/* ---------------- UTILITY ---------------- */
static inline uint32_t rotl32(const uint32_t x, int k) { return (x << k) | (x >> (32 - k)); }
static inline uint64_t rotl64(const uint64_t x, int k) { return (x << k) | (x >> (64 - k)); }

static uint64_t splitmix64_state;
static uint64_t splitmix64_next(void) {
    uint64_t z = (splitmix64_state += 0x9E3779B97F4A7C15ULL);
    z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
    z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
    return z ^ (z >> 31);
}

/* ---------------- GENERATOR INTERFACE ---------------- */
typedef struct {
    const char* name;
    int state_words;
    int bits;           /* 32 или 64 */
    void (*init)(uint64_t seed);
    uint32_t (*next32)();
    uint64_t (*next64)();
} rng_impl_t;

static rng_impl_t* g_rng = NULL;

/* wrapper для TestU01 */
double get_double_from_32bit() {
    uint32_t x = g_rng->next32();
    return x / 4294967296.0;
}

double get_double_from_64bit() {
    uint64_t x = g_rng->next64();
    return x / 18446744073709551616.0;
}


/* ---------------- XOROSHIRO64 FAMILY (32-bit state, 32-bit output) ---------------- */

/* xoroshiro64* (32-bit output) */
static uint32_t s64star_32[2];
static void init_xoroshiro64star(uint64_t seed) {
    splitmix64_state = seed;
    uint64_t sm = splitmix64_next();
    s64star_32[0] = (uint32_t)(sm & 0xFFFFFFFFu);
    s64star_32[1] = (uint32_t)(sm >> 32);
}

static uint32_t next_xoroshiro64star() {
    const uint32_t s0 = s64star_32[0];
    uint32_t s1 = s64star_32[1];
    const uint32_t result = s0 * 0x9E3779BBu;
    
    s1 ^= s0;
    s64star_32[0] = rotl32(s0, 26) ^ s1 ^ (s1 << 9);
    s64star_32[1] = rotl32(s1, 13);
    
    return result;
}

/* xoroshiro64** (32-bit output) */
static uint32_t s64starstar_32[2];
static void init_xoroshiro64starstar(uint64_t seed) {
    splitmix64_state = seed;
    uint64_t sm = splitmix64_next();
    s64starstar_32[0] = (uint32_t)(sm & 0xFFFFFFFFu);
    s64starstar_32[1] = (uint32_t)(sm >> 32);
}

static uint32_t next_xoroshiro64starstar() {
    const uint32_t s0 = s64starstar_32[0];
    uint32_t s1 = s64starstar_32[1];
    const uint32_t result = rotl32(s0 * 0x9E3779BB, 5) * 5;

    s1 ^= s0;
    s64starstar_32[0] = rotl32(s0, 26) ^ s1 ^ (s1 << 9);
    s64starstar_32[1] = rotl32(s1, 13);
    
    return result;
}

/* ---------------- XOSHIRO128 FAMILY (128-bit state, 32-bit output) ---------------- */

/* xoshiro128+ */
static uint32_t s128plus[4];
static void init_xoshiro128plus(uint64_t seed) {
    splitmix64_state = seed;
    for (int i = 0; i < 4; i++) {
        s128plus[i] = (uint32_t)(splitmix64_next() & 0xFFFFFFFFu);
    }
}

static uint32_t next_xoshiro128plus() {
    const uint32_t result = s128plus[0] + s128plus[3];
    
    const uint32_t t = s128plus[1] << 9;

    s128plus[2] ^= s128plus[0];
    s128plus[3] ^= s128plus[1];
    s128plus[1] ^= s128plus[2];
    s128plus[0] ^= s128plus[3];

    s128plus[2] ^= t;

    s128plus[3] = rotl32(s128plus[3], 11);
    
    return result;
}

/* xoshiro128++ */
static uint32_t s128plusplus[4];
static void init_xoshiro128plusplus(uint64_t seed) {
    splitmix64_state = seed;
    for (int i = 0; i < 4; i++) {
        s128plusplus[i] = (uint32_t)(splitmix64_next() & 0xFFFFFFFFu);
    }
}

static uint32_t next_xoshiro128plusplus() {
    const uint32_t result = rotl32(s128plusplus[0] + s128plusplus[3], 7) + s128plusplus[0];
    
    const uint32_t t = s128plusplus[1] << 9;

    s128plusplus[2] ^= s128plusplus[0];
    s128plusplus[3] ^= s128plusplus[1];
    s128plusplus[1] ^= s128plusplus[2];
    s128plusplus[0] ^= s128plusplus[3];

    s128plusplus[2] ^= t;

    s128plusplus[3] = rotl32(s128plusplus[3], 11);
    
    return result;
}

/* xoshiro128** */
static uint32_t s128starstar[4];
static void init_xoshiro128starstar(uint64_t seed) {
    splitmix64_state = seed;
    for (int i = 0; i < 4; i++) {
        s128starstar[i] = (uint32_t)(splitmix64_next() & 0xFFFFFFFFu);
    }
}

static uint32_t next_xoshiro128starstar() {
    const uint32_t result = rotl32(s128starstar[1] * 5u, 7) * 9u;
    
    const uint32_t t = s128starstar[1] << 9;

    s128starstar[2] ^= s128starstar[0];
    s128starstar[3] ^= s128starstar[1];
    s128starstar[1] ^= s128starstar[2];
    s128starstar[0] ^= s128starstar[3];

    s128starstar[2] ^= t;

    s128starstar[3] = rotl32(s128starstar[3], 11);
    
    return result;
}

/* ---------------- XOROSHIRO128 FAMILY (128-bit state, 64-bit output) ---------------- */

/* xoroshiro128+ */
static uint64_t s128plus_64[2];
static void init_xoroshiro128plus(uint64_t seed) {
    splitmix64_state = seed;
    s128plus_64[0] = splitmix64_next();
    s128plus_64[1] = splitmix64_next();
}

static uint64_t next_xoroshiro128plus() {
    const uint64_t s0 = s128plus_64[0];
    uint64_t s1 = s128plus_64[1];

    const uint64_t result = s0 + s1;
    
    s1 ^= s0;
    s128plus_64[0] = rotl64(s0, 24) ^ s1 ^ (s1 << 16);
    s128plus_64[1] = rotl64(s1, 37);
    
    return result;
}

/* xoroshiro128++ */
static uint64_t s128plusplus_64[2];
static void init_xoroshiro128plusplus(uint64_t seed) {
    splitmix64_state = seed;
    s128plusplus_64[0] = splitmix64_next();
    s128plusplus_64[1] = splitmix64_next();
}

static uint64_t next_xoroshiro128plusplus() {
    const uint64_t s0 = s128plusplus_64[0];
    uint64_t s1 = s128plusplus_64[1];

    const uint64_t result = rotl64(s0 + s1, 17) + s0;
    
    s1 ^= s0;
    s128plusplus_64[0] = rotl64(s0, 49) ^ s1 ^ (s1 << 21);
    s128plusplus_64[1] = rotl64(s1, 28);

    return result;
}


/* xoroshiro128** */
static uint64_t s128starstar_64[2];
static void init_xoroshiro128starstar(uint64_t seed) {
    splitmix64_state = seed;
    s128starstar_64[0] = splitmix64_next();
    s128starstar_64[1] = splitmix64_next();
}

static uint64_t next_xoroshiro128starstar() {
    const uint64_t s0 = s128starstar_64[0];
    uint64_t s1 = s128starstar_64[1];
    const uint64_t result = rotl64(s0 * 5, 7) * 9;

    s1 ^= s0;
    s128starstar_64[0] = rotl64(s0, 24) ^ s1 ^ (s1 << 16);
    s128starstar_64[1] = rotl64(s1, 37);

    return result;
}

/* ---------------- XOSHIRO256 FAMILY (256-bit state, 64-bit output) ---------------- */

/* xoshiro256+ */
static uint64_t s256plus[4];
static void init_xoshiro256plus(uint64_t seed) {
    splitmix64_state = seed;
    for (int i = 0; i < 4; i++) {
        s256plus[i] = splitmix64_next();
    }
}

static uint64_t next_xoshiro256plus() {
    const uint64_t result = s256plus[0] + s256plus[3];

    const uint64_t t = s256plus[1] << 17;

    s256plus[2] ^= s256plus[0];
    s256plus[3] ^= s256plus[1];
    s256plus[1] ^= s256plus[2];
    s256plus[0] ^= s256plus[3];

    s256plus[2] ^= t;

    s256plus[3] = rotl64(s256plus[3], 45);

    return result;
}

/* xoshiro256++ */
static uint64_t s256plusplus[4];
static void init_xoshiro256plusplus(uint64_t seed) {
    splitmix64_state = seed;
    for (int i = 0; i < 4; i++) {
        s256plusplus[i] = splitmix64_next();
    }
}

static uint64_t next_xoshiro256plusplus() {
    const uint64_t result = rotl64(s256plusplus[0] + s256plusplus[3], 23) + s256plusplus[0];

    const uint64_t t = s256plusplus[1] << 17;

    s256plusplus[2] ^= s256plusplus[0];
    s256plusplus[3] ^= s256plusplus[1];
    s256plusplus[1] ^= s256plusplus[2];
    s256plusplus[0] ^= s256plusplus[3];

    s256plusplus[2] ^= t;

    s256plusplus[3] = rotl64(s256plusplus[3], 45);

    return result;
}

/* xoshiro256** */
static uint64_t s256starstar[4];
static void init_xoshiro256starstar(uint64_t seed) {
    splitmix64_state = seed;
    for (int i = 0; i < 4; i++) {
        s256starstar[i] = splitmix64_next();
    }
}

static uint64_t next_xoshiro256starstar() {
    const uint64_t result = rotl64(s256starstar[1] * 5, 7) * 9;

    const uint64_t t = s256starstar[1] << 17;

    s256starstar[2] ^= s256starstar[0];
    s256starstar[3] ^= s256starstar[1];
    s256starstar[1] ^= s256starstar[2];
    s256starstar[0] ^= s256starstar[3];

    s256starstar[2] ^= t;

    s256starstar[3] = rotl64(s256starstar[3], 45);

    return result;
}

/* ---------------- XOSHIRO512 FAMILY (512-bit state, 64-bit output) ---------------- */

/* xoshiro512+ */
static uint64_t s512plus[8];
static void init_xoshiro512plus(uint64_t seed) {
    splitmix64_state = seed;
    for (int i = 0; i < 8; i++) {
        s512plus[i] = splitmix64_next();
    }
}

static uint64_t next_xoshiro512plus() {
    const uint64_t result = s512plus[0] + s512plus[2];

    const uint64_t t = s512plus[1] << 11;

    s512plus[2] ^= s512plus[0];
    s512plus[5] ^= s512plus[1];
    s512plus[1] ^= s512plus[2];
    s512plus[7] ^= s512plus[3];
    s512plus[3] ^= s512plus[4];
    s512plus[4] ^= s512plus[5];
    s512plus[0] ^= s512plus[6];
    s512plus[6] ^= s512plus[7];

    s512plus[6] ^= t;

    s512plus[7] = rotl64(s512plus[7], 21);

    return result;
}

/* xoshiro512++ */
static uint64_t s512plusplus[8];
static void init_xoshiro512plusplus(uint64_t seed) {
    splitmix64_state = seed;
    for (int i = 0; i < 8; i++) {
        s512plusplus[i] = splitmix64_next();
    }
}

static uint64_t next_xoshiro512plusplus() {
    const uint64_t result = rotl64(s512plusplus[0] + s512plusplus[2], 17) + s512plusplus[2];

    const uint64_t t = s512plusplus[1] << 11;

    s512plusplus[2] ^= s512plusplus[0];
    s512plusplus[5] ^= s512plusplus[1];
    s512plusplus[1] ^= s512plusplus[2];
    s512plusplus[7] ^= s512plusplus[3];
    s512plusplus[3] ^= s512plusplus[4];
    s512plusplus[4] ^= s512plusplus[5];
    s512plusplus[0] ^= s512plusplus[6];
    s512plusplus[6] ^= s512plusplus[7];

    s512plusplus[6] ^= t;

    s512plusplus[7] = rotl64(s512plusplus[7], 21);

    return result;
}

/* xoshiro512** */
static uint64_t s512starstar[8];
static void init_xoshiro512starstar(uint64_t seed) {
    splitmix64_state = seed;
    for (int i = 0; i < 8; i++) {
        s512starstar[i] = splitmix64_next();
    }
}

static uint64_t next_xoshiro512starstar() {
    const uint64_t result = rotl64(s512starstar[1] * 5, 7) * 9;

    const uint64_t t = s512starstar[1] << 11;

    s512starstar[2] ^= s512starstar[0];
    s512starstar[5] ^= s512starstar[1];
    s512starstar[1] ^= s512starstar[2];
    s512starstar[7] ^= s512starstar[3];
    s512starstar[3] ^= s512starstar[4];
    s512starstar[4] ^= s512starstar[5];
    s512starstar[0] ^= s512starstar[6];
    s512starstar[6] ^= s512starstar[7];

    s512starstar[6] ^= t;

    s512starstar[7] = rotl64(s512starstar[7], 21);

    return result;
}

/* ---------------- XOROSHIRO1024 FAMILY (1024-bit state, 64-bit output) ---------------- */
static int p;

/* xoroshiro1024* */
static uint64_t s1024star[16];
static void init_xoroshiro1024star(uint64_t seed) {
    splitmix64_state = seed;
    for (int i = 0; i < 16; i++) {
        s1024star[i] = splitmix64_next();
    }
}

static uint64_t next_xoroshiro1024star() {
    const int q = p;
    const uint64_t s0 = s1024star[p = (p + 1) & 15];
    uint64_t s15 = s1024star[q];
    const uint64_t result = s0 * 0x9e3779b97f4a7c13;

    s15 ^= s0;
    s1024star[q] = rotl64(s0, 25) ^ s15 ^ (s15 << 27);
    s1024star[p] = rotl64(s15, 36);

    return result;
}

/* xoroshiro1024++ */
static uint64_t s1024plusplus[16];

static void init_xoroshiro1024plusplus(uint64_t seed) {
    splitmix64_state = seed;
    for (int i = 0; i < 16; i++) {
        s1024plusplus[i] = splitmix64_next();
    }
}

static uint64_t next_xoroshiro1024plusplus() {
    const int q = p;
    const uint64_t s0 = s1024plusplus[p = (p + 1) & 15];
    uint64_t s15 = s1024plusplus[q];
    const uint64_t result = rotl64(s0 + s15, 23) + s15;

    s15 ^= s0;
    s1024plusplus[q] = rotl64(s0, 25) ^ s15 ^ (s15 << 27);
    s1024plusplus[p] = rotl64(s15, 36);

    return result;
}

/* xoroshiro1024** */
static uint64_t s1024starstar[16];
static void init_xoroshiro1024starstar(uint64_t seed) {
    splitmix64_state = seed;
    for (int i = 0; i < 16; i++) {
        s1024starstar[i] = splitmix64_next();
    }
}

static uint64_t next_xoroshiro1024starstar() {
    const int q = p;
    const uint64_t s0 = s1024starstar[p = (p + 1) & 15];
    uint64_t s15 = s1024starstar[q];
    const uint64_t result = rotl64(s0 * 5, 7) * 9;

    s15 ^= s0;
    s1024starstar[q] = rotl64(s0, 25) ^ s15 ^ (s15 << 27);
    s1024starstar[p] = rotl64(s15, 36);

    return result;
}

/* ---------------- MAIN ---------------- */
int main(int argc, char **argv) {
    uint64_t seed = (uint64_t)time(NULL) ^ 0xDEADBEEFDEADBEEFULL;
    int runSmall = 0, runCrush = 1, runBig = 0, verbose = 0;
    const char* gen_name = "xoshiro256**";

    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-s") && i + 1 < argc) {
            char *endp = NULL;
            if (strlen(argv[i + 1]) > 2 && argv[i + 1][0] == '0' && 
                (argv[i + 1][1] == 'x' || argv[i + 1][1] == 'X'))
                seed = strtoull(argv[i + 1], &endp, 16);
            else
                seed = strtoull(argv[i + 1], &endp, 10);
            i++;
        }
        else if (!strcmp(argv[i], "-S")) { runSmall = 1; runCrush = 0; runBig = 0; }
        else if (!strcmp(argv[i], "-C")) { runSmall = 0; runCrush = 1; runBig = 0; }
        else if (!strcmp(argv[i], "-B")) { runSmall = 0; runCrush = 0; runBig = 1; }
        else if (!strcmp(argv[i], "-v")) verbose = 1;
        else if (!strcmp(argv[i], "-g") && i + 1 < argc) { gen_name = argv[i + 1]; i++; }
        else if (!strcmp(argv[i], "-h")) {
            printf("Usage: %s [-s seed] [-r] [-S|-C|-B] [-g generator] [-v] [-l]\n", argv[0]);
            printf("\nAvailable generators:\n");
            printf("32-bit output:\n");
            printf("  xoroshiro64*     - 64-bit state, 32-bit output\n");
            printf("  xoroshiro64**    - 64-bit state, 32-bit output\n");
            printf("  xoshiro128+        - 128-bit state, 32-bit output\n");
            printf("  xoshiro128++       - 128-bit state, 32-bit output\n");
            printf("  xoshiro128**       - 128-bit state, 32-bit output\n");
            printf("\n64-bit output:\n");
            printf("  xoroshiro64*       - 64-bit state, 64-bit output\n");
            printf("  xoroshiro64**      - 64-bit state, 64-bit output\n");
            printf("  xoroshiro128+      - 128-bit state, 64-bit output\n");
            printf("  xoroshiro128++     - 128-bit state, 64-bit output\n");
            printf("  xoroshiro128**     - 128-bit state, 64-bit output\n");
            printf("  xoshiro256+        - 256-bit state, 64-bit output\n");
            printf("  xoshiro256++       - 256-bit state, 64-bit output\n");
            printf("  xoshiro256**       - 256-bit state, 64-bit output\n");
            printf("  xoshiro512+        - 512-bit state, 64-bit output\n");
            printf("  xoshiro512++       - 512-bit state, 64-bit output\n");
            printf("  xoshiro512**       - 512-bit state, 64-bit output\n");
            printf("  xoroshiro1024*     - 1024-bit state, 64-bit output\n");
            printf("  xoroshiro1024++    - 1024-bit state, 64-bit output\n");
            printf("  xoroshiro1024**    - 1024-bit state, 64-bit output\n");
            return 0;
        } else if (!strcmp(argv[i], "-l")) {
            printf("Available generators (32-bit output):\n");
            printf("  xoroshiro64*, xoroshiro64**, xoshiro128+, xoshiro128++, xoshiro128**\n");
            printf("\nAvailable generators (64-bit output):\n");
            printf("  xoroshiro64*, xoroshiro64**, xoroshiro128+, xoroshiro128++, xoroshiro128**\n");
            printf("  xoshiro256+, xoshiro256++, xoshiro256**, xoshiro512+, xoshiro512++, xoshiro512**\n");
            printf("  xoroshiro1024*, xoroshiro1024++, xoroshiro1024**\n");
            return 0;
        }
    }

    swrite_Basic = verbose ? TRUE : FALSE;

    /* ---------------- SELECT GENERATOR ---------------- */
    g_rng = malloc(sizeof(rng_impl_t));
    if (!g_rng) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* 32-bit generators */
    if (!strcmp(gen_name, "xoroshiro64*")) {
        g_rng->bits = 32; g_rng->init = init_xoroshiro64star; 
        g_rng->next32 = next_xoroshiro64star; g_rng->next64 = NULL;
    } else if (!strcmp(gen_name, "xoroshiro64**")) {
        g_rng->bits = 32; g_rng->init = init_xoroshiro64starstar; 
        g_rng->next32 = next_xoroshiro64starstar; g_rng->next64 = NULL;
    } else if (!strcmp(gen_name, "xoshiro128+")) {
        g_rng->bits = 32; g_rng->init = init_xoshiro128plus; 
        g_rng->next32 = next_xoshiro128plus; g_rng->next64 = NULL;
    } else if (!strcmp(gen_name, "xoshiro128++")) {
        g_rng->bits = 32; g_rng->init = init_xoshiro128plusplus; 
        g_rng->next32 = next_xoshiro128plusplus; g_rng->next64 = NULL;
    } else if (!strcmp(gen_name, "xoshiro128**")) {
        g_rng->bits = 32; g_rng->init = init_xoshiro128starstar; 
        g_rng->next32 = next_xoshiro128starstar; g_rng->next64 = NULL;
    /* 64-bit generators */
    } else if (!strcmp(gen_name, "xoroshiro128+")) {
        g_rng->bits = 64; g_rng->init = init_xoroshiro128plus; 
        g_rng->next32 = NULL; g_rng->next64 = next_xoroshiro128plus;
    } else if (!strcmp(gen_name, "xoroshiro128++")) {
        g_rng->bits = 64; g_rng->init = init_xoroshiro128plusplus; 
        g_rng->next32 = NULL; g_rng->next64 = next_xoroshiro128plusplus;
    } else if (!strcmp(gen_name, "xoroshiro128**")) {
        g_rng->bits = 64; g_rng->init = init_xoroshiro128starstar; 
        g_rng->next32 = NULL; g_rng->next64 = next_xoroshiro128starstar;
    } else if (!strcmp(gen_name, "xoshiro256+")) {
        g_rng->bits = 64; g_rng->init = init_xoshiro256plus; 
        g_rng->next32 = NULL; g_rng->next64 = next_xoshiro256plus;
    } else if (!strcmp(gen_name, "xoshiro256++")) {
        g_rng->bits = 64; g_rng->init = init_xoshiro256plusplus; 
        g_rng->next32 = NULL; g_rng->next64 = next_xoshiro256plusplus;
    } else if (!strcmp(gen_name, "xoshiro256**")) {
        g_rng->bits = 64; g_rng->init = init_xoshiro256starstar; 
        g_rng->next32 = NULL; g_rng->next64 = next_xoshiro256starstar;
    } else if (!strcmp(gen_name, "xoshiro512+")) {
        g_rng->bits = 64; g_rng->init = init_xoshiro512plus; 
        g_rng->next32 = NULL; g_rng->next64 = next_xoshiro512plus;
    } else if (!strcmp(gen_name, "xoshiro512++")) {
        g_rng->bits = 64; g_rng->init = init_xoshiro512plusplus; 
        g_rng->next32 = NULL; g_rng->next64 = next_xoshiro512plusplus;
    } else if (!strcmp(gen_name, "xoshiro512**")) {
        g_rng->bits = 64; g_rng->init = init_xoshiro512starstar; 
        g_rng->next32 = NULL; g_rng->next64 = next_xoshiro512starstar;
    } else if (!strcmp(gen_name, "xoroshiro1024*")) {
        g_rng->bits = 64; g_rng->init = init_xoroshiro1024star; 
        g_rng->next32 = NULL; g_rng->next64 = next_xoroshiro1024star;
    } else if (!strcmp(gen_name, "xoroshiro1024++")) {
        g_rng->bits = 64; g_rng->init = init_xoroshiro1024plusplus; 
        g_rng->next32 = NULL; g_rng->next64 = next_xoroshiro1024plusplus;
    } else if (!strcmp(gen_name, "xoroshiro1024**")) {
        g_rng->bits = 64; g_rng->init = init_xoroshiro1024starstar; 
        g_rng->next32 = NULL; g_rng->next64 = next_xoroshiro1024starstar;
    } else {
        fprintf(stderr, "Unknown generator: %s\n", gen_name);
        fprintf(stderr, "Use -h for help or -l to list available generators\n");
        free(g_rng);
        return 1;
    }
    
    g_rng->name = gen_name;
    g_rng->init(seed);
    
    if (verbose) {
        printf("Testing generator: %s\n", g_rng->name);
        printf("Seed: 0x%016llX\n", (unsigned long long)seed);
        printf("Bits: %d\n", g_rng->bits);
    }

    /* ---------------- CREATE TestU01 GENERATOR ---------------- */
    unif01_Gen *gen = NULL;
    if (g_rng->bits == 32) {
        for (int i=0; i < pow(2,16); i++)
            g_rng->next32();
        gen = unif01_CreateExternGen01((char *)g_rng->name, get_double_from_32bit);
    }
    else {
        for (int i=0; i < pow(2,16); i++)
            g_rng->next64();
        gen = unif01_CreateExternGen01((char *)g_rng->name, get_double_from_64bit);
    }
    if (!gen) {
        fprintf(stderr, "Failed to create TestU01 generator.\n");
        free(g_rng);
        return 1;
    }

    /* ---------------- RUN TEST BATTERIES ---------------- */
    if (runSmall) {
        printf("Running SmallCrush...\n");
        fflush(stdout);
        bbattery_SmallCrush(gen);
    }
    if (runCrush) {
        printf("Running Crush...\n");
        fflush(stdout);
        bbattery_Crush(gen);
    }
    if (runBig) {
        printf("Running BigCrush...\n");
        fflush(stdout);
        bbattery_BigCrush(gen);
    }

    /* CLEANUP */
    unif01_DeleteExternGenBits(gen);
    free(g_rng);
    return 0;
}