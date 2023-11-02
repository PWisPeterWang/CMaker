#include "functions.h"
#include <boost/tokenizer.hpp>

const char *GetVersionString()
{
#define XX(x) #x
#define STRINGIFY(x) XX(x)
    return STRINGIFY(CMAKER_VERSION_MAJOR.CMAKER_VERSION_MINOR.CMAKER_VERSION_PATCH);
#undef STRINGIFY
#undef XX
}

bool CreateDirIfNotExist(std::string name)
{
    if (fs::exists(name))
    {
        LOGERR("directory {} already exists!", name);
        return false;
    }
    std::error_code ec;
    if (!fs::create_directories(name, ec))
    {
        LOGERR("directory {} create failed! reason: {}", name, ec.message());
        return false;
    }
    LOGINFO("directory {} created!", name);
    return true;
}

bool IsProjectRoot()
{
    return fs::exists(".git") && fs::exists("CMakeLists.txt");
}

bool IsUnderCurrentDirectory(fs::path const &path)
{
    auto reletive = fs::canonical(path).lexically_relative(fs::current_path());
    return reletive == "." || reletive.string().substr(0, 2) != "..";
}

bool IsExecutableInPath(std::string exe)
{
    std::string path_env = std::getenv("PATH");
    // PATH delimeter in LINUX is ':' and in WIN is ';'
    boost::char_separator<char> delim(":;");
    boost::tokenizer<boost::char_separator<char>> tokens(path_env, delim);
    for (auto const &path : tokens)
    {
        fs::path exepath = fs::path(path) / exe;
        if (fs::exists(exepath) && fs::is_regular_file(exepath))
        {
            return true;
        }
    }
    return false;
}

const char *GetLicense()
{
    return R"(MIT License

Copyright (c) 2023 wbvalid@gmail.com

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
THE SOFTWARE.
)";
}

void PrintUsageAndQuit(po::options_description &desc, int errc)
{
    std::cout << std::endl << desc << std::endl;
    if (errc == 0)
        std::cout << GetLicense() << '\n';
    exit(errc);
}

bool PromptUserConfirm()
{
    std::string user_input;
    std::cin >> user_input;
    return ToUpper(user_input) == "Y";
}

std::string Pascalization(std::string str)
{
    boost::char_separator<char> delim("-_");
    boost::tokenizer<boost::char_separator<char>> tokens(str, delim);

    std::string result;
    for (auto tok : tokens)
    {
        if (!tok.empty())
        {
            tok[0] = ::toupper(tok[0]);
            result += tok;
        }
    }

    return result;
}