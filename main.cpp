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
        ("operation", po::value<std::string>()->required(), "supported operations: new, add-library, add-submodule")
        ("name", po::value<std::string>()->required(), "operand for the operation")
        ("version,v", "print the version string")
        ;
    
    creator.add_options()("static", "create a repo template based on library project (static library) [default]")
        ("shared", "create a repo template based on library project (shared library)")
        ("exe", "create a repo template based on executable project")
        ;
    adder_library.add_options()
        ("path,L", po::value<std::string>(), "path to find the library")
        ("inc,I", po::value<std::string>(), "path to find the headers")
        ;
    adder_module.add_options()
        ("url,U", po::value<std::string>(), "url to submodule")
        ("dir,D", po::value<std::string>()->default_value("thirdparty"), "directory where the submodule shall be init")
        ;
    // clang-format on
    pos_desc.add("operation", 1).add("name", 1);

    all.add(general).add(creator).add(adder_library).add(adder_module);

    if (!IsExecutableInPath("git"))
    {
        ERROR("cmaker requires " GREEN("git") " to create repo!");
    }

    if (!IsExecutableInPath("cmake"))
    {
        ERROR("cmaker requires " GREEN("cmake") " on your system!");
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
        ERROR("error: {}\n", e.what());
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
            fmt::print("invalid operation: {}\n", op);
            std::cout << all;
            return 1;
        }
    }
    else
    {
        std::cout << "you didn't set any operation.\n";
        std::cout << all;
        return 1;
    }
    return 0;
}