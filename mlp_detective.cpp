#include <cstddef>
#include <chrono>
#include <cassert>
#include <iostream>
#include <sstream>
#include <fstream>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>

enum class PatternT
{
    Cycle,
    Sequential,
    MediumStride, // 8 - so, every cache line -> L2 prefetcher picks it up, worst-cast bw behavior
    LongStride    // 512 - so, every 4KB -> L2 prefetcher shouldn't pick this up, but L1 prefetcher would
};
static std::string enumToName(PatternT d)
{
    switch (d)
    {
    case PatternT::Cycle:
        return std::string("Cycle");
    case PatternT::Sequential:
        return std::string("Sequential");
    case PatternT::MediumStride:
        return std::string("MediumStride");
    case PatternT::LongStride:
        return std::string("LongStride");
    default:
        assert(false); // Unsupported PatternT
    }
}
static PatternT characterToPatternT(char c)
{
    switch (c)
    {
    case 'c':
        return PatternT::Cycle;
    case 's':
        return PatternT::Sequential;
    case 'm':
        return PatternT::MediumStride;
    case 'l':
        return PatternT::LongStride;
    default:
        assert(false); // Unsupported PatternT
    }
}

enum class NextIndexT
{
    CalculateNext,
    ReadNext
};
static std::string enumToName(NextIndexT d)
{
    switch (d)
    {
    case NextIndexT::CalculateNext:
        return std::string("CalculateNext");
    case NextIndexT::ReadNext:
        return std::string("ReadNext");
    default:
        assert(false); // Unsupported NextIndexT
    }
}

static NextIndexT characterToNextIndexT(char c)
{
    switch (c)
    {
    case 'c':
        return NextIndexT::CalculateNext;
    case 'r':
        return NextIndexT::ReadNext;
    default:
        assert(false); // Unsupported NextIndexT
    }
}

enum class DataStructureT
{
    Array,
    Vector,
    Map,
    Unordered_map
};

static std::string enumToName(DataStructureT d)
{
    switch (d)
    {
    case DataStructureT::Array:
        return std::string("Array");
    case DataStructureT::Vector:
        return std::string("Vector");
    case DataStructureT::Map:
        return std::string("Map");
    case DataStructureT::Unordered_map:
        return std::string("Unordered_map");
    default:
        assert(false); // Unsupported DataStructureT
    }
}

static DataStructureT characterToDataStructureT(char c)
{
    switch (c)
    {
    case 'a':
        return DataStructureT::Array;
    case 'v':
        return DataStructureT::Vector;
    case 'm':
        return DataStructureT::Map;
    case 'u':
        return DataStructureT::Unordered_map;
    default:
        assert(false); // Unsupported DataStructureT
    }
}

struct ConfigT
{
    PatternT pattern;
    NextIndexT nextIndex;
    DataStructureT containerFamily;
    uint64_t pLog;
    uint64_t p;
    uint64_t blockLog;
    uint64_t trials;

    static std::string getHeader() { return std::string("HW10,pLog,totalKB,p,containerFamily,pattern,nextIndex,blockLog,trials"); }
    std::string toString() const
    {
        return "HW10," + std::to_string(pLog) + "," + std::to_string(static_cast<double>(p) / 128.0) + "," + std::to_string(p) + "," + enumToName(containerFamily) + "," + enumToName(pattern) + "," + enumToName(nextIndex) + "," + std::to_string(blockLog) + "," + std::to_string(trials);
    }
};

struct ResultT
{
    uint64_t loopSum;
    double nsPerAccess;

    static std::string getHeader() { return std::string("loopSum,nsPerAccess"); }
    std::string toString() const { return std::to_string(loopSum) + "," + std::to_string(nsPerAccess); }
};

template <PatternT Pattern>
static inline uint64_t getNext(const uint64_t index, const uint64_t P)
{
    if constexpr (Pattern == PatternT::Cycle)
    {
        auto next = index << 1ull;
        return next >= P ? next - P : next;
    }
    else if constexpr (Pattern == PatternT::Sequential)
    {
        return index >= (P - 1) ? 1ull : index + 1;
    }
    else if constexpr (Pattern == PatternT::MediumStride)
    {
        const auto next = index + 7; // assumes P > 7
        return next >= P ? next - (P - 1) : next;
    }
    else if constexpr (Pattern == PatternT::LongStride)
    {
        const auto next = index + 509; // assumes P > 509
        return next >= P ? next - (P - 1) : next;
    }
}

template <typename ContainerT, PatternT Pattern>
static inline void init(ContainerT &data, const uint64_t P)
{
    uint64_t index = 1ull;
    for (uint64_t i = 0; i < P; i++)
    {
        const auto next = getNext<Pattern>(index, P);
        data[index] = next;
        index = next;
    }
}

template <typename ContainerT, PatternT Pattern, NextIndexT Next>
static inline void traverseNext(ContainerT &data, const uint64_t P, uint64_t &index, uint64_t &loopSum)
{
    loopSum += data[index];
    if constexpr (Next == NextIndexT::CalculateNext)
    {
        index = getNext<Pattern>(index, P);
    }
    else if constexpr (Next == NextIndexT::ReadNext)
    {
        index = data[index];
    }
}

template <typename ContainerT, PatternT Pattern, NextIndexT Next, uint64_t B_Log>
static inline ResultT traverseOnetrial(const ConfigT config, ContainerT &data)
{
    constexpr uint64_t blockSize = 1ull << B_Log;
    constexpr uint64_t remainderMask = blockSize - 1;
    uint64_t index = 1ull;
    uint64_t loopSum = 0ull;
    const uint64_t P = config.p;
    const uint64_t numBlocks = (P - 1) >> B_Log;
    const uint64_t remainderElements = (P - 1) & remainderMask;

    const auto t1 = std::chrono::high_resolution_clock::now();

    for (uint64_t i = 0; i < numBlocks; i++)
    {
        for (uint64_t j = 0; j < blockSize; j++)
        {
            traverseNext<ContainerT, Pattern, Next>(data, P, index, loopSum);
        }
    }
    for (uint64_t i = 0; i < remainderElements; i++)
    {
        traverseNext<ContainerT, Pattern, Next>(data, P, index, loopSum);
    }
    const auto t2 = std::chrono::high_resolution_clock::now();
    const std::chrono::duration<double, std::nano> duration = t2 - t1;
    const auto nsPerAccess = duration.count() / static_cast<double>(P - 1);
    return ResultT({loopSum, nsPerAccess});
}

template <typename ContainerT, PatternT Pattern, NextIndexT Next, uint64_t B_Log>
static inline ResultT traverse(const ConfigT config, ContainerT &data)
{
    std::vector<ResultT> results;
    for (uint64_t i = 0ull; i < config.trials; i++)
    {
        results.push_back(traverseOnetrial<ContainerT, Pattern, Next, B_Log>(config, data));
    }
    ResultT bestResult = results[0];
    for (uint64_t i = 1ull; i < config.trials; i++)
    {
        assert(bestResult.loopSum == results[i].loopSum);
        if (results[i].nsPerAccess < bestResult.nsPerAccess)
        {
            bestResult = results[i];
        }
    }
    return bestResult;
}

template <typename ContainerT, PatternT Pattern, NextIndexT Next, uint64_t B_Log>
static inline ResultT testTraversal(const ConfigT config, ContainerT &data)
{
    init<ContainerT, Pattern>(data, config.p);
    return traverse<ContainerT, Pattern, Next, B_Log>(config, data);
}

template <typename ContainerT, PatternT Pattern, uint64_t B_Log>
static inline ResultT testNextIndex(const ConfigT config, ContainerT &data)
{
    switch (config.nextIndex)
    {
    case NextIndexT::CalculateNext:
        return testTraversal<ContainerT, Pattern, NextIndexT::CalculateNext, B_Log>(config, data);
    case NextIndexT::ReadNext:
        return testTraversal<ContainerT, Pattern, NextIndexT::ReadNext, B_Log>(config, data);
    default:
        assert(false); // Unsupported NextIndexT
    }
}

template <typename ContainerT, uint64_t B_Log>
static inline ResultT testAccessPattern(const ConfigT config, ContainerT &data)
{
    switch (config.pattern)
    {
    case PatternT::Cycle:
        return testNextIndex<ContainerT, PatternT::Cycle, B_Log>(config, data);
    case PatternT::Sequential:
        return testNextIndex<ContainerT, PatternT::Sequential, B_Log>(config, data);
    case PatternT::MediumStride:
        return testNextIndex<ContainerT, PatternT::MediumStride, B_Log>(config, data);
    case PatternT::LongStride:
        return testNextIndex<ContainerT, PatternT::LongStride, B_Log>(config, data);
    default:
        assert(false); // Unsupported PatternT
    }
}

template <typename ContainerT>
static inline ResultT testBlockingFactor(const ConfigT config, ContainerT &data)
{
    switch (config.blockLog)
    {
    case 0:
        return testAccessPattern<ContainerT, 0>(config, data);
    case 1:
        return testAccessPattern<ContainerT, 1>(config, data);
    case 2:
        return testAccessPattern<ContainerT, 2>(config, data);
    case 3:
        return testAccessPattern<ContainerT, 3>(config, data);
    case 4:
        return testAccessPattern<ContainerT, 4>(config, data);
    default:
        assert(false); // Unsupported block size
    }
}

static inline ResultT testDataStructure(const ConfigT config)
{
    switch (config.containerFamily)
    {
    case DataStructureT::Array:
    {
        uint64_t *data = new uint64_t[config.p];
        const auto result = testBlockingFactor<uint64_t *>(config, data);
        delete[] data;
        return result;
    }
    case DataStructureT::Vector:
    {
        std::vector<uint64_t> data(config.p, 0ull);
        return testBlockingFactor<std::vector<uint64_t>>(config, data);
    }
    case DataStructureT::Map:
    {
        std::map<uint64_t, uint64_t> data;
        return testBlockingFactor<std::map<uint64_t, uint64_t>>(config, data);
    }
    case DataStructureT::Unordered_map:
    {
        std::unordered_map<uint64_t, uint64_t> data;
        return testBlockingFactor<std::unordered_map<uint64_t, uint64_t>>(config, data);
    }
    default:
        assert(false); // Unsupported DataStructureT
    }
}

void getPrimes(uint64_t p_start, uint64_t p_end)
{
    std::vector<uint64_t> primes;
    for (uint64_t i = p_start; i <= p_end; i++)
    {
        uint64_t p = (1 << i) - 1;
        for (;; p -= 2)
        {
            uint64_t sums[4] = {0ull, 0ull, 0ull, 0ull};
            uint64_t index[4] = {1ull, 1ull, 1ull, 1ull};

            std::vector<bool> visited(p, false);
            bool isPrimitiveRoot = true;

            for (uint64_t k = 1; k < p; k++)
            {
                for (uint64_t m = 0; m < 4; m++)
                {
                    sums[m] += index[m];
                }
                index[0] = getNext<PatternT::Cycle>(index[0], p);
                index[1] = getNext<PatternT::Sequential>(index[1], p);
                index[2] = getNext<PatternT::MediumStride>(index[2], p);
                index[3] = getNext<PatternT::LongStride>(index[3], p);

                isPrimitiveRoot &= !visited[index[0]];
                visited[index[0]] = true;
            }

            if ((sums[0] == (p * (p - 1) / 2)) && (sums[0] == sums[1]) && (sums[1] == sums[2]) && (sums[2] == sums[3]) && isPrimitiveRoot)
            {
                primes.push_back(p);
                break;
            }
        }
    }
    std::cout << "std::vector<uint64_t> primes({" << primes[0];

    for (uint64_t i = 1; i < primes.size(); i++)
    {
        std::cout << ", " << primes[i];
    }
    std::cout << "});" << std::endl;
}

template <typename IntegralType>
static const IntegralType charStarToVal(char *argv, IntegralType minVal, IntegralType maxVal)
{
    std::stringstream ss(argv);
    IntegralType val;
    ss >> val;
    if (val >= minVal && val <= maxVal)
    {
        return val;
    }
    assert(false); // Invalid range
}

int main(int argc, char *argv[])
{
    // getPrimes(10, 25);
    // mlp_detective data_structure{a,v,m,u} pattern{c,s,m,l} nextIndex{r,c} pow_log[10:25] block_log[0:4] trials
    std::vector<uint64_t> primes({947, 2029, 4093, 8179, 16363, 32749, 65371, 131059, 262139, 524269, 1048571, 2097133, 4194187, 8388587, 16776989, 33554371});

    if (argc == 2)
    {
        std::ifstream argsFile(argv[1]);
        std::string line;
        
        while (std::getline(argsFile, line))
        {
            std::istringstream iss(line);
            std::vector<std::string> args;
            std::string arg;
            
            while (iss >> arg) {
                args.push_back(arg);
            }
            
            if (args.size() == 6) {
                ConfigT config;
                config.containerFamily = characterToDataStructureT(args[0][0]);
                config.pattern = characterToPatternT(args[1][0]);
                config.nextIndex = characterToNextIndexT(args[2][0]);
                config.pLog = std::stoul(args[3]) - 3;
                config.p = primes[config.pLog - 10];
                config.blockLog = std::stoul(args[4]);
                config.trials = std::stoul(args[5]);
                ResultT result = testDataStructure(config);
                std::cout << config.toString() << "," << result.toString() << std::endl;
            }
        }
        return 0;
    }
    std::cout << ConfigT::getHeader() << "," << ResultT::getHeader() << std::endl;
    return 0;
}
