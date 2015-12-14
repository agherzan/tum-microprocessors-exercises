#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>
#include <stdint.h>

#define CACHE_LINE_SIZE 64
#define MICROS_PER_SECF 1000000.0
#define STRIDE 4160
#define NR_OF_READS 10000000
#define KB 1024
#define MB 1024 * 1024
#define TIMES 1

/* Fake a 64 bytes data structure */
typedef struct elements {
    uint64_t value;
    uint64_t value1;
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

double benchmark(int N) {
    int sum = 0;
    FILE *debug;

    //printf("[INFO] Initialize array...\n");

    element * array = malloc(N * sizeof(element));
    element * carrier;


    if (sizeof(element) != CACHE_LINE_SIZE) {
        printf("[ERROR] Wrong element size detected.\n");
        exit(1);
    }

    /* Initialize array */
    srand(time(NULL));
    carrier = array;
    for (int i = 0; i<N; i++) {
        if (((i + STRIDE) % N) == i)
            return 0;
        carrier->value = rand();
        carrier->value1 = rand();
        carrier->value2 = rand();
        carrier->value3 = rand();
        carrier->value4 = rand();
        carrier->value5 = rand();
        carrier->value6 = rand();
        carrier->next = array + ((i + STRIDE) % N);
        //printf("[DEBUG] a[%d] = %d -> %d\n",i , carrier->value, ((i + STRIDE) % N));
        carrier++;
    }

    /* Dry run to get cache filled if possible */
    sum = 0;
    carrier = array;
    for (int i=0; i<N; i++) {
        sum += carrier->value;
        carrier++;
    }
    /* Ensure sum is used to prevent over-optimization */
    debug = fopen("/dev/null", "w");
    fprintf(debug, "%d", sum);
    fclose(debug);

    /* Run reads following next */
    //printf("[INFO] Run reads...\n");
    sum = 0;
    carrier = array;
    double start = gettime();
    for (int i=0; i<NR_OF_READS; i++) {
        sum += carrier->value;
        //printf("[DEBUG] %d\n", carrier->value);
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

    int sizes[] = { 
        1 * KB, 4 * KB, 8 * KB, 16 * KB, 32 * KB,

        48 * KB, 64 * KB, 96 * KB, 128 * KB, 160 * KB, 192 * KB, 224 * KB, 256 * KB,

        512 * KB, 1 * MB, 1.5 * MB, 2 * MB, 2.5 * MB, 3 * MB, 3.5 * MB, 4 * MB,

        4.5 * MB, 5 * MB, 5.5 * MB, 6 * MB, 6.5 * MB, 7 * MB, 7.5 * MB, 8 * MB,
        8.5 * MB, 9 * MB, 9.5 * MB, 10 * MB, 10.5 * MB, 11 * MB, 11.5 * MB, 12 * MB,
        12.5 * MB, 13 * MB, 13.5 * MB, 14 * MB, 14.5 * MB, 15 * MB, 15.5 * MB, 16 * MB,
        16.5 * MB, 17 * MB, 17.5 * MB, 18 * MB, 18.5 * MB, 19 * MB, 19.5 * MB, 20 * MB,
        20.5 * MB, 21 * MB, 21.5 * MB, 22 * MB, 22.5 * MB, 23 * MB, 23.5 * MB, 24 * MB,
        24.5 * MB, 25 * MB, 25.5 * MB, 26 * MB, 26.5 * MB, 27 * MB, 27.5 * MB, 28 * MB,
        //100.5 * MB, 101 * MB, 101.5 * MB, 102 * MB, 102.5 * MB, 103 * MB, 103.5 * MB, 104 * MB,
        //200.5 * MB, 201 * MB, 201.5 * MB, 202 * MB, 202.5 * MB, 203 * MB, 203.5 * MB, 204 * MB,
    };

    FILE * log = fopen("cache-size.txt", "w");
    for (int i=0; i< sizeof(sizes) / sizeof (int); i++) {
        double times[TIMES];
        for (int try=0; try<TIMES; try++)
            times[try] = benchmark(sizes[i]/CACHE_LINE_SIZE);
        qsort(times, TIMES, sizeof(double), cmpdoubles);
        res = times[TIMES/2];
        if (res != 0)
            fprintf(log, "%20d\t%20.6f\n", sizes[i], res);
        // clean caches
        //FILE * drop = fopen("/proc/sys/vm/drop_caches", "w");
        //fprintf(drop, "3");
        //fclose(drop);
    }
    fclose(log);

    return 0;
}
