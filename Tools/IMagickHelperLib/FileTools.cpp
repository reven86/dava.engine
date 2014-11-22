#include "FileTools.h"


#if defined( WIN32 )
#include <direct.h>
#include <io.h> 
#include <sys/types.h>
#include <sys/stat.h>
#include <Shlobj.h>
#include <tchar.h>

#else
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/errno.h>
#include <copyfile.h>
#include <libproc.h>
#include <libgen.h>

#endif 

using namespace std;

namespace FileTool
{

bool IsDirectory(const string& pathToCheck)
{
#if defined (WIN32)
    DWORD stats = GetFileAttributesA(pathToCheck.c_str());
    return (stats != -1) && (0 != (stats & FILE_ATTRIBUTE_DIRECTORY));
#else
    struct stat s;
    if(stat(pathToCheck.c_str(), &s) == 0)
    {
        return (0 != (s.st_mode & S_IFDIR));
    }
#endif
    
    return false;
}

eCreateDirectoryResult CreateDir( const string& dirPath )
{
    if( IsDirectory( dirPath ) )
        return DIRECTORY_EXISTS;

#ifdef WIN32
    BOOL res = ::CreateDirectoryA(dirPath.c_str(), 0);
#else
    int res = mkdir(dirPath.c_str(), 0777);
#endif 

    return (res == 0) ? DIRECTORY_CANT_CREATE : DIRECTORY_CREATED;

}

string  GetFilename(const string &path)
{
    string::size_type dotpos = path.rfind(string("/"));
    if (dotpos == string::npos)
    {
        dotpos = path.rfind(string("\\"));
        if (dotpos == string::npos)
        {
            return path;
        }
    }

    return path.substr(dotpos+1);
}

string GetExtension( const string& path  ) 
{
    string filename = GetFilename( path );

    string::size_type dotpos = filename.rfind(string("."));
    if (dotpos == string::npos)
        return string();

    return filename.substr(dotpos);
}

string GetBasename( const string& path )
{
    const string filename = GetFilename( path );

    const string::size_type dotpos = filename.rfind(string("."));
    if (dotpos == string::npos)
        return filename;

    return filename.substr(0, dotpos);
}

string GetDirectory( const string& path )
{
    string directory;

    const string::size_type slashpos  = path.rfind(string("/"));
    const string::size_type slashpos2 = path.rfind(string("\\"));

    if (slashpos != string::npos)
    {
        directory = path.substr(0, slashpos + 1);
    }
    else
    if (slashpos2 != string::npos)
    {
        directory = path.substr(0, slashpos2 + 1);   
    }

    return directory;
}

string WithNewExtension( const string& path, const string&  extension )
{
    if( !path.length() )
    {
        return path;
    }

    const string basename = GetBasename( path );

    return GetDirectory( path ) + (basename + extension);
}

std::string ReplaceDirectory( const std::string& path, const string &directory )
{
    string filename = GetFilename( path );
    return directory.length() ?  directory + "/" + filename : filename;
}

string ReplaceBasename( const std::string& path, const string &basename )
{
    string extension = GetExtension( path );
    return GetDirectory( path ) + (basename + extension);
} 

}