#include "FileSystem/FileSystem.h"
#include "FileSystem/UnmanagedMemoryFile.h"
#include "Base/BaseTypes.h"
#include "Scene3D/PathManip.h"
#include "_mcpp.h"

struct FileEntry
{
    std::string file_name;
    FILE* handle;
    int eof;
    DAVA::File* file;
};

const char* MCPP_Text = "<input>";
mcpp_preprocessed_text _mcpp_preprocessed_text = {};
std::vector<std::string> IncludeSearchPath;
static std::vector<FileEntry> _FileEntry;
static FILE* const _HandleBase = reinterpret_cast<FILE*>(0x1000);

int mcpp_fputc_impl(int ch, OUTDEST dst)
{
    if (dst == MCPP_OUT)
    {
        *(_mcpp_preprocessed_text.buffer + _mcpp_preprocessed_text.pos) = static_cast<char>(ch);
        ++_mcpp_preprocessed_text.pos;
    }
    return ch;
}

int mcpp_fputs_impl(const char* str, OUTDEST dst)
{
    if (dst == MCPP_OUT)
    {
        DAVA::uint32 length = static_cast<DAVA::uint32>(strlen(str));
        memcpy(_mcpp_preprocessed_text.buffer + _mcpp_preprocessed_text.pos, str, length);
        _mcpp_preprocessed_text.pos += length;
    }
    return 0;
}

int mcpp_fprintf_impl(OUTDEST dst, const char* format, ...)
{
    static const DAVA::uint32 localBufferSize = 2048;
    int count = 0;
    if (dst == MCPP_OUT)
    {
        char localBuffer[localBufferSize] = {};
        va_list arglist;
        va_start(arglist, format);
        count = vsnprintf(localBuffer, localBufferSize, format, arglist);
        va_end(arglist);
        mcpp_fputs_impl(localBuffer, dst);
    }
    return count;
}

//------------------------------------------------------------------------------

void mcpp__set_cur_file(const char* filename)
{
    DAVA::FilePath fileDir = DAVA::FilePath(filename).GetDirectory();

    IncludeSearchPath.clear();
    IncludeSearchPath.push_back(fileDir.IsEmpty() ? std::string() : fileDir.GetFrameworkPath());
    IncludeSearchPath.push_back(DAVA::FilePath("~res:/Materials/Shaders/").MakeDirectoryPathname().GetFrameworkPath());
}

//------------------------------------------------------------------------------

void mcpp__set_input(const void* data, unsigned data_sz)
{
    mcpp__cleanup();

    FileEntry entry;

    entry.file = new DAVA::UnmanagedMemoryFile(reinterpret_cast<const DAVA::uint8*>(data), data_sz);
    entry.file_name = MCPP_Text;
    entry.eof = 0;
    entry.handle = _HandleBase + 0;

    _FileEntry.push_back(entry);
}

//------------------------------------------------------------------------------

extern void xbegin_allocations(); // defined in support.cpp

void mcpp__startup()
{
    xbegin_allocations();
}

//------------------------------------------------------------------------------

extern void xend_allocations(); // defined in support.cpp

void mcpp__shutdown()
{
    xend_allocations();
}

//------------------------------------------------------------------------------

void mcpp__cleanup()
{
    for (unsigned i = 0; i != _FileEntry.size(); ++i)
    {
        _FileEntry[i].file->Release();
    }

    _FileEntry.clear();
}

//------------------------------------------------------------------------------

FILE* mcpp__fopen(const char* filename, const char* mode)
{
    FILE* file = NULL;

    if ((!strcmp(filename, MCPP_Text)) || (filename[0] == '/' && strcmp(filename + 1, MCPP_Text) == 0))
    {
        file = _FileEntry[0].handle;
    }
    else
    {
        for (std::vector<std::string>::const_iterator p = IncludeSearchPath.begin(), p_end = IncludeSearchPath.end(); p != p_end; ++p)
        {
            std::string name = *p;
            if (filename[0] == '/')
                name += (filename + 1);
            else
                name += filename;

            if (DAVA::FileSystem::Instance()->IsFile(name.c_str()))
            {
                FileEntry entry;

                entry.file = DAVA::File::Create(name, DAVA::File::OPEN | DAVA::File::READ);
                entry.file_name = filename;
                entry.eof = 0;
                entry.handle = _HandleBase + _FileEntry.size();

                _FileEntry.push_back(entry);
                file = entry.handle;

                break;
            }
        }
        /*
else
{
DAVA::Logger::Error("can't open \"%s\"",filename);
    }
    */
    }

    //DAVA::Logger::Info("mcpp-open \"%s\" %p",filename,file);
    return file;
};

//------------------------------------------------------------------------------

int mcpp__fclose(FILE* file)
{
    //DAVA::Logger::Info("mcpp-close %p",file);
    int retval = -1;

    for (unsigned i = 0; i != _FileEntry.size(); ++i)
    {
        if (_FileEntry[i].handle == file)
        {
            _FileEntry[i].file->Release();
            _FileEntry.erase(_FileEntry.begin() + i);
            retval = 0;
            break;
        }
    }

    return retval;
}

//------------------------------------------------------------------------------

int mcpp__ferror(FILE* file)
{
    int eof = 0;

    for (std::vector<FileEntry>::const_iterator f = _FileEntry.begin(), f_end = _FileEntry.end(); f != f_end; ++f)
    {
        if (f->handle == file)
        {
            eof = f->eof;
            break;
        }
    }

    return eof;
}

//------------------------------------------------------------------------------

char* mcpp__fgets(char* buf, int max_size, FILE* file)
{
    for (std::vector<FileEntry>::iterator f = _FileEntry.begin(), f_end = _FileEntry.end(); f != f_end; ++f)
    {
        if (f->handle == file)
        {
            if (f->file->IsEof())
            {
                buf[0] = 0;
                f->eof = 0;
            }
            else
            {
                f->file->ReadLine(static_cast<void*>(buf), max_size);

                // workaround to prevent MCPP from ignoring line
                if (!buf[0])
                {
                    buf[0] = ' ';
                    buf[1] = 0;
                }
            }

            //DAVA::Logger::Info("mcpp-read %p  \"%s\"",f->handle,buf);
            break;
        }
    }

    return buf;
}

//------------------------------------------------------------------------------

int mcpp__stat(const char* path, stat_t* buffer)
{
    int ret = -1;

    if (strcmp(path, MCPP_Text) != 0)
    {
        memset(buffer, 0, sizeof(stat_t));
        buffer->st_mode = S_IFREG
#if defined(__DAVAENGINE_ANDROID__)
        | S_IRUSR
#endif
        ;
        ret = 0;
    }

    return ret;
}
