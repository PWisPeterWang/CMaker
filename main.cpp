#include "functions.h"

po::options_description all(fmt::format(
    GREEN("\"CMaker\"") " ({}) is A CMakeLists.txt generater helper.\n"
                        "The program will help you create a repo based on CMake build system "
                        "with ease.\n"
                        "You don't have to remember all those obscure syntax and commands.\n"
                        "Start coding in just one command. All the common features of CMake "
                        "will be added in\n"
                        "the well-elaborated CMakeLists.txt template, along with GTest and "
                        "benchmark examples.\n"
                        "Here are the allowed options",
    GetVersionString()));
po::options_description general(BLUE("general options"));
po::options_description creator(BLUE("create a new repo"));
po::options_description adder_library(BLUE("add thirdparty library"));
po::options_description adder_module(BLUE("add submodule"));
po::positional_options_description pos_desc;
po::variables_map vm;

int main(int argc, const char *argv[])
{
    // clang-format off
    general.add_options()("help", "print the help message")
        ("operation", po::value<std::string>()->required(), 
            "1st positional argument. supported operations: new, add-library, add-submodule")
        ("name", po::value<std::string>()->required(), 
            "2nd positional argument. operand for the operation")
        ("version,v", "print the version string")
        ("std", po::value<std::string>()->default_value("11"),
            "c++ standard version, default value is: 11")
        ("license", po::value<std::string>()->default_value("MIT"), 
            "license type, default value is: MIT, supported values: MIT, Apache, LGPLv3, Boost")
        ;
    
    creator.add_options()("static", "create a repo template based on library project (static library) [default]")
        ("shared", "create a repo template based on library project (shared library)")
        ("exe", "create a repo template based on executable project")
        ;
    adder_library.add_options()
        ("path,L", po::value<std::string>(), "path to find the library, such as: thirdparty/mydep/lib")
        ("inc,I", po::value<std::string>(), "path to find the header file, such as: thirdparty/mydep/dep.h")
        ;
    adder_module.add_options()
        ("url,U", po::value<std::string>(), "url to submodule, such as: https://github.com/me/myrepo.git")
        ("dir,D", po::value<std::string>()->default_value("thirdparty"), "directory where the submodule shall be initialized, default value is: thirdparty")
        ;
    // clang-format on
    pos_desc.add("operation", 1).add("name", 1);

    all.add(general).add(creator).add(adder_library).add(adder_module);

    if (!IsExecutableInPath("git"))
    {
        LOGERR("cmaker requires " GREEN("git") " to create repo!");
    }

    if (!IsExecutableInPath("cmake"))
    {
        LOGERR("cmaker requires " GREEN("cmake") " on your system!");
    }

    try
    {
        po::store(po::command_line_parser(argc, argv).options(all).positional(pos_desc).run(), vm);
        // po::notify(vm);
        if (argc == 1 || vm.count("help"))
        {
            PrintUsageAndQuit(all, 0);
        }
        if (vm.count("version"))
        {
            fmt::print(GREEN("CMaker") " version {}\n", GetVersionString());
            return 0;
        }
    }
    catch (std::exception const &e)
    {
        LOGERR("error: {}\n", e.what());
    }

    if (vm.count("operation"))
    {
        auto op = vm["operation"].as<std::string>();
        if (op == "new")
        {
            CreateNewProject();
        }
        else if (op == "add-library")
        {
            AddThirdpartyLibrary();
        }
        else if (op == "add-submodule")
        {
            AddSubmodule();
        }
        else
        {
            LOGWARN("invalid operation: {}\n", op);
            PrintUsageAndQuit(all, 1);
        }
    }
    else
    {
        LOGWARN("you didn't set any operation.");
        PrintUsageAndQuit(all, 1);
    }
    return 0;
}