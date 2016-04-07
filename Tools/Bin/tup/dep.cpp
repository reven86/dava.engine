#include <iostream>
#include <fstream>
#include <filesystem>
#include <string>
#include <set>

int main(int argc, const char* argv[])
{
    bool cmdok = false;

    if (argc > 1)
    {
        const char* cmd = argv[1];

        if (0 == strcmp(cmd, "help") || 0 == strcmp(cmd, "--help"))
        {
            cmdok = true;
            std::cout << std::endl;
            std::cout << "Usage: dep <command> [<args>]" << std::endl;
            std::cout << std::endl;
            std::cout << "Commands:" << std::endl;
            std::cout << "    echo [options] [<file>...] - Try to open each file and if success prints it name" << std::endl;
            std::cout << "    options:" << std::endl;
            std::cout << "        -p <prefix>            - add prefix to printed name" << std::endl;
            std::cout << std::endl;
            std::cout << "    cat [<wildcard>...] - Try to open each file and if success prints it content" << std::endl;
        }
        else if (0 == strcmp(cmd, "echo"))
        {
            cmdok = true;
            if (argc > 2)
            {
                std::string prefix;
                std::set<std::string> files;

                // collect all files and options
                for (int i = 2; i < argc; ++i)
                {
                    if (0 == strcmp(argv[i], "-p") && (i + 1) < argc)
                    {
                        i++;
                        prefix = argv[i];
                        prefix.append("/");
                    }
                    else
                    {
                        files.insert(argv[i]);
                    }
                }

                // process files
                for (auto& path : files)
                {
                    std::ifstream file(path);
                    if (file)
                    {
                        std::cout << prefix << path << std::endl;
                    }
                }
            }
        }
        else if (0 == strcmp(cmd, "cat"))
        {
            cmdok = true;
            if (argc > 2)
            {
                std::set<std::string> files;

                // collect all files and options
                for (int i = 2; i < argc; ++i)
                {
                    if (0 /* no options at the moment */)
                    {
                    }
                    else
                    {
                        files.insert(argv[i]);
                    }
                }

                // process files
                for (auto& path : files)
                {
                    /*
                    std::dire
                    std::ifstream file(path);
                    if (file)
                    {
                        std::string line;
                        while (std::getline(file, line))
                        {
                            std::cout << line << std::endl;
                        }
                    }
                    */
                }
            }
        }

        /*
        for (int i = 2; i < argc; ++i)
        {
            const char *path = argv[i];
            FILE *f = fopen(path, "r");
            if (NULL != f)
            {
                printf("%s/%s\n", dir, path);
                fclose(f);
            }
        }
        */
    }

    int ret = 0;
    if (!cmdok)
    {
        ret = 1;
        std::cerr << "Error: unknown command" << std::endl;
    }

    return ret;
}
