#include "writer_funcs.h"
#include <fmt/format.h>

void WriteCMakeLists(WriterContext const &ctx)
{
    auto upper_name = ToUpper(ctx.repo_name);
    std::ofstream cmakelist("CMakeLists.txt");
    cmakelist << fmt::format(R"(cmake_minimum_required(VERSION 3.21)
set({0}_VERSION_MAJOR 0)
set({0}_VERSION_MINOR 0)
set({0}_VERSION_PATCH 1)
project({1} 
    LANGUAGES
        CXX C 
    VERSION
        ${{{0}_VERSION_MAJOR}}.${{{0}_VERSION_MINOR}}.${{{0}_VERSION_PATCH}})
)",
        upper_name, ctx.repo_name);

    cmakelist << "# edit the following settings as you desire\n"
              << fmt::format("set(CMAKE_CXX_STANDARD {})\n", ctx.cxx_std)
              << "set(CMAKE_CXX_STANDARD_REQUIRED ON)\n"
              << "set(CMAKE_CXX_STANDARD_EXTENSION OFF)\n"
              << "add_compile_options(-Wfatal-errors)\n\n";
    cmakelist << "# edit the following line to add your cmake modules\n"
              << "list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake_modules)\n";
    cmakelist << "# edit the following line to add your dependencies\n"
              << "find_package(Threads REQUIRED)\n\n";
    // for executable repo, scan the 'repo_name' dir to add all cpp files as it's SRCS
    cmakelist << "# Please note, CMake does not recommend GLOB to collect a list of source files "
                 "from your source tree.\n"
              << "# Any new files added to your source tree won't be noticed by CMake until you "
                 "rerun CMake manually.\n";
    if (RepoType::EXECUTABLE == ctx.repo_type)
    {
        cmakelist << fmt::format("file(GLOB_RECURSE EXECUTABLE_SRC \"{}/*.cpp\")\n", ctx.repo_name)
                  << "add_executable(${PROJECT_NAME} ${EXECUTABLE_SRC})\n";
        cmakelist << "target_include_directories(${PROJECT_NAME} PRIVATE\n    "
                  << fmt::format("${{PROJECT_SOURCE_DIR}}/{})\n\n", ctx.repo_name);
    }
    else // for library repo, scan the 'src' dir to add all cpp files as it's SRCS
    {
        cmakelist << fmt::format("file(GLOB_RECURSE LIBRARY_SRC \"{}/*.cpp\")\n", ctx.repo_name)
                  << "add_library(${PROJECT_NAME} "
                  << (ctx.repo_type == RepoType::SHARED ? "SHARED" : "STATIC")
                  << " \"${LIBRARY_SRC}\")\n";
        cmakelist << "# you may add more dependencies' header dir here\n";
        cmakelist << "target_include_directories(${PROJECT_NAME}\n"
                  << "    PUBLIC\n"
                  << "        $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/" << ctx.repo_name << ">\n"
                  << "        $<INSTALL_INTERFACE:" << ctx.repo_name << ">\n"
                  << "    PRIVATE\n"
                  << "        ${PROJECT_SOURCE_DIR}/src)\n\n";
    }

    // link example
    cmakelist << R"(# edit the following line to link your dependencies libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
        Threads::Threads)
)";

    // install
    cmakelist << fmt::format(R"(# install settings
include(GNUInstallDirs)
install(TARGETS ${{PROJECT_NAME}}
    EXPORT {}_EXPORT
    RUNTIME DESTINATION ${{CMAKE_INSTALL_BINDIR}}
    LIBRARY DESTINATION ${{CMAKE_INSTALL_LIBDIR}}
    ARCHIVE DESTINATION ${{CMAKE_INSTALL_LIBDIR}})
)",
        upper_name);

    // install public headers if it's library repo
    if (RepoType::EXECUTABLE != ctx.repo_type)
    {
        cmakelist << fmt::format(R"(# install public headers
set(PUBLIC_HEADER_DIR ${{PROJECT_SOURCE_DIR}}/{})
install(DIRECTORY ${{PUBLIC_HEADER_DIR}}
    DESTINATION ${{CMAKE_INSTALL_INCLUDEDIR}}
    FILES_MATCHING
        PATTERN "*.h"
        PATTERN "*.hpp")
)",
            ctx.repo_name);
    }

    // export cmake config
    cmakelist << fmt::format(R"(install(EXPORT {0}_EXPORT
    FILE {1}-config.cmake
    NAMESPACE {1}::
    DESTINATION cmake)
    include(CMakePackageConfigHelpers)
write_basic_package_version_file(
    "{1}-config-version.cmake"
    COMPATIBILITY SameMajorVersion)
install(FILES
    "${{CMAKE_CURRENT_BINARY_DIR}}/{1}-config-version.cmake"
    DESTINATION cmake)
)",
        upper_name, ctx.repo_name);

    // both library and executable repo support versioning
    cmakelist << fmt::format(
        R"(message(STATUS "current {0} version: ${{{1}_VERSION_MAJOR}}.${{{1}_VERSION_MINOR}}.${{{1}_VERSION_PATCH}}")
set(CPACK_PACKAGE_VERSION_MAJOR ${{{1}_VERSION_MAJOR}})
set(CPACK_PACKAGE_VERSION_MINOR ${{{1}_VERSION_MINOR}})
set(CPACK_PACKAGE_VERSION_PATCH ${{{1}_VERSION_PATCH}})
)",
        ctx.repo_name, upper_name);
    // cpack settings
    cmakelist << "# cpack settings, edit the following to pack up as you desire\n";
    cmakelist << "if(UNIX)\n"
              << "    set(CPACK_GENERATOR \"TGZ\")\n"
              << "else()\n"
              << "    set(CPACK_GENERATOR \"ZIP\")\n"
              << "endif()\n"
              << "set(CPACK_PACKAGE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/packages)\n"
              << "include(CPack)\n";
    LOGINFO("root CMakeLists.txt written complete!");
}

void WriteUnitTests()
{
    std::ofstream unit_test("unit_test/CMakeLists.txt");
    unit_test << R"(# an easy way to add unit test
# for now it supports only one src file for each TestCase
function(add_unit_test CASE_TARGET SOURCE)
    add_executable(${CASE_TARGET} ${SOURCE})
    target_link_libraries(${CASE_TARGET}
        PRIVATE 
            ${LIBRARIES_FOR_TEST} # put your library here
            GTest::gtest
            GTest::gtest_main
            Threads::Threads)
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

    LOGINFO("unit_test/CMakeLists.txt written complete!");
}

void WriteBenchmark()
{
    std::ofstream bench("bench/CMakeLists.txt");
    bench << R"(function(add_benchmark BENCH_NAME SOURCE)
    add_executable(${BENCH_NAME} ${SOURCE})
    target_link_libraries(${BENCH_NAME}
        PRIVATE
            benchmark
            Threads::Threads)
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
    LOGINFO("benchmark example written complete!");
}

void WriteSrcAndHeader(WriterContext const &ctx)
{
    std::string upper_name = ToUpper(ctx.repo_name);
    // src file and header
    // library repo's repo_name.cpp will be located in 'repo_name' dir
    auto cppfile_name = fmt::format("{0}/{0}.cpp", ctx.repo_name);
    std::ofstream cppfile(cppfile_name);
    if (!cppfile)
    {
        LOGERR("failed to open file: {}", cppfile_name);
    }
    cppfile << fmt::format(R"(#include "{1}.h"
const char *GetVersionString()
{{
#define XX(x) #x
#define STRINGIFY(x) XX(x)
    return STRINGIFY({0}_VERSION_MAJOR.{0}_VERSION_MINOR.{0}_VERSION_PATCH);
#undef XX
#undef STRINGIFY
}}
)",
        upper_name, ctx.repo_name);

    LOGINFO("{} written complete!", cppfile_name);

    if (RepoType::EXECUTABLE == ctx.repo_type)
    {
        std::ofstream helloworld(fmt::format("{}/main.cpp", ctx.repo_name));
        if (!helloworld)
        {
            LOGERR("failed to open file: {}/main.cpp", ctx.repo_name);
        }
        helloworld << fmt::format(R"(#include <iostream>
#include "{0}.h"
int main(int argc, char** argv)
{{
    std::cout << "hello {0}! version:" << GetVersionString() << std::endl;
}}
)",
            ctx.repo_name);
    }

    // headers are always under the 'repo_name' dir
    auto header_name = fmt::format("{0}/{0}.h", ctx.repo_name);
    std::ofstream header(header_name);
    if (!header)
    {
        LOGERR("failed to open file: {}", header_name);
    }
    header << fmt::format(
        R"(#pragma once
#define {0}_VERSION_MAJOR 0
#define {0}_VERSION_MINOR 0
#define {0}_VERSION_PATCH 1
const char* GetVersionString();)",
        upper_name);
    header << R"(
// if you want auto versioning feature, you can rename this file as {0}.h.in
// replace the version number to "@{0}_VERSION_MAJOR@" , etc.
// then add `configure_file({0}.h.in {0}.h @ONLY)` in your CMakeLists.txt
// CMake will replace the "@{0}_VERSION_MAJOR@" string with the actual version number
// and generate the {0}.h file in ${CMAKE_BINARY_DIR}
)";

    LOGINFO("{} written complete!", header_name);
}

void WriteGitignore()
{
    std::ofstream gitignore(".gitignore");
    gitignore << R"(# compile outputs
*.o
*.obj
*.ko
*.so
*.a
*.exe
*.out
*.app

# compile intermediate outputs
*.d
*.depend
*.gcno
*.gcda
*.stackdump

# Visual Studion Code dirs
.vscode/
.vs/

# Python cache
__pycache__/
*.pyc

# build related
autom4te.cache/
build/
build64_debug/
build64_release/
blade-bin/

# binary output
bin/

# os related
.cache/
.DS_Store
ehthumbs.db
Thumbs.db)";
}

void WriteClangformat()
{
    std::ofstream clangformat(".clang-format");
    clangformat << R"(---
Language:        Cpp
# BasedOnStyle:  LLVM
AccessModifierOffset: -4
AlignAfterOpenBracket: DontAlign
AlignConsecutiveAssignments: false
AlignConsecutiveDeclarations: false
AlignEscapedNewlines: Right
AlignOperands:   true
AlignTrailingComments: true
AllowAllParametersOfDeclarationOnNextLine: true
AllowShortBlocksOnASingleLine: true
AllowShortCaseLabelsOnASingleLine: false
AllowShortFunctionsOnASingleLine: Empty
AllowShortIfStatementsOnASingleLine: false
AllowShortLoopsOnASingleLine: false
AlwaysBreakAfterDefinitionReturnType: None
AlwaysBreakAfterReturnType: None
AlwaysBreakBeforeMultilineStrings: false
AlwaysBreakTemplateDeclarations: true
BinPackArguments: true
BinPackParameters: true
BraceWrapping:     
  AfterClass:      true
  AfterControlStatement: true
  AfterEnum:       true
  AfterFunction:   true
  AfterNamespace:  false
  AfterObjCDeclaration: true
  AfterStruct:     true
  AfterUnion:      true
  BeforeCatch:     true
  BeforeElse:      true
  IndentBraces:    false
  SplitEmptyFunction: false
  SplitEmptyRecord: false
  SplitEmptyNamespace: false
BreakBeforeBinaryOperators: None
BreakBeforeBraces: Custom
BreakBeforeInheritanceComma: false
BreakBeforeTernaryOperators: true
BreakConstructorInitializersBeforeComma: true
BreakConstructorInitializers: BeforeColon
BreakAfterJavaFieldAnnotations: false
BreakStringLiterals: true
ColumnLimit:     100
CommentPragmas:  '^ IWYU pragma:'
CompactNamespaces:  false
ConstructorInitializerAllOnOneLineOrOnePerLine: false
ConstructorInitializerIndentWidth: 4
ContinuationIndentWidth: 4
Cpp11BracedListStyle: true
DerivePointerAlignment: false
DisableFormat:   false
ExperimentalAutoDetectBinPacking: false
FixNamespaceComments: true
ForEachMacros:   
  - foreach
  - Q_FOREACH
  - BOOST_FOREACH
IncludeCategories: 
  - Regex:           '^"(llvm|llvm-c|clang|clang-c)/'
    Priority:        2
  - Regex:           '^(<|"(gtest|gmock|isl|json)/)'
    Priority:        3
  - Regex:           '.*'
    Priority:        1
IncludeIsMainRegex: '(Test)?$'
IndentCaseLabels: false
IndentWidth:     4
IndentWrappedFunctionNames: false
JavaScriptQuotes: Leave
JavaScriptWrapImports: true
KeepEmptyLinesAtTheStartOfBlocks: true
MacroBlockBegin: ''
MacroBlockEnd:   ''
MaxEmptyLinesToKeep: 1
NamespaceIndentation: None
ObjCBlockIndentWidth: 2
ObjCSpaceAfterProperty: false
ObjCSpaceBeforeProtocolList: true
PenaltyBreakAssignment: 2
PenaltyBreakBeforeFirstCallParameter: 19
PenaltyBreakComment: 300
PenaltyBreakFirstLessLess: 120
PenaltyBreakString: 1000
PenaltyExcessCharacter: 1000000
PenaltyReturnTypeOnItsOwnLine: 60
PointerAlignment: Right
ReflowComments:  true
SortIncludes:    false
SortUsingDeclarations: true
SpaceAfterCStyleCast: false
SpaceAfterTemplateKeyword: false
SpaceBeforeAssignmentOperators: true
SpaceBeforeParens: ControlStatements
SpaceInEmptyParentheses: false
SpacesBeforeTrailingComments: 1
SpacesInAngles:  false
SpacesInContainerLiterals: true
SpacesInCStyleCastParentheses: false
SpacesInParentheses: false
SpacesInSquareBrackets: false
Standard:        Cpp11
TabWidth:        8
UseTab:          Never
IndentPPDirectives: AfterHash
...
)";
}

const std::string license_MIT = R"(MIT License

Copyright (c) <year> <example@email.com>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the “Software”), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
copies of the Software, and to permit persons to whom the Software is 
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.)";

const std::string license_LGPLV3 = R"(                   GNU LESSER GENERAL PUBLIC LICENSE
                       Version 3, 29 June 2007

 Copyright (C) 2007 Free Software Foundation, Inc. <https://fsf.org/>
 Everyone is permitted to copy and distribute verbatim copies
 of this license document, but changing it is not allowed.


  This version of the GNU Lesser General Public License incorporates
the terms and conditions of version 3 of the GNU General Public
License, supplemented by the additional permissions listed below.

  0. Additional Definitions.

  As used herein, "this License" refers to version 3 of the GNU Lesser
General Public License, and the "GNU GPL" refers to version 3 of the GNU
General Public License.

  "The Library" refers to a covered work governed by this License,
other than an Application or a Combined Work as defined below.

  An "Application" is any work that makes use of an interface provided
by the Library, but which is not otherwise based on the Library.
Defining a subclass of a class defined by the Library is deemed a mode
of using an interface provided by the Library.

  A "Combined Work" is a work produced by combining or linking an
Application with the Library.  The particular version of the Library
with which the Combined Work was made is also called the "Linked
Version".

  The "Minimal Corresponding Source" for a Combined Work means the
Corresponding Source for the Combined Work, excluding any source code
for portions of the Combined Work that, considered in isolation, are
based on the Application, and not on the Linked Version.

  The "Corresponding Application Code" for a Combined Work means the
object code and/or source code for the Application, including any data
and utility programs needed for reproducing the Combined Work from the
Application, but excluding the System Libraries of the Combined Work.

  1. Exception to Section 3 of the GNU GPL.

  You may convey a covered work under sections 3 and 4 of this License
without being bound by section 3 of the GNU GPL.

  2. Conveying Modified Versions.

  If you modify a copy of the Library, and, in your modifications, a
facility refers to a function or data to be supplied by an Application
that uses the facility (other than as an argument passed when the
facility is invoked), then you may convey a copy of the modified
version:

   a) under this License, provided that you make a good faith effort to
   ensure that, in the event an Application does not supply the
   function or data, the facility still operates, and performs
   whatever part of its purpose remains meaningful, or

   b) under the GNU GPL, with none of the additional permissions of
   this License applicable to that copy.

  3. Object Code Incorporating Material from Library Header Files.

  The object code form of an Application may incorporate material from
a header file that is part of the Library.  You may convey such object
code under terms of your choice, provided that, if the incorporated
material is not limited to numerical parameters, data structure
layouts and accessors, or small macros, inline functions and templates
(ten or fewer lines in length), you do both of the following:

   a) Give prominent notice with each copy of the object code that the
   Library is used in it and that the Library and its use are
   covered by this License.

   b) Accompany the object code with a copy of the GNU GPL and this license
   document.

  4. Combined Works.

  You may convey a Combined Work under terms of your choice that,
taken together, effectively do not restrict modification of the
portions of the Library contained in the Combined Work and reverse
engineering for debugging such modifications, if you also do each of
the following:

   a) Give prominent notice with each copy of the Combined Work that
   the Library is used in it and that the Library and its use are
   covered by this License.

   b) Accompany the Combined Work with a copy of the GNU GPL and this license
   document.

   c) For a Combined Work that displays copyright notices during
   execution, include the copyright notice for the Library among
   these notices, as well as a reference directing the user to the
   copies of the GNU GPL and this license document.

   d) Do one of the following:

       0) Convey the Minimal Corresponding Source under the terms of this
       License, and the Corresponding Application Code in a form
       suitable for, and under terms that permit, the user to
       recombine or relink the Application with a modified version of
       the Linked Version to produce a modified Combined Work, in the
       manner specified by section 6 of the GNU GPL for conveying
       Corresponding Source.

       1) Use a suitable shared library mechanism for linking with the
       Library.  A suitable mechanism is one that (a) uses at run time
       a copy of the Library already present on the user's computer
       system, and (b) will operate properly with a modified version
       of the Library that is interface-compatible with the Linked
       Version.

   e) Provide Installation Information, but only if you would otherwise
   be required to provide such information under section 6 of the
   GNU GPL, and only to the extent that such information is
   necessary to install and execute a modified version of the
   Combined Work produced by recombining or relinking the
   Application with a modified version of the Linked Version. (If
   you use option 4d0, the Installation Information must accompany
   the Minimal Corresponding Source and Corresponding Application
   Code. If you use option 4d1, you must provide the Installation
   Information in the manner specified by section 6 of the GNU GPL
   for conveying Corresponding Source.)

  5. Combined Libraries.

  You may place library facilities that are a work based on the
Library side by side in a single library together with other library
facilities that are not Applications and are not covered by this
License, and convey such a combined library under terms of your
choice, if you do both of the following:

   a) Accompany the combined library with a copy of the same work based
   on the Library, uncombined with any other library facilities,
   conveyed under the terms of this License.

   b) Give prominent notice with the combined library that part of it
   is a work based on the Library, and explaining where to find the
   accompanying uncombined form of the same work.

  6. Revised Versions of the GNU Lesser General Public License.

  The Free Software Foundation may publish revised and/or new versions
of the GNU Lesser General Public License from time to time. Such new
versions will be similar in spirit to the present version, but may
differ in detail to address new problems or concerns.

  Each version is given a distinguishing version number. If the
Library as you received it specifies that a certain numbered version
of the GNU Lesser General Public License "or any later version"
applies to it, you have the option of following the terms and
conditions either of that published version or of any later version
published by the Free Software Foundation. If the Library as you
received it does not specify a version number of the GNU Lesser
General Public License, you may choose any version of the GNU Lesser
General Public License ever published by the Free Software Foundation.

  If the Library as you received it specifies that a proxy can decide
whether future versions of the GNU Lesser General Public License shall
apply, that proxy's public statement of acceptance of any version is
permanent authorization for you to choose that version for the
Library.)";

const std::string license_APACHE = R"(                                 Apache License
                           Version 2.0, January 2004
                        http://www.apache.org/licenses/

   TERMS AND CONDITIONS FOR USE, REPRODUCTION, AND DISTRIBUTION

   1. Definitions.

      "License" shall mean the terms and conditions for use, reproduction,
      and distribution as defined by Sections 1 through 9 of this document.

      "Licensor" shall mean the copyright owner or entity authorized by
      the copyright owner that is granting the License.

      "Legal Entity" shall mean the union of the acting entity and all
      other entities that control, are controlled by, or are under common
      control with that entity. For the purposes of this definition,
      "control" means (i) the power, direct or indirect, to cause the
      direction or management of such entity, whether by contract or
      otherwise, or (ii) ownership of fifty percent (50%) or more of the
      outstanding shares, or (iii) beneficial ownership of such entity.

      "You" (or "Your") shall mean an individual or Legal Entity
      exercising permissions granted by this License.

      "Source" form shall mean the preferred form for making modifications,
      including but not limited to software source code, documentation
      source, and configuration files.

      "Object" form shall mean any form resulting from mechanical
      transformation or translation of a Source form, including but
      not limited to compiled object code, generated documentation,
      and conversions to other media types.

      "Work" shall mean the work of authorship, whether in Source or
      Object form, made available under the License, as indicated by a
      copyright notice that is included in or attached to the work
      (an example is provided in the Appendix below).

      "Derivative Works" shall mean any work, whether in Source or Object
      form, that is based on (or derived from) the Work and for which the
      editorial revisions, annotations, elaborations, or other modifications
      represent, as a whole, an original work of authorship. For the purposes
      of this License, Derivative Works shall not include works that remain
      separable from, or merely link (or bind by name) to the interfaces of,
      the Work and Derivative Works thereof.

      "Contribution" shall mean any work of authorship, including
      the original version of the Work and any modifications or additions
      to that Work or Derivative Works thereof, that is intentionally
      submitted to Licensor for inclusion in the Work by the copyright owner
      or by an individual or Legal Entity authorized to submit on behalf of
      the copyright owner. For the purposes of this definition, "submitted"
      means any form of electronic, verbal, or written communication sent
      to the Licensor or its representatives, including but not limited to
      communication on electronic mailing lists, source code control systems,
      and issue tracking systems that are managed by, or on behalf of, the
      Licensor for the purpose of discussing and improving the Work, but
      excluding communication that is conspicuously marked or otherwise
      designated in writing by the copyright owner as "Not a Contribution."

      "Contributor" shall mean Licensor and any individual or Legal Entity
      on behalf of whom a Contribution has been received by Licensor and
      subsequently incorporated within the Work.

   2. Grant of Copyright License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      copyright license to reproduce, prepare Derivative Works of,
      publicly display, publicly perform, sublicense, and distribute the
      Work and such Derivative Works in Source or Object form.

   3. Grant of Patent License. Subject to the terms and conditions of
      this License, each Contributor hereby grants to You a perpetual,
      worldwide, non-exclusive, no-charge, royalty-free, irrevocable
      (except as stated in this section) patent license to make, have made,
      use, offer to sell, sell, import, and otherwise transfer the Work,
      where such license applies only to those patent claims licensable
      by such Contributor that are necessarily infringed by their
      Contribution(s) alone or by combination of their Contribution(s)
      with the Work to which such Contribution(s) was submitted. If You
      institute patent litigation against any entity (including a
      cross-claim or counterclaim in a lawsuit) alleging that the Work
      or a Contribution incorporated within the Work constitutes direct
      or contributory patent infringement, then any patent licenses
      granted to You under this License for that Work shall terminate
      as of the date such litigation is filed.

   4. Redistribution. You may reproduce and distribute copies of the
      Work or Derivative Works thereof in any medium, with or without
      modifications, and in Source or Object form, provided that You
      meet the following conditions:

      (a) You must give any other recipients of the Work or
          Derivative Works a copy of this License; and

      (b) You must cause any modified files to carry prominent notices
          stating that You changed the files; and

      (c) You must retain, in the Source form of any Derivative Works
          that You distribute, all copyright, patent, trademark, and
          attribution notices from the Source form of the Work,
          excluding those notices that do not pertain to any part of
          the Derivative Works; and

      (d) If the Work includes a "NOTICE" text file as part of its
          distribution, then any Derivative Works that You distribute must
          include a readable copy of the attribution notices contained
          within such NOTICE file, excluding those notices that do not
          pertain to any part of the Derivative Works, in at least one
          of the following places: within a NOTICE text file distributed
          as part of the Derivative Works; within the Source form or
          documentation, if provided along with the Derivative Works; or,
          within a display generated by the Derivative Works, if and
          wherever such third-party notices normally appear. The contents
          of the NOTICE file are for informational purposes only and
          do not modify the License. You may add Your own attribution
          notices within Derivative Works that You distribute, alongside
          or as an addendum to the NOTICE text from the Work, provided
          that such additional attribution notices cannot be construed
          as modifying the License.

      You may add Your own copyright statement to Your modifications and
      may provide additional or different license terms and conditions
      for use, reproduction, or distribution of Your modifications, or
      for any such Derivative Works as a whole, provided Your use,
      reproduction, and distribution of the Work otherwise complies with
      the conditions stated in this License.

   5. Submission of Contributions. Unless You explicitly state otherwise,
      any Contribution intentionally submitted for inclusion in the Work
      by You to the Licensor shall be under the terms and conditions of
      this License, without any additional terms or conditions.
      Notwithstanding the above, nothing herein shall supersede or modify
      the terms of any separate license agreement you may have executed
      with Licensor regarding such Contributions.

   6. Trademarks. This License does not grant permission to use the trade
      names, trademarks, service marks, or product names of the Licensor,
      except as required for reasonable and customary use in describing the
      origin of the Work and reproducing the content of the NOTICE file.

   7. Disclaimer of Warranty. Unless required by applicable law or
      agreed to in writing, Licensor provides the Work (and each
      Contributor provides its Contributions) on an "AS IS" BASIS,
      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or
      implied, including, without limitation, any warranties or conditions
      of TITLE, NON-INFRINGEMENT, MERCHANTABILITY, or FITNESS FOR A
      PARTICULAR PURPOSE. You are solely responsible for determining the
      appropriateness of using or redistributing the Work and assume any
      risks associated with Your exercise of permissions under this License.

   8. Limitation of Liability. In no event and under no legal theory,
      whether in tort (including negligence), contract, or otherwise,
      unless required by applicable law (such as deliberate and grossly
      negligent acts) or agreed to in writing, shall any Contributor be
      liable to You for damages, including any direct, indirect, special,
      incidental, or consequential damages of any character arising as a
      result of this License or out of the use or inability to use the
      Work (including but not limited to damages for loss of goodwill,
      work stoppage, computer failure or malfunction, or any and all
      other commercial damages or losses), even if such Contributor
      has been advised of the possibility of such damages.

   9. Accepting Warranty or Additional Liability. While redistributing
      the Work or Derivative Works thereof, You may choose to offer,
      and charge a fee for, acceptance of support, warranty, indemnity,
      or other liability obligations and/or rights consistent with this
      License. However, in accepting such obligations, You may act only
      on Your own behalf and on Your sole responsibility, not on behalf
      of any other Contributor, and only if You agree to indemnify,
      defend, and hold each Contributor harmless for any liability
      incurred by, or claims asserted against, such Contributor by reason
      of your accepting any such warranty or additional liability.

   END OF TERMS AND CONDITIONS

   APPENDIX: How to apply the Apache License to your work.

      To apply the Apache License to your work, attach the following
      boilerplate notice, with the fields enclosed by brackets "[]"
      replaced with your own identifying information. (Don't include
      the brackets!)  The text should be enclosed in the appropriate
      comment syntax for the file format. We also recommend that a
      file or class name and description of purpose be included on the
      same "printed page" as the copyright notice for easier
      identification within third-party archives.

   Copyright [yyyy] [name of copyright owner]

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.)";

const std::string license_BOOST = R"(Boost Software License - Version 1.0 - August 17th, 2003

Permission is hereby granted, free of charge, to any person or organization
obtaining a copy of the software and accompanying documentation covered by
this license (the "Software") to use, reproduce, display, distribute,
execute, and transmit the Software, and to prepare derivative works of the
Software, and to permit third-parties to whom the Software is furnished to
do so, all subject to the following:

The copyright notices in the Software and this entire statement, including
the above license grant, this restriction and the following disclaimer,
must be included in all copies of the Software, in whole or in part, and
all derivative works of the Software, unless such copies or derivative
works are solely in the form of machine-executable object code generated by
a source language processor.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
)";

void WriteLicense(WriterContext const &ctx)
{
    std::ofstream license("LICENSE");
    if (ctx.license.empty())
    {
        license << license_MIT;
    }
    else if (ToUpper(ctx.license) == "MIT")
    {
        license << license_MIT;
    }
    else if (ToUpper(ctx.license) == "LGPLV3")
    {
        license << license_LGPLV3;
    }
    else if (ToUpper(ctx.license) == "APACHE")
    {
        license << license_APACHE;
    }
    else if (ToUpper(ctx.license) == "BOOST")
    {
        license << license_BOOST;
    }
    else
    {
        LOGWARN("unsupported license type: {}, use MIT license instead", ctx.license);
        license << license_MIT;
    }
}

void WriteReadme(WriterContext const &ctx)
{
    std::ofstream readme("README.md");
    readme << fmt::format("# {}\n", ctx.repo_name);
    if (RepoType::EXECUTABLE == ctx.repo_type)
    {
        readme << "## Build\n"
               << "```bash\n"
               << "mkdir build && cd build\n"
               << "cmake ..\n"
               << "make\n"
               << "```\n";
    }
    else
    {
        readme << "## Build\n"
               << "```bash\n"
               << "mkdir build && cd build\n"
               << "cmake .. -DBUILD_TESTS=ON -DBUILD_BENCHMARKS=ON\n"
               << "make\n"
               << "make test\n"
               << "make bench\n"
               << "```\n";
    }
    readme << "## License\n"
           << fmt::format("This project is licensed under the {} License - see the "
                          "[LICENSE](LICENSE) file for details.\n",
                  ctx.license);
}

void WriteClangTidy()
{
    std::ofstream clangtidy(".clang-tidy");
    clangtidy << R"(Checks: 'cppcoreguidelines-*,
performance-*,
modernize-*,
google-*,
misc-*
cert-*,
readability-*,
clang-analyzer-*,
-performance-unnecessary-value-param,
-modernize-use-trailing-return-type,
-google-runtime-references,
-misc-non-private-member-variables-in-classes,
-readability-braces-around-statements,
-google-readability-braces-around-statements,
-cppcoreguidelines-avoid-magic-numbers,
-readability-magic-numbers,
-readability-magic-numbers,
-cppcoreguidelines-pro-type-vararg,
-cppcoreguidelines-pro-bounds-pointer-arithmetic,
-cppcoreguidelines-avoid-c-arrays,
-modernize-avoid-c-arrays,
-cppcoreguidelines-pro-bounds-array-to-pointer-decay,
-readability-named-parameter,
-cert-env33-c
'

WarningsAsErrors: ''
HeaderFilterRegex: ''
AnalyzeTemporaryDtors: false
FormatStyle:     none

CheckOptions:    
  - key:             google-readability-braces-around-statements.ShortStatementLines
    value:           '1'
  - key:             google-readability-function-size.StatementThreshold
    value:           '800'
  - key:             google-readability-namespace-comments.ShortNamespaceLines
    value:           '10'
  - key:             google-readability-namespace-comments.SpacesBeforeComments
    value:           '2'
  - key:             modernize-loop-convert.MaxCopySize
    value:           '16'
  - key:             modernize-loop-convert.MinConfidence
    value:           reasonable
  - key:             modernize-loop-convert.NamingStyle
    value:           CamelCase
  - key:             modernize-pass-by-value.IncludeStyle
    value:           llvm
  - key:             modernize-replace-auto-ptr.IncludeStyle
    value:           llvm
  - key:             modernize-use-nullptr.NullMacros
    value:           'NULL'
)";
}
