cmake_minimum_required(VERSION 3.16)

project(cmaker LANGUAGES CXX C)

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-std=c++17" CXX17_SUPPORTED)
if(NOT CXX17_SUPPORTED)
    message(WARNING "C++17 is not supported by the compiler")
    set(CMAKE_CXX_STANDARD 11)
    add_compile_definitions(USE_BOOST_FILESYSTEM)
else()
    message(STATUS "C++17 is supported by the compiler")
    set(CMAKE_CXX_STANDARD 17)

    # Generate the test file
    set(TEST_FILE ${CMAKE_BINARY_DIR}/test_cxx17_filesystem.cpp)
    file(WRITE ${TEST_FILE}
        "#include <filesystem>\n"
        "int main() { std::filesystem::path p; return 0; }\n"
    )

    # Check if the compiler supports C++17's <filesystem>
    try_compile(CXX17_FILESYSTEM_SUPPORTED
        ${CMAKE_BINARY_DIR}/temp
        ${TEST_FILE}
        COMPILE_DEFINITIONS "-std=c++17"
    )

    # Remove the test file
    file(REMOVE ${TEST_FILE})

    if(CXX17_FILESYSTEM_SUPPORTED)
        message(STATUS "C++17's <filesystem> is supported by the compiler")
    else()
        message(WARNING "C++17's <filesystem> is not supported by the compiler")
        add_compile_definitions(USE_BOOST_FILESYSTEM)
    endif()
endif()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_EXTENSION OFF)

add_compile_options(-Wfatal-errors)

# Enable extra find_package modules
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

# Find the library
if (NOT CXX17_SUPPORTED)
    find_package(Boost REQUIRED COMPONENTS program_options filesystem)
else()
    find_package(Boost REQUIRED COMPONENTS program_options)
endif()

find_package(fmt REQUIRED)
find_package(Threads REQUIRED)

option(BUILD_TESTS "build unit tests" ON)
if (BUILD_TESTS)
    find_package(GTest REQUIRED)
    enable_testing()
    add_subdirectory(unit_test)
endif()

# Set up the executable target
add_executable(cmaker
    main.cpp
    cmaker/create_new_project.cpp
    cmaker/add_thirdparty_library.cpp
    cmaker/add_submodule.cpp
    cmaker/functions.cpp
    cmaker/writer_funcs.cpp
    )

# Set include dirs
target_include_directories(cmaker
    PRIVATE
        ${PROJECT_SOURCE_DIR}/cmaker
)

target_compile_definitions(cmaker PRIVATE "FMT_HEADER_ONLY")

# Link to thirdparty libraries
if (NOT CXX17_SUPPORTED OR NOT CXX17_FILESYSTEM_SUPPORTED)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            fmt::fmt
            Boost::program_options
            Boost::filesystem
            Threads::Threads)
else()
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
            fmt::fmt
            Boost::program_options
            Threads::Threads)
endif()

include(GNUInstallDirs)
# Set up the installation 
install(TARGETS ${PROJECT_NAME} 
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Set up CPack
if(UNIX)
    set(CPACK_GENERATOR "TGZ")
else()
    set(CPACK_GENERATOR "ZIP")
endif()

file(STRINGS ${PROJECT_SOURCE_DIR}/cmaker/functions.h
    HEADER_CONTENTS REGEX "#define CMAKER_VERSION_.*")
string(REGEX MATCH "#define CMAKER_VERSION_MAJOR ([0-9]*)" _ ${HEADER_CONTENTS})
set(CMAKER_VER_MAJOR ${CMAKE_MATCH_1})
string(REGEX MATCH "#define CMAKER_VERSION_MINOR ([0-9]*)" _ ${HEADER_CONTENTS})
set(CMAKER_VER_MINOR ${CMAKE_MATCH_1})
string(REGEX MATCH "#define CMAKER_VERSION_PATCH ([0-9]*)" _ ${HEADER_CONTENTS})
set(CMAKER_VER_PATCH ${CMAKE_MATCH_1})

message(STATUS "current CMaker version: ${CMAKER_VER_MAJOR}.${CMAKER_VER_MINOR}.${CMAKER_VER_PATCH}")
set(CPACK_PACKAGE_VERSION_MAJOR ${CMAKER_VER_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${CMAKER_VER_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${CMAKER_VER_PATCH})

set(CPACK_PACKAGE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/packages")
include(CPack)
