#include <stdlib.h>
#include <stdio.h>
#include <string>

#include <libproc.h>
#include <unistd.h>

int main(int argc, char** argv)
{
    char path[PATH_MAX];
    proc_pidpath(getpid(), path, PATH_MAX);

    std::string pathExec(path);
    std::string pathDir, openComand;

    const size_t last_slash_idx = pathExec.rfind('/');

    if (std::string::npos != last_slash_idx)
    {
        pathDir = pathExec.substr(0, last_slash_idx);
    }

    openComand = "open " + pathDir;

    system(openComand.c_str());

    return 1;
}