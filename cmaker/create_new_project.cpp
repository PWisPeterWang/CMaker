#include "functions.h"
#include "writer_funcs.h"

extern po::options_description creator;
extern po::variables_map vm;

void CreateNewProject()
{
    WriterContext ctx;
    if (!vm.count("name"))
    {
        LOGERR("missing repo name for creating project");
        PrintUsageAndQuit(creator);
    }
    ctx.repo_name = vm["name"].as<std::string>();
    if (vm.count("shared"))
    {
        LOGINFO("creating repo for shared library: {}", ctx.repo_name);
        ctx.repo_type = RepoType::SHARED;
    }
    else if (vm.count("exe"))
    {
        LOGINFO("creating repo for executable: {}", ctx.repo_name);
        ctx.repo_type = RepoType::EXECUTABLE;
    }
    else
    {
        LOGINFO("creating repo for static library: {}", ctx.repo_name);
    }
    // has default value = 11
    ctx.cxx_std = vm["std"].as<std::string>();
    // has default value = MIT
    ctx.license = vm["license"].as<std::string>();

    if (!CreateDirIfNotExist(ctx.repo_name))
    {
        LOGERR("abort!");
        exit(1);
    }

    // with chdir to
    {
        PushD _(ctx.repo_name);

        CreateDirIfNotExist(ctx.repo_name);
        CreateDirIfNotExist("cmake_modules");
        CreateDirIfNotExist("thirdparty");
        CreateDirIfNotExist("bench");
        CreateDirIfNotExist("unit_test");
        WriteCMakeLists(ctx);
        WriteUnitTests(ctx);
        WriteBenchmark(ctx);
        WriteSrcAndHeader(ctx);
        WriteGitignore();
        WriteClangformat();
        WriteLicense(ctx);
        WriteReadme(ctx);

        std::system("git init");
    }
}
