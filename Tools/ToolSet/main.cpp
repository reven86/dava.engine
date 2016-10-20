#include <stdlib.h>
#include <stdio.h>
#include <string>

#include <libproc.h>
#include <unistd.h>

#include <fstream>
#include <iostream>

int main(int argc, char** argv)
{
    char path[PATH_MAX];
    proc_pidpath(getpid(), path, PATH_MAX);

    std::string pathExec(path);
    std::string pathDir;

    std::cout << path << "  !!!";

    const size_t last_slash_idx = pathExec.rfind('/');

    if (std::string::npos != last_slash_idx)
    {
        pathDir = pathExec.substr(0, last_slash_idx);
    }

    sprintf(path, "sh %s/../Resources/open.sh %s", pathDir.c_str(), pathDir.c_str());
    system(path);

    // std::string openScriptFile(pathDir + "/../Resources/OpenFinder.script");

    return 1;
}