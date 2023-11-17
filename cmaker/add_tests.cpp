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
    WriterContext ctx;
    CreateDirIfNotExist("unit_test");
    WriteUnitTests(ctx);
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
