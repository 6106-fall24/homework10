#include <bits/cstd++.h>

#define BLOCK_SIZE_LOG 0

uint64_t iterate(uint64_t *A, const uint64_t P, const uint64_t iterations)
{
    uint64_t loopSum = 0ull;
    uint64_t index = 1ull;
    for (uint64_t i = 0ull; i < (iterations >> BLOCK_SIZE_LOG); i++)
    {
        for (uint64_t j = 0ull; j < (1ull << BLOCK_SIZE_LOG); j++)
        {
            loopSum += A[index];
            index = index << 1ull;
            index = index >= P ? index - P : index;
        }
    }
}
