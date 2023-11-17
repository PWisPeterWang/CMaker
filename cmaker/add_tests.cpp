#include "functions.h"
#include "writer_funcs.h"

extern po::options_description creator;
extern po::variables_map vm;

void AddTests()
{
    if (!IsProjectRoot())
    {
        LOGERR("must be under the project root directory!");
    }
    CreateDirIfNotExist("unit_test");
    WriteUnitTests();
    LOGINFO("Unittests template generated!");
    LOGINFO("Remember to add the following lines into your CMakeLists.txt to take effect:");
    fmt::print(R"(
option(BUILD_TESTS "build unit tests" OFF)
if(BUILD_TESTS)
    find_package(GTest REQUIRED)
    enable_testing()
    add_subdirectory(unit_test)
endif()

)");
}

void AddTemplate()
{
    if (!IsProjectRoot())
    {
        LOGERR("Opration 'template' MUST be called under the project root!");
        return;
    }
    if (vm.count("name") == 0)
    {
        LOGERR("Missing operend for 'template' operation. Available templates:");
        fmt::print("    [bench, tests]\n");
        return;
    }
    auto name = vm["name"].as<std::string>();
    if (name == "help")
    {
        LOGINFO("Available templates:");
        fmt::print("    [bench, tests]\n");
        return;
    }
    if (name == "bench")
    {
        AddBench();
    }
    else if (name == "tests")
    {
        AddTests();
    }
    else
    {
        LOGERR("Template not supported: {}", name);
    }
}