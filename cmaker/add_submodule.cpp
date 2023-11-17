#include "functions.h"
#include <boost/process.hpp>
namespace bp = boost::process;

extern po::options_description adder_module;
extern po::variables_map vm;

static std::string name = "undefined";

void AddSubmodule()
{
    if (!IsProjectRoot())
    {
        LOGERR("must be under the project root directory!");
    }

    if (!vm.count("name"))
    {
        LOGWARN("submodule name is required!");
        PrintUsageAndQuit(adder_module);
    }

    name = vm["name"].as<std::string>();
    if (name == "help")
    {
        std::cout << adder_module;
        return ;
    }

    if (!vm.count("url"))
    {
        LOGWARN("url is required to init submodule!");
        PrintUsageAndQuit(adder_module);
    }
    std::string url = vm["url"].as<std::string>();
    std::string module_dir = vm["dir"].as<std::string>();

    if (!IsUnderCurrentDirectory(module_dir))
    {
        LOGERR("submodule dir {{{}}} is not under current working directory {{{}}}", module_dir,
            fs::current_path().string());
    }

    auto target_path = fs::path(module_dir) / name;
    if (fs::exists(target_path) && !fs::is_empty(target_path))
    {
        LOGERR("the target path {{{}}} for submodule to be initialized exists and not empty!!",
            target_path.string());
    }

    auto command = fmt::format("git submodule add {} {}", url, target_path.string());
    int result = bp::system(command);
    if (0 != result)
    {
        LOGERR("submodule initialize error! ec:{}, reason:{}", result, strerror(result));
    }

    LOGINFO("submodule initialized in {}", fs::canonical(target_path).string());
    LOGINFO("please insert 'add_subdirectory({})' to add dependency", target_path.string());
    LOGINFO("or call 'cmaker add-library ...' to add dependency");
}