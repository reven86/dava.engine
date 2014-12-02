#ifndef __FileTools__
#define __FileTools__

#include <string>

namespace FileTool
{
    enum eCreateDirectoryResult
    {
        DIRECTORY_CANT_CREATE = 0,
        DIRECTORY_EXISTS = 1,
        DIRECTORY_CREATED = 2,
    };

    eCreateDirectoryResult CreateDir( const std::string& dirPath );
    std::string GetBasename( const std::string& path );
    std::string WithNewExtension( const std::string& path, const std::string&  extension );
    std::string ReplaceDirectory( const std::string& path, const std::string &directory );
    std::string ReplaceBasename( const std::string& path, const std::string &basename );
}

#endif /* defined(__FileTools__) */