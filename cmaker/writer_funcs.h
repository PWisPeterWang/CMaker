#pragma once
#include "functions.h"
#include <fstream>

enum class RepoType
{
    STATIC,
    SHARED,
    EXECUTABLE,
};

struct WriterContext
{
    std::string repo_name;
    RepoType repo_type;
    std::string cxx_std{"11"};
    std::string licence;
};

void WriteCMakeLists(WriterContext const& ctx);
void WriteUnitTests(WriterContext const& ctx);
void WriteBenchmark(WriterContext const& ctx);
void WriteSrcAndHeader(WriterContext const& ctx);
void WriteGitignore();
void WriteClangformat();
void WriteLicense(WriterContext const& ctx);
void WriteReadme(WriterContext const& ctx);
