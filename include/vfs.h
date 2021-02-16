#pragma once
#include <string>
#include <filesystem> 
#include <optional>
namespace fs = std::filesystem;

#ifdef _WIN32
const std::string SEP = "\\";
#else
const std::string SEP = "/";
#endif
const std::string UP_DIR = "..";
const std::string CURRENT_DIR = ".";

class Vfs
{
public:
    static fs::path GetRootPath();
    static fs::path GetResPath();
    static fs::path GetShaderPath();
    static fs::path GetWorkingDirectoryPath();
    static fs::path GetExecutablePath();
    static std::optional<fs::path> SearchFolderInCurPathAndNestedParents(fs::path curPath,std::string folder);
    static fs::path ConcatPath(fs::path pathA, std::string pathB);
};