#include <stdlib.h>
#include <stdio.h>
#include <string>

#include <libproc.h>
#include <unistd.h>

#include <fstream>

int main(int argc, char** argv)
{
    char path[PATH_MAX];
    proc_pidpath(getpid(), path, PATH_MAX);

    std::string pathExec(path);
    std::string pathDir;

    const size_t last_slash_idx = pathExec.rfind('/');

    if (std::string::npos != last_slash_idx)
    {
        pathDir = pathExec.substr(0, last_slash_idx);
    }

    std::string openScriptFile(pathDir + "/../Resources/OpenFinder.script");

    std::ofstream openScriptStream(openScriptFile, std::ios::out);

    openScriptStream << "tell application \"Finder\"\n";
    openScriptStream << "    open (\"" << pathDir << "/\" as POSIX file)\n";
    openScriptStream << "    activate\n";
    openScriptStream << "end tell";

    openScriptStream.close();

    sprintf(path, "osascript %s", openScriptFile.c_str());
    system(path);

    return 1;
}