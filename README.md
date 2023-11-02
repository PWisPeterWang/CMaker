# CMaker

CMaker is a CMake project generator for C/C++ projects. It is designed to be a simple tool to generate a CMake project with a single command. It is also designed to be a simple tool to add libraries and submodules to the project.

The project is good for C/C++ beginners who want to start a new project with CMake. It demonstrates most of the commonly used functions in CMake and common practices in C/C++ software development. It is also good for C/C++ experts who want to start a new project with CMake quickly but tired of writing boilerplates CMake code.

The created repository consists of the following directories:

```bash
# cmaker new mylib
[INFO] creating repo for static library: mylib
[INFO] directory mylib created!
[INFO] entering directory mylib
[INFO] directory src created!
[INFO] directory mylib created!
[INFO] directory cmake_modules created!
[INFO] directory thirdparty created!
[INFO] directory bench created!
[INFO] directory unit_test created!
[INFO] root CMakeLists.txt written complete!
[INFO] unit_test/CMakeLists.txt written complete!
[INFO] benchmark example written complete!
Initialized empty Git repository in /home/wb/myrepo/cmaker/mylib/.git/
[INFO] leaving directory mylib

# tree mylib 
mylib
├── bench
│   ├── bench_example.cpp
│   └── CMakeLists.txt
├── CMakeLists.txt
├── README.md
├── LICENSE
├── .gitignore
├── .clang-format
├── cmake_modules
├── mylib
│   ├── mylib.h.in
│   └── mylib.cpp
├── thirdparty
└── unit_test
    ├── CMakeLists.txt
    └── example.cpp

5 directories, 9 files
```

1. a `bench` directory for benchmarking, with pre-configured CMakeLists.txt and a sample benchmark source file.
2. a `cmake_modules` directory for custom CMake modules. It is empty by default. You can add your own CMake modules here. When calling `cmaker add-library`, a `FindMyLib.cmake` file related to the library will be created under this directory, containing the common practice of how you can write a `FindXXX.cmake` module as an example (and it works).
3. a `mylib` directory for the library source files. When calling `cmaker new` the library name will be the same as the project name. The directoy constains a header file `header.h` with default `GetVersionString` function.
4. a `src` directory for the library source files. It contains a 'library.cpp' file with the implementation of the `GetVersionString` function.
5. a `thirdparty` directory for third-party libraries. It is empty by default. You can add your own third-party libraries here. When calling `cmaker add-module`, the module will be downloaded through `git submodule add` and added to this directory.
6. a `unit_test` directory for unit testing, with pre-configured CMakeLists.txt and a sample unit test source file.
7. a root `CMakeLists.txt` file for the project. It contains all the common CMake commands for a C/C++ project, such as, `add_library` or `add_executable` (when called with `cmaker new mylib --exe`), `install`, `enable_testing`, `include(CTest)`, `include(CPack)`, etc.

The pre-configured install commands in the root CMakeLists.txt will by default install the `mylib` directory to the `${CMAKE_INSTALL_INCLUDEDIR}` as public headers, and the headers located under `src` will be considered as private headers. You can change the install commands in the `CMakeLists.txt` file to achieve more customization.

Note that by default, the `CMakeLists.txt` file will install the `mylib` directory as a static library. If you want to install it as a shared library, you can add the `--shared` option when calling `cmaker new`. 

Also note that by default, the package location will be under the `build/package` directoy after calling `cpack` under the `build` directory, and it will be packed as tarball in Linux and Zip archive in Windows. You can change the package location and packaging behavior by modifying the CPack section in the root `CMakeLists.txt` file.

Another thing to note is that by default, the `header.h` contains your library's version string consisting from `MYLIB_VERSION_MAJOR`, `MYLIB_VERSION_MINOR` and `MYLIB_VERSION_PATCH` ('MYLIB' is the library name). The CPack section in the root `CMakeLists.txt` file will automatically set the package version to the version string. So that whenever you changes the version of you header, the package name will automatically change as well.

The pre-configured unit test CMakeLists.txt will scan the `unit_test` directory for any `.cpp` source files and add them as unit test executables. It provides a CMake function that simplifies the code needed to add a unit test.
You can add your own unit test source files to the `unit_test` directory and they will be automatically added to the unit test executables. For now, the `add_unit_test` function takes only 1 source file as argument. If you want to add more source files in a unit test executable, you would have to modify the `add_unit_test` function by yourself.

It also contains pre-configured `.gitignore`, `.clang-format` and `.clang-tidy` (stolen from fmtlib :p) files. You can modify them to suit your needs.

## Table of Contents

- [Dependencies](#dependencies)
- [Installation](#installation)
- [Usage](#usage)
- [Contributing](#contributing)
- [License](#license)

## Dependencies

- [Boost](https://www.boost.org/) [program_options](https://www.boost.org/doc/libs/1_83_0/doc/html/program_options.html)
- [fmtlib](https://fmt.dev)
- [CMake](https://cmake.org)
- [Ninja](https://github.com/ninja-build/ninja)
- [Git](https://git-scm.com/downloads)
- [GTest](https://github.com/google/googletest)
- [Benchmark](https://github.com/google/benchmark)
- A C++ compiler that supports C++11 like GCC 4.8.5 with Boost library, or C++17 like GCC 9.3.0 or above (requried for std::filesystem and initializer for-loop)

The project is tested on rhel 7.9 with GCC 4.8.5 (plus boost.filesystem) and ubuntu 20.04 with GCC 13.2.0.
If your compiler does not support C++17, Boost.filesystem is required.

## Installation

```bash
cmake -H. -Bbuild -G Ninja -DCMAKE_BUILD_TYPE=Release
ninja -C build install
```

## Usage

```bash
# creating a new repository
cmaker new mylib

# adding a new library
cd mylib
cmaker add-library mydep -I thirdparty/include/mydep.h -L thirdparty/lib

# adding a new submodule
cmaker add-submodule myrepo -U https://github.com/me/myrepo.git
```

## Contributing

Feel free to contribute to the project. You can open an issue or create a pull request.

## License

MIT License

Copyright (c) 2023 wbvalid@gmail.com

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.