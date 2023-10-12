#pragma once
#include <boost/program_options.hpp>
#include <fmt/format.h>
#include <iostream>
#include <fstream>
#include <string>
#include <filesystem>
namespace fs = std::filesystem;
namespace po = boost::program_options;

#define CMAKER_VERSION_MAJOR 0
#define CMAKER_VERSION_MINOR 0
#define CMAKER_VERSION_PATCH 1

#define COLORED(xx, id) "\033[1;" #id "m" xx "\033[0m"
#define RED(xx) COLORED(xx, 31)
#define GREEN(xx) COLORED(xx, 32)
#define YELLOW(xx) COLORED(xx, 33)
#define BLUE(xx) COLORED(xx, 34)
#define PINK(xx) COLORED(xx, 35)
#define SKY(xx) COLORED(xx, 36)

void PrintUsageAndQuit(po::options_description &des, int errc = 1);

#define INFO(xx, ...) fmt::print("[" GREEN("INFO") "] " xx "\n", ##__VA_ARGS__)
#define WARN(xx, ...) fmt::print("[" YELLOW("WARN") "] " xx "\n", ##__VA_ARGS__)
#define ERROR(xx, ...)                                                                             \
    do                                                                                             \
    {                                                                                              \
        fmt::print(stderr, "[" RED("ERROR") "] " xx "\n", ##__VA_ARGS__);                          \
        exit(1);                                                                                   \
    } while (0)

const char *GetVersionString();
const char *GetLicense();
void CreateNewProject();
void AddThirdpartyLibrary();
void AddSubmodule();

// true on success false on fail (exists)
bool CreateDirIfNotExist(std::string name);

// check if current working directoy is the root directory of project
bool IsProjectRoot();

// check if the path is under current working directory, useful to verify the path of submodule
bool IsUnderCurrentDirectory(fs::path const &path);

// check if the executable is in PATH
bool IsExecutableInPath(std::string exe);

bool PromptUserConfirm();

// convert string into Pascal Case naming style
std::string Pascalization(std::string str);

inline std::string ToUpper(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](int c) { return std::toupper(c); });
    return str;
}

inline std::string ToLower(std::string str)
{
    std::transform(str.begin(), str.end(), str.begin(), [](int c) { return std::tolower(c); });
    return str;
}

struct PushD
{
    PushD(std::string name)
        : cwd(fs::current_path())
        , target(name)
    {
        std::error_code ec;
        fs::current_path(target, ec);
        if (ec)
        {
            ERROR("pushd to {} error, reason: {}", target.string(), ec.message());
            exit(1);
        }
        INFO("entering directory {}", target.string());
    }
    ~PushD()
    {
        std::error_code ec;
        fs::current_path(cwd, ec);
        if (ec)
        {
            ERROR("popd to {} error, reason: {}", cwd.string(), ec.message());
        }
        INFO("leaving directory {}", target.string());
    }

    fs::path cwd;
    fs::path target;
};