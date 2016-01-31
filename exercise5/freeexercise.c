#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <arm_neon.h>

#define MICROS_PER_SECF 1000000.0
#define BYTES_PER_GB (1024*1024*1024LL)
#define BYTES_PER_MB (1024*1024LL)
#define SIZE (200*BYTES_PER_MB)

char array[SIZE] __attribute__((aligned (32)));

static inline double to_bw(size_t bytes, double secs) {
  double size_bytes = (double) bytes;
  double size_gb = size_bytes / ((double) BYTES_PER_GB);
  return size_gb / secs;
}

static inline double gettime(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((double) tv.tv_sec) + ((double) tv.tv_usec / (MICROS_PER_SECF));
}

void computebw (char * array) {
    int i;
    double before, after, elapsed;

    before = gettime();
    size_t* carray = (size_t*) array;
    for (i = 0; i < SIZE / sizeof(size_t); i++)
        carray[i] = 1;
    after = gettime();
    elapsed = after - before;
    printf("Bandwidth int       : %.3f GiB/s\n", to_bw(SIZE , elapsed));

    before = gettime();
    char* ccarray = array;
    for (i = 0; i < SIZE / sizeof(char); i++)
        ccarray[i] = '1';
    after = gettime();
    elapsed = after - before;
    printf("Bandwith char       : %.3f GiB/s\n", to_bw(SIZE , elapsed));

    before = gettime();
    char* cccarray = array;
    for (; *cccarray!='\0'; cccarray++)
        *cccarray = '1';
    after = gettime();
    elapsed = after - before;
    printf("Bandwidth char null : %.3f GiB/s\n", to_bw(SIZE , elapsed));

    before = gettime();
    uint8x8_t* varray = (uint8x8_t*) array;
    uint8_t lane = '1';
    for (i = 0; i < SIZE / sizeof(uint8x8_t); i++) {
        varray[i] = vdup_n_u8(lane);
    }
    after = gettime();
    elapsed = after - before;
    printf("Bandwidth neon       : %.3f GiB/s\n", to_bw(SIZE , elapsed));
}

/* Simple checkings on each char */
static void toupper_simple(char * text) {
    double before, after, elapsed;

    printf("\nBefore:  %.40s...\n",text);
    before = gettime();
    char* array = text;
    for (; *array != '\0'; array++) {
        if ((*array >= 'a') && (*array <= 'z'))
            *array -= 32;
    }
    after = gettime();
    elapsed = after - before;
    printf("Simple              : %.3f GiB/s\n", to_bw(SIZE , elapsed));
    printf("After:  %.40s...\n",text);
}

/*
 * Bitwise version
 * b - 1 1 0 0 0 0 1
 * B - 1 0 0 0 0 0 1
 */
static void toupper_simple_bitwise(char * text) {
    double before, after, elapsed;

    printf("\nBefore:  %.40s...\n",text);
    before = gettime();
    char* array = text;
    for (; *array != '\0'; array++) {
        *array &= ~(1<<5);
    }
    after = gettime();
    elapsed = after - before;
    printf("Simple bitwise      : %.3f GiB/s\n", to_bw(SIZE , elapsed));
    printf("After:  %.40s...\n",text);
}

static void toupper_optimised_neon(char * text) {
    double before, after, elapsed;
    int i;
    uint8_t masklane = ~(1<<5);
    uint8x8_t mask = vdup_n_u8(masklane); /* set each lane to masklane */

    printf("\nBefore:  %.40s...\n",text);
    before = gettime();
    uint8x8_t* array = (uint8x8_t*) text;
    for (i = 0; i < SIZE / sizeof(int8x8_t); i++) {
        array[i] = vand_u8(array[i], mask);
    }
    after = gettime();
    elapsed = after - before;
    printf("NEON      : %.3f GiB/s\n", to_bw(SIZE , elapsed));
    printf("After:  %.40s...\n",text);
}

int main() {
    memset(array, 'a', SIZE);
    array[(int)(SIZE-1)] = '\0';
    computebw(array);

    memset(array, 'a', SIZE);
    array[(int)(SIZE-1)] = '\0';
    toupper_simple(array);

    memset(array, 'a', SIZE);
    array[(int)(SIZE-1)] = '\0';
    toupper_simple_bitwise(array);

    memset(array, 'a', SIZE);
    array[(int)(SIZE-1)] = '\0';
    toupper_optimised_neon(array);

    return 0;
}
