#include <Vfs.h>
#include <iostream>

void AppendPath(fs::path & srcPath, fs::path && appendingPath)
{
    srcPath += SEP;
    srcPath += appendingPath;
}

void AppendPath(fs::path & srcPath, std::string && appendingPath)
{
    AppendPath(srcPath, fs::path(appendingPath));

}

fs::path Vfs::ConcatPath(fs::path pathA, std::string pathB)
{
    auto pathCopy = pathA;
    AppendPath(pathCopy, pathB);
    return pathCopy;
}

fs::path Vfs::GetRootPath()
{
    static std::optional<fs::path> configPath = SearchFolderInCurPathAndNestedParents(fs::current_path(),std::string("toyEngineConfig"));

    return configPath.value().parent_path();
}

fs::path Vfs::GetResPath()
{
    static std::optional<fs::path> rootPath = SearchFolderInCurPathAndNestedParents(fs::current_path(),std::string("res"));
    return rootPath.value();
}

fs::path Vfs::GetShaderPath()
{
    static std::optional<fs::path> rootPath = SearchFolderInCurPathAndNestedParents(fs::current_path(),std::string("shaders"));
    return rootPath.value();
}

fs::path Vfs::GetExecutablePath()
{
    auto workingDirectory = fs::current_path();
    AppendPath(workingDirectory, std::string("HuntWheel.exe"));
    return workingDirectory;
}

fs::path Vfs::GetWorkingDirectoryPath()
{
    static fs::path workingDirPath = fs::current_path();
    return workingDirPath;
}

std::optional<fs::path> Vfs::SearchFolderInCurPathAndNestedParents(fs::path curPath,std::string folder)
{
    auto searchingPath = curPath;
    do 
    {
        auto p = searchingPath;
        p.append(folder);
        if (fs::exists(p))
        {
            return std::optional<fs::path>(p);
        }
        searchingPath = searchingPath.parent_path();
    }
    while (searchingPath!=curPath.root_path());
    return std::optional<fs::path>();
}

void DisplayDirTree(const fs::path& pathToShow, int level)
{
    if (fs::exists(pathToShow) && fs::is_directory(pathToShow))
    {
        auto lead = std::string(level * 3, ' ');
        for (const auto& entry : fs::directory_iterator(pathToShow))
        {
            auto filename = entry.path().filename();
            if (fs::is_directory(entry.status()))
            {
                std::cout << lead << "[+] " << filename << "\n";
                DisplayDirTree(entry, level + 1);
                std::cout << "\n";
            }
            else if (fs::is_regular_file(entry.status()))
            {

            }
                //DisplayFileInfo(entry, lead, filename);
            else
                std::cout << lead << " [?]" << filename << "\n";
        }
    }
}