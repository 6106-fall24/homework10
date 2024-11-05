#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "fasttime.h"
#include "common.h"
#include <assert.h>
#include <math.h>
#include <float.h>

typedef struct
{
    uint64_t expectedBlockLength;
    uint64_t bufferLengthLog;
    uint64_t trials;
    uint64_t prefetchDistanceInBytes;
} ConfigT;

char *getConfigHeader()
{
    return createFormattedString("HW10,expBlockBytes,totalKB,trials,prefetchDistanceInBytes");
}

char *configToString(ConfigT config)
{
    return createFormattedString("HW10,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64, 8 * config.expectedBlockLength, 1ull << (config.bufferLengthLog - 7ull), config.trials, config.prefetchDistanceInBytes);
}

static inline uint64_t getGeometricSample(const double oneMinusP)
{
    const double uniformSample = ((double)rand()) / ((double)RAND_MAX);
    return (uint64_t)MAX(1.0, 1.0 + (log10(uniformSample) / log10(oneMinusP)));
}

static inline uint64_t *init(const ConfigT config)
{
    const uint64_t bufferLength = 1ull << config.bufferLengthLog;
    uint64_t *data = malloc(sizeof(uint64_t) * bufferLength);
    const double oneMinusP = 1.0 - (1.0 / ((double)config.expectedBlockLength));

    for (uint64_t i = 0ull; i < bufferLength; i++)
    {
        data[i] = getGeometricSample(oneMinusP);
        assert(data[i] > 0ull);
    }
    return data;
}

static inline ResultT testPrefetchDistance(const ConfigT config)
{
    const uint64_t bufferLength = 1ull << config.bufferLengthLog;
    const uint64_t bufferMask = bufferLength - 1ull;

    uint64_t *data = init(config);
    double nsPerAccess = DBL_MAX;
    uint64_t loopSum = 0ull;

    for (uint64_t trial = 0; trial < config.trials; trial++)
    {

        const fasttime_t t1 = gettime();

        loopSum = 0ull;
        uint64_t index = 0ull;
        for (uint64_t i = 0ull; i < bufferLength; i++)
        {
            // --------------------------- //
            // Put prefetch code here
            // --------------------------- //
            loopSum += data[index];
            index = (index + data[index]) & bufferMask;
        }
        const fasttime_t t2 = gettime();
        nsPerAccess = MIN(((tdiff(t1, t2) * 1e9) / ((double)bufferLength)), nsPerAccess);
    }
    free(data);
    return (ResultT){loopSum, nsPerAccess};
}

#define BOUND(x, min, max) ((x) <= (min) ? (min) : ((x) >= (max) ? (max) : (x)))

int main(int argc, char *argv[])
{
    // ./sweep_phase expected_block_length bytes_log trials prefetch_distance_in_bytes
    if (argc == 2)
    {
        FILE *fp = fopen(argv[1], "r");
        if (!fp) {
            printf("Could not open file %s\n", argv[1]);
            return 1;
        }

        char line[256];
        while (fgets(line, sizeof(line), fp))
        {
            ConfigT config;
            uint32_t expected_block_length, buffer_length_log, trials, prefetch_distance;
            
            if (sscanf(line, "%u %u %u %u", &expected_block_length, &buffer_length_log,
                      &trials, &prefetch_distance) == 4)
            {
                config.expectedBlockLength = BOUND(expected_block_length, 1, 1024) / 8;
                config.bufferLengthLog = BOUND(buffer_length_log, 14, 28) - 3;
                config.trials = BOUND(trials, 1, 100);
                config.prefetchDistanceInBytes = BOUND(prefetch_distance, 0, 16384);
                
                const ResultT result = testPrefetchDistance(config);
                char *configString = configToString(config);
                char *resultString = resultToString(result);
                printf("%s,%s\n", configString, resultString);
                free(configString);
                free(resultString);
            }
        }
        
        fclose(fp);
        return 0;
    }
    char *configHeader = getConfigHeader();
    char *resultHeader = getResultHeader();
    printf("%s,%s\n", configHeader, resultHeader);
    free(configHeader);
    free(resultHeader);
    return 0;
}
