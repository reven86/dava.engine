/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_TOOLS_PATCH_FILE_H__
#define __DAVAENGINE_TOOLS_PATCH_FILE_H__

#include "FileSystem/FilePath.h"
#include "BSDiff.h"

namespace DAVA
{

class File;

// ======================================================================================
// information about patch
// ======================================================================================
struct PatchInfo
{
    friend class PatchFileReader;
    friend class PatchFileWriter;

    String origPath;
    uint32 origCRC;
    uint32 origSize;

    String newPath;
    uint32 newSize;
    uint32 newCRC;

    PatchInfo();

protected:
    void Reset();
    bool Write(File* file);
    bool Read(File *file);

private:
    bool ReadString(File* file, String &);
    bool WriteString(File* file, const String &);
};

// ======================================================================================
// class for creating/writing patch file
// ======================================================================================
class PatchFileWriter
{
public:
    enum WriterMode
    {
        WRITE,  // Create an empty file for output operations
        APPEND  // Open file for output at the end of a file. The file will be created if it does not exist.
    };

    PatchFileWriter(const FilePath &path, WriterMode mode, BSType diffType, bool beVerbose = false);
    ~PatchFileWriter();

    // TODO:
    // description
    bool Write(const FilePath &origBase, const FilePath &origPath, const FilePath &newBase, const FilePath &newPath);

protected:
    bool SingleWrite(const FilePath &origBase, const FilePath &origPath, const FilePath &newBase, const FilePath &newPath);
    void EnumerateDir(const FilePath &path, const FilePath &base, List<String> &in);

    DAVA::FilePath patchPath;
    BSType diffType;
    bool verbose;
};

// ======================================================================================
// class for reading/applying patch file
// ======================================================================================
class PatchFileReader
{
public:
    enum PatchError
    {
        ERROR_NO = 0,
        ERROR_MEMORY,       // can't allocate memory
        ERROR_CANT_READ,    // path file can't be read
        ERROR_CORRUPTED,    // path file is corrupted
        ERROR_EMPTY_PATCH,  // no data to apply patch
        ERROR_ORIG_READ,    // file on origPath can't be opened for reading
        ERROR_ORIG_CRC,     // file on origPath has wrong crc to apply patch
        ERROR_NEW_CREATE,   // file on newPath can't be opened for writing
        ERROR_NEW_WRITE,    // file on newPath can't be written
        ERROR_NEW_CRC,      // file on newPath has wrong crc after applied patch
        ERROR_UNKNOWN
    };

    PatchFileReader(const FilePath &path, bool beVerbose = false);
    ~PatchFileReader();

    bool ReadFirst();
    bool ReadLast();
    bool ReadNext();
    bool ReadPrev();

    const PatchInfo* GetCurInfo() const;
    
    PatchError GetLastError() const;
    PatchError GetParseError() const;
    int32 GetErrno() const;

    bool Truncate();
    bool Apply(const FilePath &origBase, const FilePath &origPath, const FilePath &newBase, const FilePath &newPath);

protected:
    File *patchFile;
    PatchInfo curInfo;
    PatchError lastError;
    PatchError parseError;
    int32 curErrno;
    bool verbose;
    bool eof;

    Vector<int32> patchPositions;
    size_t initialPositionsCount;
    size_t curPatchIndex;
    uint32 curBSDiffPos;

    bool DoRead();
    bool ReadDataBack(void *data, uint32 size);
};

}

#endif // __DAVAENGINE_TOOLS_RESOURCE_PATCHER_H__
