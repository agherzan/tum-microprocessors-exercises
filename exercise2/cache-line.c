#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <time.h>

#define SIZE 1024*4096*256
#define MICROS_PER_SECF 1000000.0
#define TIMES 10 // times to run to get median

/*
 * BIG array please  - to ensure that at the end of the reading starting elements are
 * already out of cache
 */
char array[SIZE];

/* Get the current time in seconds */
static inline double gettime(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return ((double) tv.tv_sec) + ((double) tv.tv_usec / (MICROS_PER_SECF));
}

/* Compare two floats */
int cmpfloats(const void * a, const void * b) {
   return ( *(float*)a - *(float*)b );
}

/* Read the array stride argument */
int readarray(int stride) {
    int sum = 0;
    float times[TIMES];

    for (int j=0; j<TIMES; j++) {
        double start = gettime();
        for (int i = 0; i<SIZE; i=i+stride) {
            sum += array[i];
        }
        double stop = gettime();
        times[j] = stop - start;
    }

    /* Some calculations */
    double bcount = (1 + (SIZE-1) / (float)stride);
    qsort(times, TIMES, sizeof(float), cmpfloats);
    double secs = times[TIMES/2];
    double bps = bcount / secs;
    printf("%20d\t%20.3f\n",stride, bps); // Get for each stride the bytes per seconds

    /* Ensure sum is used to prevent over-optimization */
    FILE *debug = fopen("/dev/null", "w");
    fprintf(debug, "%d", sum);
}

/* MAIN */
int main (int argc, char **argv) {
    int maxstride = 1000;

    /* Initialize array */
    srand(time(NULL));
    for (int i = 0; i<SIZE; i++) {
        array[i] = rand() % 128;
    }

    /* Read array with stride */
    if (argc == 2)
        maxstride = atoi(argv[1]);
    for (int stride = 1; stride <= maxstride; stride++)
        readarray(stride);

    return 0;
}
