#include <stdio.h>

int main(int argc, const char* argv[])
{
    const char* dir = argv[1];
    for(int i = 2; i < argc; ++i)
    {
        const char *path = argv[i];
        FILE *f = fopen(path, "r");
        if(NULL != f)
        {
            printf("%s/%s\n", dir, path);
        }
        fclose(f);
    }

    return 0;
}
