add_executable(test_functions
    test_functions.cpp
    ${PROJECT_SOURCE_DIR}/cmaker/functions.cpp)

target_include_directories(test_functions PRIVATE
    ${PROJECT_SOURCE_DIR}/cmaker)

    if(NOT CXX17_SUPPORTED OR NOT CXX17_FILESYSTEM_SUPPORTED)
    target_link_libraries(test_functions PRIVATE
        Boost::program_options
        Boost::filesystem
        fmt::fmt
        GTest::gtest GTest::gtest_main
        Threads::Threads)
else()
    target_link_libraries(test_functions PRIVATE
        Boost::program_options
        fmt::fmt
        GTest::gtest GTest::gtest_main
        Threads::Threads)
endif()

add_test(NAME CMAKER_FUNCTIONS COMMAND test_functions)