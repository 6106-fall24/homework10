#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <stdarg.h>
#include "fasttime.h"
#include "common.h"
#include <assert.h>
#include <float.h>

typedef struct
{
    uint64_t nextIndex;
    uint64_t *innerArray;
} OuterNodeT;

typedef struct
{
    uint64_t p;
    uint64_t totalAccesses;
} TraversalConfigT;

char *getTraversalConfigHeader(char *prefix)
{
    return createFormattedString("%sArrayBytes,%sTotalAccesses", prefix, prefix);
}

char *traversalConfigToString(TraversalConfigT self)
{
    return createFormattedString("%" PRIu64 ",%" PRIu64, 8 * self.p, self.totalAccesses);
}

typedef struct
{
    TraversalConfigT inner;
    TraversalConfigT outer;
    int enablePrefetch;
    int trials;
} ConfigT;

char *getConfigHeader()
{
    char *innerHeader = getTraversalConfigHeader("inner");
    char *outerHeader = getTraversalConfigHeader("outer");
    char *result = createFormattedString("HW10,%s,%s,totalKB,enablePrefetch", innerHeader, outerHeader);
    free(innerHeader);
    free(outerHeader);
    return result;
}

char *configToString(ConfigT self)
{
    char *innerString = traversalConfigToString(self.inner);
    char *outerString = traversalConfigToString(self.outer);
    char *result = createFormattedString("HW10,%s,%s,%d,%d", innerString, outerString, (int)(self.inner.p * self.outer.p) / 128, self.enablePrefetch);
    free(innerString);
    free(outerString);
    return result;
}

static inline uint64_t getNext(const uint64_t index, const uint64_t P)
{
    uint64_t next = index << 1ull;
    return next >= P ? next - P : next;
}

static inline OuterNodeT *init(const ConfigT config)
{
    OuterNodeT *data = malloc(sizeof(OuterNodeT) * config.outer.p);
    for (uint64_t i = 0; i < config.outer.p; i++)
    {
        data[i].innerArray = malloc(sizeof(uint64_t) * config.inner.p);
    }
    uint64_t outerIndex = 1ull;
    for (uint64_t i = 0; i < config.outer.p; i++)
    {
        const uint64_t nextOuterIndex = getNext(outerIndex, config.outer.p);
        data[outerIndex].nextIndex = nextOuterIndex;
        uint64_t innerIndex = 1ull;
        for (uint64_t j = 0; j < config.inner.p; j++)
        {
            const uint64_t nextInnerIndex = getNext(innerIndex, config.inner.p);
            data[outerIndex].innerArray[innerIndex] = nextInnerIndex;
            innerIndex = nextInnerIndex;
        }
        outerIndex = nextOuterIndex;
    }
    return data;
}

static inline void deallocate(const ConfigT config, OuterNodeT *data)
{
    for (uint64_t i = 0; i < config.outer.p; i++)
    {
        free(data[i].innerArray);
    }
    free(data);
}

static inline ResultT traverse(const ConfigT config, OuterNodeT *data)
{
    double nsPerAccess = DBL_MAX;
    uint64_t loopSum = 0ull;

    for (int trial = 0; trial < config.trials; trial++)
    {
        loopSum = 0ull;
        uint64_t innerIndex = 1ull;
        uint64_t outerIndex = 1ull;
        const fasttime_t t1 = gettime();

        for (uint64_t i = 0; i < config.outer.totalAccesses; i++)
        {
            uint64_t *innerArray = data[outerIndex].innerArray;
            // --------------------------- //
            // Put prefetch code here
            // --------------------------- //
            for (uint64_t j = 0; j < config.inner.totalAccesses; j++)
            {
                loopSum += innerArray[innerIndex];
                innerIndex = innerArray[innerIndex];
            }
            outerIndex = data[outerIndex].nextIndex;
        }
        const fasttime_t t2 = gettime();
        nsPerAccess = MIN((tdiff(t1, t2) * 1e9) / (config.inner.totalAccesses * config.outer.totalAccesses), nsPerAccess);
    }
    return (ResultT){loopSum, nsPerAccess};
}

static inline ResultT testTraversal(const ConfigT config)
{
    OuterNodeT *data = init(config);
    const ResultT result = traverse(config, data);
    deallocate(config, data);
    return result;
}

#define BOUND(x, min, max) ((x) <= (min) ? (min) : ((x) >= (max) ? (max) : (x)))

int main(int argc, char *argv[])
{
    uint64_t primes[] = {29, 61, 107, 227, 509, 947, 2029, 4093, 8179, 16363, 32749, 65371, 131059, 262139, 524269, 1048571, 2097133, 4194187, 8388587};
  
    if (argc == 2)
    {
              
        FILE *fp = fopen(argv[1], "r");
        if (!fp) {
            printf("Could not open file %s\n", argv[1]);
            return 1;
        }

        char line[256];
        while (fgets(line, sizeof(line), fp)) {
            ConfigT config;
            uint64_t totalElementsLog, totalAccesses, innerElementsLog;
            uint64_t enablePrefetch, trials;
            
            if (sscanf(line, "%lu %lu %lu %lu %lu %lu", 
                &totalElementsLog, &totalAccesses, &innerElementsLog,
                &config.inner.totalAccesses, &enablePrefetch, &trials) == 6) {
                
                totalElementsLog = BOUND(totalElementsLog, 13, 28) - 3;
                totalAccesses = BOUND(totalAccesses, 1, 1000000000);
                innerElementsLog = BOUND(innerElementsLog, 8, 15) - 3;
                config.inner.p = primes[innerElementsLog - 5];
                config.outer.p = primes[totalElementsLog - innerElementsLog - 5];
                config.inner.totalAccesses = BOUND(config.inner.totalAccesses, 1, 1024);
                config.outer.totalAccesses = totalAccesses / config.inner.totalAccesses;
                config.enablePrefetch = BOUND(enablePrefetch, 0, 1);
                config.trials = BOUND(trials, 1, 100);

                const ResultT result = testTraversal(config);
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
