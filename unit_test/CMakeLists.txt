add_executable(test_functions
    test_functions.cpp
    ${PROJECT_SOURCE_DIR}/cmaker/functions.cpp)

target_include_directories(test_functions PRIVATE
    ${PROJECT_SOURCE_DIR}/cmaker)

target_link_libraries(test_functions PRIVATE
    Boost::program_options
    fmt
    GTest::gtest GTest::gtest_main
    Threads::Threads)

add_test(NAME CMAKER_FUNCTIONS COMMAND test_functions)