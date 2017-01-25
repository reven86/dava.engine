#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <sstream>

#include "pack_archive.h"

using DWORD = std::uint32_t;
const std::uint32_t MAX_PATH = 260;

#if defined(__MINGW32__)
#define DLL_EXPORT __declspec(dllexport)
#define STDCALL __stdcall
#else /* defined (_WIN32) */
#define DLL_EXPORT
#define STDCALL
#endif

#include "wcxhead.h"

using HANDLE = void*;

extern "C" {
DLL_EXPORT HANDLE STDCALL
OpenArchive(tOpenArchiveData* ArchiveData);
DLL_EXPORT int STDCALL
ReadHeader(HANDLE hArcData, tHeaderData* HeaderData);
DLL_EXPORT int STDCALL
ProcessFile(HANDLE hArcData, int Operation, char* DestPath, char* DestName);
DLL_EXPORT int STDCALL CloseArchive(HANDLE hArcData);
DLL_EXPORT void STDCALL
SetChangeVolProc(HANDLE hArcData, tChangeVolProc pChangeVolProc1);
DLL_EXPORT void STDCALL
SetProcessDataProc(HANDLE hArcData, tProcessDataProc pProcessDataProc);
DLL_EXPORT int STDCALL GetPackerCaps();
}

std::ofstream l;

HANDLE STDCALL OpenArchive(tOpenArchiveData* ArchiveData)
{
    if (!l.is_open())
    {
        const char* logger_path = std::getenv("DVPK_PLUGIN_LOG");
        if (logger_path != nullptr)
        {
            l.open(logger_path);
        }
    }

    PackArchive* archive = nullptr;
    try
    {
        l << "begin open archive: " << ArchiveData->ArcName << '\n';

        archive = new PackArchive(ArchiveData->ArcName);

        l << "open archive: " << ArchiveData->ArcName << '\n';
    }
    catch (std::exception& ex)
    {
        l << ex.what() << std::flush;
        ArchiveData->OpenResult = E_BAD_ARCHIVE;
    }
    return archive;
}

int STDCALL CloseArchive(HANDLE hArcData)
{
    PackArchive* archive = reinterpret_cast<PackArchive*>(hArcData);

    l << "close archive: " << archive->arcName << '\n';

    delete archive;
    return 0;
}

// called multiple times till return 0 on finish files
int STDCALL ReadHeader(HANDLE hArcData, tHeaderData* HeaderData)
{
    l << "read header\n";

    PackArchive* archive = reinterpret_cast<PackArchive*>(hArcData);

    const std::vector<FileInfo>& files = archive->GetFilesInfo();
    if (archive->fileIndex < files.size())
    {
        const FileInfo& info = files.at(archive->fileIndex);

        std::string name(info.relativeFilePath);

#if defined(__MINGW32__) || defined(_MSVC)
        std::for_each(begin(name), end(name), [](char& ch) {
            if (ch == '/')
            {
                ch = '\\';
            }
        });
#endif

        HeaderData->FileAttr = 1; // FILE_SHARE_READ;
        std::strncpy(HeaderData->FileName, name.c_str(), MAX_PATH);
        std::strncpy(HeaderData->ArcName, archive->arcName.c_str(), MAX_PATH);

        HeaderData->FileCRC = info.hash;
        HeaderData->FileTime = 0;
        HeaderData->UnpSize = info.originalSize;
        HeaderData->PackSize = info.compressedSize;

        archive->lastFileName = info.relativeFilePath;

        l << "read header file: " << HeaderData->FileName << " relative: "
          << info.relativeFilePath << " index: " << archive->fileIndex << '\n';

        ++archive->fileIndex;
    }
    else
    {
        if (archive->fileIndex == files.size() && archive->HasMeta())
        {
            ++archive->fileIndex;
            std::memset(HeaderData, 0, sizeof(*HeaderData));

            std::strncpy(HeaderData->FileName, "meta.meta", MAX_PATH);
            std::strncpy(HeaderData->ArcName, archive->arcName.c_str(), MAX_PATH);
            HeaderData->PackSize = 0;
            auto& meta = archive->GetMeta();
            HeaderData->UnpSize = meta.GetNumPacks();

            archive->lastFileName = "meta.meta";

            l << "add meta file\n";
            return 0;
        }
        else
        {
            archive->fileIndex = 0;
            std::memset(HeaderData, 0, sizeof(*HeaderData));

            l << "end header\n";
            return E_END_ARCHIVE;
        }
    }

    return 0;
}

int STDCALL ProcessFile(HANDLE hArcData, int Operation, char* DestPath,
                        char* DestName)
{
    l << "process_file\n";

    if (PK_SKIP == Operation)
    {
    }
    else if (PK_TEST == Operation)
    {
    }
    else if (PK_EXTRACT == Operation)
    {
        PackArchive* archive = reinterpret_cast<PackArchive*>(hArcData);
        if (archive->lastFileName == std::string("meta.meta"))
        {
            std::string data = archive->PrintMeta();
            std::string outputName = DestName ? DestName : DestPath;

            std::ofstream out(outputName, std::ios_base::binary);
            out.write(reinterpret_cast<const char*>(data.data()), data.size());
        }
        else if (archive->HasFile(archive->lastFileName))
        {
            std::vector<uint8_t> data;
            archive->LoadFile(archive->lastFileName, data);
            std::string outputName = DestName ? DestName : DestPath;

            std::ofstream out(outputName, std::ios_base::binary);
            out.write(reinterpret_cast<const char*>(data.data()), data.size());
        }
    }
    return 0;
}

void STDCALL
SetChangeVolProc(HANDLE hArcData, tChangeVolProc pChangeVolProc1)
{
}

int STDCALL GetPackerCaps()
{
    return PK_CAPS_MULTIPLE; // | PK_CAPS_HIDE;
}

void STDCALL SetProcessDataProc(HANDLE hArcData, tProcessDataProc pProcessDataProc)
{
    // do nothing
}
