#include "functions.h"
#include "writer_funcs.h"

extern po::options_description creator;
extern po::variables_map vm;

void AddBench()
{
    CreateDirIfNotExist("bench");
    WriteBenchmark();
    LOGINFO("Benchmark template generated!");
    LOGINFO("Remember to add the following lines into your CMakeLists.txt to take effect:");
    fmt::print(R"(
option(BUILD_BENCHMARKS "build benchmark tests" OFF)
if(BUILD_BENCHMARKS)
    find_package(benchmark REQUIRED)
    add_subdirectory(bench)
endif()

)");
}