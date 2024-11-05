#pragma once

#include <inttypes.h>

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

char *createFormattedString(char *format, ...)
{
    va_list args1, args2;
    va_start(args1, format);
    va_copy(args2, args1);
    int len = vsnprintf(NULL, 0, format, args1) + 1;
    char *buf = malloc(len);
    vsnprintf(buf, len, format, args2);
    va_end(args1);
    va_end(args2);
    return buf;
}

uint64_t charStarToVal(char *argv, uint64_t minVal, uint64_t maxVal)
{
    uint64_t val = strtoul(argv, NULL, 10);
    if (val >= minVal && val <= maxVal)
    {
        return val;
    }
    assert(0); // Invalid range
}

typedef struct
{
    uint64_t loopSum;
    double nsPerAccess;
} ResultT;

char *getResultHeader()
{
    return createFormattedString("loopSum,nsPerAccess");
}

char *resultToString(ResultT result)
{
    return createFormattedString("%" PRIu64 ",%f", result.loopSum, result.nsPerAccess);
}
