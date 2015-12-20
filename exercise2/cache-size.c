#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>
#include <string.h>

#define CACHE_LINE_SIZE 64
#define MICROS_PER_SECF 1000000.0
#define STRIDE 67
#define NR_OF_READS 10000000
#define KB 1024
#define MB 1024 * 1024

/* Fake a 64 bytes data structure */
typedef struct elements {
    uint64_t value;
    uint64_t index;
    uint64_t value2;
    uint64_t value3;
    uint64_t value4;
    uint64_t value5;
    uint64_t value6;
    void *next;
} element ;

/* Get the current time in seconds */
static inline double gettime(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((double) tv.tv_sec) + ((double) tv.tv_usec / (MICROS_PER_SECF));
}

/* Compare two doubles */
int cmpdoubles(const void * a, const void * b) {
   return ( *(double*)a - *(double*)b );
}

double benchmark(int N, int stride) {
    int sum = 0;
    FILE *debug;

    element * array = malloc(N * sizeof(element));
    memset(array, 0, N * sizeof(element));
    element * carrier;

    /* Initialize array */
    srand(time(NULL));
    carrier = array;
    for (int i = 0; i<N; i++) {
        carrier->value = rand();
        carrier->index = i;
        carrier->value2 = rand();
        carrier->value3 = rand();
        carrier->value4 = rand();
        carrier->value5 = rand();
        carrier->value6 = rand();
        carrier->next = array + ((i + stride) % N);
        carrier++;
    }

    /* Dry run to get cache filled if possible */
    sum = 0;
    carrier = array;
    for (int i=0; i<N; i++) {
        sum = sum + carrier->value;
        carrier++;
    }
    /* Ensure sum is used to prevent over-optimization */
    debug = fopen("/dev/null", "w");
    fprintf(debug, "%d", sum);
    fclose(debug);

    /* Run reads following next */
    sum = 0;
    carrier = array;
    double start = gettime();
    for (int i=0; i<NR_OF_READS; i++) {
        if ((i<N) && (i>0) && (carrier->index == 0) ) {
            free(array);
            return 0;
        }
        sum += carrier->value;
        carrier = carrier->next;
    }
    double stop = gettime();

    /* Ensure sum is used to prevent over-optimization */
    debug = fopen("/dev/null", "w");
    fprintf(debug, "%d", sum);
    fclose(debug);

    free(array);

    return (stop - start);
}

/* MAIN */
int main (int argc, char **argv) {
    double res;
    int i;

    if (sizeof(element) != CACHE_LINE_SIZE) {
        printf("[ERROR] Wrong element size detected.\n");
        exit(1);
    }

    FILE * log = fopen("cache-size.txt", "w");

    i=1;
    do {
        int size = i * KB;
        res = benchmark(size/CACHE_LINE_SIZE, STRIDE);
        if (res != 0)
            fprintf(log, "%20d\t%20.6f\n", size, res);
        i=i+10;
    } while (i<5120);

    fclose(log);

    return 0;

}
