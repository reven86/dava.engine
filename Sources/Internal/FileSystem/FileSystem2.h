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

#include "Base/BaseTypes.h"

namespace DAVA
{
    /// Path class for manipulation file name
    /// 1. all path in unicode
    /// 2. no changes for filesystem, only Path object
    /// examples:
    ///     ~res:/folder1/file1.txt
    ///     ~doc:/folder2/file2.txt
    ///     c:/mingw32/bin/gcc.exe
    ///     /Users/l_chayka/job/dava.framework
    ///     Data/files/config.yaml
    /// Grammar for portable generic path strings (from BOOST docs with modifications)
    /// The grammar is specified in extended BNF, with terminal symbols in quotes:
    ///     path ::= {[uri] | [root]} [relative-path]  // an empty path is valid
    ///     uri  ::= ~res:/ | ~doc:/
    ///     root ::= [root-name] [root-directory]
    ///     root-directory ::= separator
    ///     relative-path ::= path-element { separator path-element } [separator]
    ///    path-element ::= name | parent-directory | directory-placeholder
    ///     name ::= char { char }
    ///     directory-placeholder ::= "."
    ///     parent-directory ::= ".."
    ///     separator ::= "/"  // an implementation may define additional separators
    class Path final
    {
    public:
        Path();
        Path(const Path& path);
        Path(Path&& path);
        Path(const String& sourcePathUtf8);
        Path(const WideString& src);
        Path(const char* sourcePathUtf8);
        ~Path();

        Path& operator=(const Path&);
        Path& operator=(Path&&);
        Path operator+=(const Path&);
        Path operator+(const Path&);

        bool operator==(const Path& path) const;
        bool operator!=(const Path& path) const;

        bool IsEmpty() const;
        bool IsVirtual() const;
        bool HasRootName() const;
        bool HasRootDirectory() const;
        bool HasRootPath() const;
        bool HasRelativePath() const;
        bool HasParentPath() const;
        bool HasFilename() const;
        bool HasStem() const;
        bool HasExtension() const;
        bool IsAbsolute() const;
        bool IsRelative() const;

        Path GetRootName() const;
        Path GetRootDirectory() const;
        Path GetRootPath() const;
        Path GetRelativePath() const;
        Path GetParentPath() const;
        Path GetFilename() const;
        Path GetStem() const;
        Path GetExtension() const;

        void Clear();
        Path& RemoveFilename();
        Path& ReplaceExtension(const Path& newExtension = Path());

        String ToStringUtf8() const;
        WideString ToWideString() const;
        size_t Hash() const;
    private:
        String pathname;
    };

    // exceptions in case of error
    class InputStream
    {
    public:
        virtual ~InputStream();
        virtual uint64 Read(void* data, uint64 size) = 0;
        virtual void Seek(uint64 position) = 0;
        virtual uint64 Tell() = 0;
        virtual uint64 GetSize() = 0;
    };

    class OutputStream
    {
    public:
        virtual ~OutputStream();
        virtual void Write(void* data, int64 size) = 0;
        virtual void Seek(uint64 position) = 0;
        virtual uint64 Tell() = 0;
        virtual uint64 GetSize() = 0;
    };

    class FileDevice
    {
    public:
        enum class State
        {
            UNAVAILABLE,
            READ_ONLY,
            WRITE_ONLY,
            READ_WRITE
        };
        virtual ~FileDevice();
        virtual int32 GetPriority() = 0;
        virtual State GetState() = 0;
        virtual bool Exist(const Path&, uint64* fileSize = nullptr) = 0;
        virtual bool IsFile(const Path&) = 0;
        virtual bool IsDirectory(const Path&) = 0;
        virtual Vector<Path> EnumerateFiles(const Path& base = Path(), size_t depth = 0) = 0;
        virtual std::unique_ptr<InputStream> OpenFile(const Path&) = 0;
        virtual std::unique_ptr<OutputStream> CreateFile(const Path&, bool recreate) = 0;
        virtual void DeleteFile(const Path&) = 0;
        virtual void CreateDirectory(const Path&, bool isRecursive) = 0;
        virtual void DeleteDirectory(const Path&, bool isRecursive) = 0;
    };

    // use exception to give client code ability to understand why something not working
    class FileSystem2Impl;

    class FileSystem2
    {
    public:
        FileSystem2();
        ~FileSystem2();

        String ReadFileContentAsString(const Path& pathname);
        // open or create stream from mounted pakfile or OS file sysem
        // or wrapper around FILE* or android stream or pakfile stream
        std::unique_ptr<InputStream> OpenFile(const Path&);
        std::unique_ptr<OutputStream> CreateFile(const Path&, bool recreate);
        // works on OS file sysem
        void DeleteFile(const Path&);
        // works on OS file sysem
        void DeleteDirectory(const Path& path, bool isRecursive);
        // works on OS file sysem
        void CreateDirectory(const Path& path, bool isRecursive);
        // works on OS file sysem
        Path GetCurrentWorkingDirectory();

        // write path for save, logs ets. "~doc:/logs/today.txt" -> ~doc: == GetPrefPath() == "C:/Users/l_chayka/Documents"
        Path GetPrefPath();
        // works only for OS file system and using current working directory
        Path GetAbsolutePath(const Path& p);
        // works only for OS file system and using current working directory
        Path GetRelativePath(const Path& p);
        // works only for OS file system
        bool SetCurrentWorkingDirectory(const Path& newWorkingDirectory);

        // test file path if file on FileDevice or OS file system
        bool IsFile(const Path& pathToCheck);
        // test if path is directory 
        bool IsDirectory(const Path& pathToCheck);
        // find path in FileDevice or OS file system
        bool Exist(const Path& Path);
        // copy from FileDevice to OS file system, or from OS to OS
        bool CopyFile(const Path& existingFile, const Path& newFile, bool overwriteExisting);
        // move file from OS to OS
        bool MoveFile(const Path& existingFile, const Path& newFile, bool overwriteExisting);
        // copy directory from FileDevice to OS or from OS to OS
        bool CopyDirectory(const Path& srcDir, const Path& dstDir, bool overwriteExisting);
        // virtualName = {~res:/|~doc:/|~web:/|~pak1:/|~[user_string]:/}
        void Mount(const String& virtualName, std::unique_ptr<FileDevice>);
        Vector<std::unique_ptr<FileDevice>>& GetMountedDevices();
        // works on FileDevice or OS
        uint64 GetFileSize(const Path& path);

        // can be empty, or >1 (if two sdcard present)
        Vector<Path> AndroidGetExternalStoragePath();
        Path AndroidGetInternalStoragePath();
        FileDevice::State AndroidGetExternalStorageState();
    private:
        std::unique_ptr<FileSystem2Impl> impl;
    };
} // end namespace DAVA


