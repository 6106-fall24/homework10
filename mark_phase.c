#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <stdarg.h>
#include "fasttime.h"
#include "common.h"
#include <assert.h>
#include <math.h>
#include <float.h>

#define MAX_NEIGHBORS 15

typedef struct
{
    uint32_t numLivePredecessors;
    uint32_t neighbors[MAX_NEIGHBORS];
} NodeT;

typedef struct
{
    uint32_t numNodes;
    uint32_t numNeighbors;
    uint32_t numRoots;
    uint32_t trials;
    uint32_t prefetchDistance;
} ConfigT;

char *getConfigHeader()
{
    return createFormattedString("HW10,totalKB,numNodes,numNeighbors,numRoots,trials,prefetchDistance");
}

char *configToString(ConfigT config)
{
    return createFormattedString("HW10,%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64 ",%" PRIu64,
                                 config.numNodes / 16, config.numNodes, config.numNeighbors, config.numRoots, config.trials, config.prefetchDistance);
}

static inline uint32_t getUniformSample(const uint32_t numNodes)
{
    const double uniformSample = (((double)rand()) / ((double)RAND_MAX)) * (double)numNodes;
    return MIN(numNodes - 1, (uint32_t)uniformSample);
}

static inline NodeT *init(const ConfigT config)
{
    NodeT *data = malloc(sizeof(NodeT) * config.numNodes);

    for (uint32_t i = 0ull; i < config.numNodes; i++)
    {
        for (uint32_t j = 0ull; j < config.numNeighbors; j++)
        {
            data[i].neighbors[j] = getUniformSample(config.numNodes);
        }
    }
    return data;
}

static inline ResultT testPrefetchDistance(const ConfigT config)
{
    NodeT *data = init(config);
    uint32_t *queue = malloc(sizeof(uint32_t) * config.numNodes * MAX_NEIGHBORS);
    double nsPerAccess = DBL_MAX;
    uint32_t loopSum = 0ull;

    for (uint32_t trial = 0ull; trial < config.trials; trial++)
    {
        uint32_t head = 0ull;
        uint32_t tail = 0ull;
        loopSum = 0ull;

        for (uint32_t i = 0; i < config.numNodes; i++)
        {
            data[i].numLivePredecessors = 0ul;
        }

        const fasttime_t t1 = gettime();

        for (uint32_t i = 0; i < config.numRoots; i++)
        {
            queue[head++] = getUniformSample(config.numNodes);
        }

        while (head != tail)
        {
            // --------------------------- //
            // Put prefetch code here
            // --------------------------- //
            NodeT *node = &data[queue[tail++]];
            if (node->numLivePredecessors == 0ull)
            {
                for (uint32_t i = 0ull; i < config.numNeighbors; i++)
                {
                    queue[head++] = node->neighbors[i];
                }
            }
            node->numLivePredecessors++;
            loopSum++;
        }
        const fasttime_t t2 = gettime();
        nsPerAccess = MIN(((tdiff(t1, t2) * 1e9) / ((double)loopSum)), nsPerAccess);
    }

    return (ResultT){loopSum, nsPerAccess};
}
#define BOUND(x, min, max) ((x) <= (min) ? (min) : ((x) >= (max) ? (max) : (x)))


int main(int argc, char *argv[])
{
    // ./mark_phase num_nodes num_neighbors num_roots trials prefetch_distance
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
            uint64_t num_nodes, num_neighbors, num_roots, trials, prefetch_distance;
            
            if (sscanf(line, "%lu %lu %lu %lu %lu", &num_nodes, &num_neighbors, &num_roots, 
                      &trials, &prefetch_distance) == 5)
            {
                config.numNodes = BOUND(num_nodes, 1ull, 1ull << 30ull);
                config.numNeighbors = BOUND(num_neighbors, 1ull, MAX_NEIGHBORS);
                config.numRoots = BOUND(num_roots, 1ull, config.numNodes);
                config.trials = BOUND(trials, 1ull, 1000ull);
                config.prefetchDistance = BOUND(prefetch_distance, 0ull, config.numNodes);
                
                const ResultT result = testPrefetchDistance(config);
                printf("%s,%s\n", configToString(config), resultToString(result));
            }
        }
        
        fclose(fp);
        return 0;
    }
    printf("%s,%s\n", getConfigHeader(), getResultHeader());
    return 0;
}
