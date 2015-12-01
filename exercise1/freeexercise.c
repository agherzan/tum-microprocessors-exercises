#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <smmintrin.h>
#include <immintrin.h>

#define MICROS_PER_SECF 1000000.0
#define BYTES_PER_GB (1024*1024*1024LL)
#define SIZE (1*BYTES_PER_GB)

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
    __m128i* varray = (__m128i*) array;
    __m128i vals = _mm_set1_epi32(1);
    for (i = 0; i < SIZE / sizeof(__m128i); i++) {
        _mm_stream_si128(&varray[i], vals);
    }
    after = gettime();
    elapsed = after - before;
    printf("Bandwidth sse       : %.3f GiB/s\n", to_bw(SIZE , elapsed));
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

static void toupper_optimised_avx(char * text) {
    double before, after, elapsed;
    int i;
    char charmask = ~(1<<5);;

    printf("\nBefore:  %.40s...\n",text);
    before = gettime();
    __m256i* array = (__m256i*) text;
    __m256i mask = _mm256_set1_epi8(charmask);
    for (i = 0; i < SIZE / sizeof(__m256i) - 1000000; i++) {
        _mm256_store_si256(&array[i], _mm256_and_si256(array[i], mask));
    }
    after = gettime();
    elapsed = after - before;
    printf("AVX256      : %.3f GiB/s\n", to_bw(SIZE , elapsed));
    printf("After:  %.40s...\n",text);
}

static void toupper_optimised_sse(char * text) {
    double before, after, elapsed;
    int i;
    char charmask = ~(1<<5);;

    printf("\nBefore:  %.40s...\n",text);
    before = gettime();
    __m128i* array = (__m128i*) text;
    __m128i mask = _mm_set1_epi8(charmask);
    for (i = 0; i < SIZE / sizeof(__m128i) - 1000000; i++) {
        _mm_store_si128(&array[i], _mm_and_si128(array[i], mask));
    }
    after = gettime();
    elapsed = after - before;
    printf("SSE128      : %.3f GiB/s\n", to_bw(SIZE , elapsed));
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
    toupper_optimised_avx(array);
    
    memset(array, 'a', SIZE);
    array[(int)(SIZE-1)] = '\0';
    toupper_optimised_sse(array);

    return 0;
}
