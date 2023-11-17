#include <benchmark/benchmark.h>
static bool isPrime(int n)
{
    if (n < 2)
    {
        return false;
    }
    for (int i = 2; i * i <= n; ++i)
    {
        if (n % i == 0)
        {
            return false;
        }
    }
    return true;
}

static int findPrimes(int n)
{
    int count = 0;
    for (int i = 2; i <= n; ++i)
    {
        if (isPrime(i))
        {
            count++;
        }
    }
    return count;
}

static void BM_findPrimes(benchmark::State &state)
{
    for (auto _ : state)
    {
        int n = state.range(0);
        int count = findPrimes(n);
        benchmark::DoNotOptimize(count);
    }
}

BENCHMARK(BM_findPrimes)->Range(1, 100000);
BENCHMARK_MAIN();
