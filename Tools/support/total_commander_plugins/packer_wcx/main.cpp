#include <sstream>
#include <algorithm>

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <advpub.h>

#include "pack_archive.h"
#include "wcxhead.h"

namespace plugin
{
HANDLE hInstance = 0;
} // end namespace plugin

std::vector<std::string>&
split(const std::string& s, char delim, std::vector<std::string>& elems)
{
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim))
    {
        elems.push_back(item);
    }
    return elems;
}

std::vector<std::string> split(const std::string& s, char delim)
{
    std::vector<std::string> elems;
    split(s, delim, elems);
    return elems;
}

extern "C"
{
__declspec(dllexport) HANDLE __stdcall
OpenArchive(tOpenArchiveData* ArchiveData);
__declspec(dllexport) int __stdcall
ReadHeader(HANDLE hArcData, tHeaderData* HeaderData);
__declspec(dllexport) int __stdcall
ProcessFile(HANDLE hArcData, int Operation, char* DestPath, char* DestName);
__declspec(dllexport) int __stdcall CloseArchive(HANDLE hArcData);
__declspec(dllexport) void __stdcall
SetChangeVolProc(HANDLE hArcData, tChangeVolProc pChangeVolProc1);
__declspec(dllexport) void __stdcall
SetProcessDataProc(HANDLE hArcData, tProcessDataProc pProcessDataProc);
__declspec(dllexport) int __stdcall GetPackerCaps();
}

BOOL APIENTRY DllMain(HANDLE hModule,
                      DWORD ul_reason_for_call,
                      LPVOID lpReserved
                      )
{
    // Save HINSTANCE value
    plugin::hInstance = static_cast<HINSTANCE>(hModule);
    OutputDebugStringA("start DllMain PackerExtension");
    return TRUE;
}

HANDLE __stdcall OpenArchive(tOpenArchiveData* ArchiveData)
{
    PackArchive* archive = nullptr;
    try
    {
        archive = new PackArchive(ArchiveData->ArcName);
    }
    catch (std::exception& ex)
    {
        OutputDebugStringA(ex.what());
        ArchiveData->OpenResult = E_BAD_ARCHIVE;
    }
    return archive;
}

int __stdcall CloseArchive(HANDLE hArcData)
{
    PackArchive* archive = reinterpret_cast<PackArchive*>(hArcData);
    delete archive;
    return 0;
}

int __stdcall ReadHeader(HANDLE hArcData, tHeaderData* HeaderData)
{
    PackArchive* archive = reinterpret_cast<PackArchive*>(hArcData);

    const std::vector<FileInfo>& files = archive->GetFilesInfo();
    if (files.size() > archive->fileIndex)
    {
        const FileInfo& info = files[archive->fileIndex];
        archive->fileIndex++;

        std::string name(info.relativeFilePath);

        std::for_each(begin(name), end(name), [](char& ch) {
            if (ch == '/')
            {
                ch = '\\';
            }
        });

        HeaderData->FileAttr = FILE_SHARE_READ;
        strcpy(HeaderData->FileName, name.c_str());
        archive->fileIndex++;

        strcpy(HeaderData->ArcName, archive->arcName.c_str());

        HeaderData->FileCRC = 0;

        HeaderData->FileTime = 0;

        HeaderData->UnpSize = info.originalSize;
        HeaderData->PackSize = info.compressedSize;

        archive->lastFileName = info.relativeFilePath;
    }
    else
    {
        archive->fileIndex = 0;
        return E_END_ARCHIVE;
    }

    return 0;
}

int __stdcall ProcessFile(HANDLE hArcData, int Operation, char* DestPath,
                          char* DestName)
{
    // Extract a frame data from avi file
    if (PK_SKIP == Operation)
    {
        // just check intergity skip it
    }
    else if (PK_TEST == Operation)
    {
    }
    else if (PK_EXTRACT == Operation)
    {
        PackArchive* archive = reinterpret_cast<PackArchive*>(hArcData);
        if (archive->HasFile(archive->lastFileName))
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

void __stdcall
SetChangeVolProc(HANDLE hArcData, tChangeVolProc pChangeVolProc1)
{
}

int __stdcall GetPackerCaps()
{
    // Accept New file, Options dialog, Multiple files, and hide (press CTRL+Down to enter avi file for reading)
    return PK_CAPS_MULTIPLE |
    PK_CAPS_HIDE /*|PK_CAPS_MODIFY|PK_CAPS_DELETE|PK_CAPS_OPTIONS|PK_CAPS_NEW*/;
}
