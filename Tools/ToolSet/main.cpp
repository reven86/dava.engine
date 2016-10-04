#include <stdlib.h>
#include <stdio.h>
#include <string>
int main( int argc,char** argv )
{
    std::string pathExec( argv[0] );
    std::string pathDir, openComand;
    
    const size_t last_slash_idx = pathExec.rfind('/');
    
    if (std::string::npos != last_slash_idx)
    {
        pathDir = pathExec.substr(0, last_slash_idx);
    }

    openComand = "open " + pathDir;
    
    system ( openComand.c_str() );
    
    return 1;
}