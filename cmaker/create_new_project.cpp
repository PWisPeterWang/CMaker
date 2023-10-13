#include "functions.h"

extern po::options_description creator;
extern po::variables_map vm;

enum class RepoType
{
    STATIC,
    SHARED,
    EXECUTABLE,
};

static RepoType repo_type = RepoType::STATIC;
static std::string repo_name = "undefined";
static std::string upper_name;

static void CreateRepoDirs();
static void WriteCMakeLists();
static void WriteUnitTests();
static void WriteBenchmark();
static void WriteSrcAndHeader();

void CreateNewProject()
{
    if (!vm.count("name"))
    {
        ERROR("missing repo name for creating project");
        PrintUsageAndQuit(creator);
    }
    repo_name = vm["name"].as<std::string>();
    if (vm.count("shared"))
    {
        INFO("creating repo for shared library: {}", repo_name);
        repo_type = RepoType::SHARED;
    }
    else if (vm.count("exe"))
    {
        INFO("creating repo for executable: {}", repo_name);
        repo_type = RepoType::EXECUTABLE;
    }
    else
    {
        INFO("creating repo for static library: {}", repo_name);
    }

    CreateRepoDirs();
}

void CreateRepoDirs()
{
    if (!CreateDirIfNotExist(repo_name))
    {
        ERROR("abort!");
        exit(1);
    }

    // with chdir to
    {
        PushD _(repo_name);

        CreateDirIfNotExist("src");
        CreateDirIfNotExist(repo_name);
        CreateDirIfNotExist("cmake_modules");
        CreateDirIfNotExist("thirdparty");
        CreateDirIfNotExist("bench");
        CreateDirIfNotExist("unit_test");
        WriteCMakeLists();
        WriteUnitTests();
        WriteBenchmark();
        WriteSrcAndHeader();

        std::system("git init");
    }
    // and back to orgin cwd
}

void WriteCMakeLists()
{
    upper_name = ToUpper(repo_name);
    std::ofstream cmakelist("CMakeLists.txt");
    cmakelist << "cmake_minimum_required(VERSION 3.21)\n\n"
              << "project(" << repo_name << " LANGUAGES CXX C)\n\n";
    cmakelist << "# edit the following settings as you desire\n"
              << "set(CMAKE_CXX_STANDARD 17)\n"
              << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n"
              << "set(CMAKE_CXX_STANDARD_EXTENSION OFF)\n"
              << "add_compile_options(-Wfatal-errors)\n\n";
    cmakelist << "# edit the following line to add your cmake modules\n"
              << "list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)\n";
    cmakelist << "# options flags to turn on/off unit tests and benchmarks\n"
              << "option(BUILD_TESTS \"build unit tests\" OFF)\n"
              << "option(BUILD_BENCHMARKS \"build benchmark tests\" OFF)\n";
    cmakelist << "# edit the following line to add your dependencies\n"
              << "find_package(Threads REQUIRED)\n\n";
    // for executable repo, scan the 'repo_name' dir to add all cpp files as it's SRCS
    if (RepoType::EXECUTABLE == repo_type)
    {
        cmakelist << fmt::format("file(GLOB EXECUTABLE_SRC \"{}/*.cpp\")\n", repo_name)
                  << "add_executable(${PROJECT_NAME} ${EXECUTABLE_SRC})\n";
        cmakelist << "target_include_directories(${PROJECT_NAME} PRIVATE\n    "
                  << fmt::format("${{PROJECT_SOURCE_DIR}}/{})\n\n", repo_name);
    }
    else // for library repo, scan the 'src' dir to add all cpp files as it's SRCS
    {
        cmakelist << "file(GLOB LIBRARY_SRC \"src/*.cpp\")\n"
                  << "add_library(${PROJECT_NAME} "
                  << (repo_type == RepoType::SHARED ? "SHARED" : "STATIC")
                  << " \"${LIBRARY_SRC}\")\n";
        cmakelist << "# you may add more dependencies' header dir here\n";
        cmakelist << "target_include_directories(${PROJECT_NAME}\n"
                  << "    PUBLIC\n"
                  << "        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/" << repo_name << ">\n"
                  << "        $<INSTALL_INTERFACE:" << repo_name << ">\n"
                  << "    PRIVATE\n"
                  << "        ${PROJECT_SOURCE_DIR}/src)\n\n";
    }

    // link example
    cmakelist << "# edit the following line to link your dependencies libraries\n"
              << "target_link_libraries(${PROJECT_NAME} PRIVATE Threads::Threads)\n\n";

    // unit tests
    cmakelist << "#if called with -DBUILD_TESTS the unit test targets will be enabled\n";
    cmakelist << "if(BUILD_TESTS)\n"
              << "    find_package(GTest REQUIRED)\n"
              << "    enable_testing()\n"
              << "    add_subdirectory(unit_test)\nendif()\n\n";

    // benchmark tests
    cmakelist << "#if called with -DBUILD_BENCHMARKS the benchmark target will be enabled\n";
    cmakelist << "if(BUILD_BENCHMARKS)\n"
              << "    find_package(benchmark REQUIRED)\n"
              << "    add_subdirectory(bench)\nendif()\n\n";

    // install
    cmakelist << "# install settings\n";
    cmakelist << "include(GNUInstallDirs)\n"
              << "install(TARGETS ${PROJECT_NAME}\n"
              << "    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}\n"
              << "    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}\n"
              << "    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}\n)\n";
    // install public headers if it's library repo
    if (RepoType::EXECUTABLE != repo_type)
    {
        cmakelist << "# install public headers\n";
        cmakelist << "set(PUBLIC_HEADER_DIR ${PROJECT_SOURCE_DIR}/" << repo_name << ")\n"
                  << "install(DIRECTORY ${PUBLIC_HEADER_DIR}\n"
                  << "    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}\n"
                  << "    FILES_MATCHING\n"
                  << "        PATTERN \"*.h\"\n"
                  << "        PATTERN \"*.hpp\"\n)\n";
    }
    // both library and executable repo support versioning
    cmakelist << "# auto versioning reads from header.h to get versionstring\n";
    cmakelist << fmt::format(R"=(file(STRINGS ${{PROJECT_SOURCE_DIR}}/{0}/header.h
    HEADER_CONTENTS REGEX "#define {1}_VERSION_.*")
string(REGEX MATCH "#define {1}_VERSION_MAJOR ([0-9]*)" _ ${{HEADER_CONTENTS}})
set({1}_VER_MAJOR ${{CMAKE_MATCH_1}})
string(REGEX MATCH "#define {1}_VERSION_MINOR ([0-9]*)" _ ${{HEADER_CONTENTS}})
set({1}_VER_MINOR ${{CMAKE_MATCH_1}})
string(REGEX MATCH "#define {1}_VERSION_PATCH ([0-9]*)" _ ${{HEADER_CONTENTS}})
set({1}_VER_PATCH ${{CMAKE_MATCH_1}})

message(STATUS "current {0} version: ${{{1}_VER_MAJOR}}.${{{1}_VER_MINOR}}.${{{1}_VER_PATCH}}")
set(CPACK_PACKAGE_VERSION_MAJOR ${{{1}_VER_MAJOR}})
set(CPACK_PACKAGE_VERSION_MINOR ${{{1}_VER_MINOR}})
set(CPACK_PACKAGE_VERSION_PATCH ${{{1}_VER_PATCH}})
)=",
        repo_name, upper_name);
    // cpack settings
    cmakelist << "# cpack settings, edit the following to pack up as you desire\n";
    cmakelist << "if(UNIX)\n"
              << "    set(CPACK_GENERATOR \"TGZ\")\n"
              << "else()\n"
              << "    set(CPACK_GENERATOR \"ZIP\")\n"
              << "endif()\n"
              << "set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/packages)\n"
              << "include(CPack)\n";
    INFO("root CMakeLists.txt written complete!");
}

void WriteUnitTests()
{
    std::ofstream unit_test("unit_test/CMakeLists.txt");
    unit_test << R"(set(LIBRARIES_FOR_TEST "") # put your library here
# an easy way to add unit test
# for now it supports only one src file as SOURCE
function(add_unit_test CASE_TARGET SOURCE)
    add_executable(${CASE_TARGET} ${SOURCE})
    target_link_libraries(${CASE_TARGET} PRIVATE )";
    unit_test << (RepoType::EXECUTABLE != repo_type ? repo_name : "") << R"(
    ${LIBRARIES_FOR_TEST} GTest::gtest GTest::gtest_main Threads::Threads)
    string(TOUPPER ${CASE_TARGET} CASE_TARGET_UPPERCASE)
    set(CASE_NAME "TEST_${CASE_TARGET_UPPERCASE}")
    add_test(NAME ${CASE_NAME} COMMAND ${CASE_TARGET})
endfunction()

file(GLOB UNIT_TEST_SRCS "*.cpp")

foreach(TEST_SRC ${UNIT_TEST_SRCS})
    get_filename_component(BASE_NAME ${TEST_SRC} NAME_WE)
    add_unit_test(${BASE_NAME} ${TEST_SRC})
endforeach()
)";

    std::ofstream unit_test_example("unit_test/example.cpp");
    unit_test_example << R"(#include <gtest/gtest.h>
TEST(EXAMPLE, example_case)
{
    int i = 1;
    EXPECT_EQ(i, 1);
    bool no = false;
    EXPECT_FALSE(no);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
)";

    INFO("unit_test/CMakeLists.txt written complete!");
}

void WriteBenchmark()
{
    std::ofstream bench("bench/CMakeLists.txt");
    bench << R"(set(LIBRARIES_FOR_BENCH "") # put your library here
function(add_benchmark BENCH_NAME SOURCE)
    add_executable(${BENCH_NAME} ${SOURCE})
    target_link_libraries(${BENCH_NAME} PRIVATE )";
    bench << (RepoType::EXECUTABLE != repo_type ? repo_name : "") << R"(
    ${LIBRARIES_FOR_BENCH} benchmark Threads::Threads)
endfunction()

add_benchmark(bench_example bench_example.cpp)
)";
    std::ofstream bench_example("bench/bench_example.cpp");
    bench_example << R"(#include <benchmark/benchmark.h>
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
)";
    INFO("benchmark example written complete!");
}

void WriteSrcAndHeader()
{

    // src file and header
    // library repo's library.cpp will be located in 'src' dir
    // executable repo's library.cpp will be located in 'repo_name' dir
    auto cppfile_name =
        fmt::format("{}/library.cpp", repo_type == RepoType::EXECUTABLE ? repo_name : "src");
    std::ofstream cppfile(cppfile_name);
    if (!cppfile)
    {
        ERROR("failed to open file: {}", cppfile_name);
    }
    cppfile << fmt::format(R"(#include "header.h"
const char *GetVersionString()
{{
#define XX(x) #x
#define STRINGIFY(x) XX(x)
    return STRINGIFY({0}_VERSION_MAJOR.{0}_VERSION_MINOR.{0}_VERSION_PATCH);
#undef XX
#undef STRINGIFY
}}
)",
        upper_name);

    INFO("{} written complete!", cppfile_name);

    if (RepoType::EXECUTABLE == repo_type)
    {
        std::ofstream helloworld(fmt::format("{}/main.cpp", repo_name));
        if (!helloworld)
        {
            ERROR("failed to open file: {}/main.cpp", repo_name);
        }
        helloworld << "#include <iostream>\n"
                   << "#include \"header.h\"\n\n"
                   << "int main(int argc, char** argv)\n{\n"
                   << fmt::format("    std::cout << \"hello {}! version:\"<< GetVersionString() << "
                                  "std::endl;\n}}\n",
                          repo_name);
    }

    // headers are always under the 'repo_name' dir
    auto header_name = fmt::format("{}/header.h", repo_name);
    std::ofstream header(header_name);
    if (!header)
    {
        ERROR("failed to open file: {}", header_name);
    }
    header << fmt::format(
        R"(#pragma once
#define {0}_VERSION_MAJOR 0
#define {0}_VERSION_MINOR 0
#define {0}_VERSION_PATCH 1
const char* GetVersionString();
)",
        upper_name);

    INFO("{} written complete!", header_name);
}