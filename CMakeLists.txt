cmake_minimum_required(VERSION 3.16)

project(cmaker LANGUAGES CXX C)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_STANDARD_EXTENSION OFF)
add_compile_options(-Wfatal-errors)

# Enable extra find_package modules
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)

# Find the library
find_package(Boost REQUIRED COMPONENTS program_options system filesystem)
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
    )

# Set include dirs
target_include_directories(cmaker
    PRIVATE
        ${PROJECT_SOURCE_DIR}/cmaker
)

target_compile_definitions(cmaker PRIVATE "FMT_HEADER_ONLY")

# Link to thirdparty libraries
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::program_options)

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

message(STATUS "current myexe version: ${CMAKER_VER_MAJOR}.${CMAKER_VER_MINOR}.${CMAKER_VER_PATCH}")
set(CPACK_PACKAGE_VERSION_MAJOR ${CMAKER_VER_MAJOR})
set(CPACK_PACKAGE_VERSION_MINOR ${CMAKER_VER_MINOR})
set(CPACK_PACKAGE_VERSION_PATCH ${CMAKER_VER_PATCH})

set(CPACK_PACKAGE_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}/packages")
include(CPack)
