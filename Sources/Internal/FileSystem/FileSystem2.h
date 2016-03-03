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
    /// 2. no changes for file system, only Path object
    /// examples:
    ///     ~res:/folder1/file1.txt
    ///     ~doc:/folder2/file2.txt
    ///     c:/mingw32/bin/gcc.exe
    ///     /Users/l_chayka/job/dava.framework
    ///     Data/files/config.yaml
    /// Grammar for portable generic path strings (from BOOST docs with modifications)
    /// The grammar is specified in extended BNF, with terminal symbols in quotes:
    ///     path ::= {[virt] | [root]} [relative-path]  // an empty path is valid
    ///     virt  ::= ~res:/ | ~doc:/
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
        Path(const wchar_t* sourcePath);
        ~Path();

        Path& operator=(const Path&);
        Path& operator=(Path&&);
        Path& operator+=(const Path&);
        Path operator+(const Path&) const;

        bool operator==(const Path& path) const;
        bool operator!=(const Path& path) const;
        bool operator<(const Path& path) const;

        bool IsEmpty() const;
        bool IsVirtual() const;
        bool IsAbsolute() const;
        bool IsRelative() const;

        bool HasRootDirectory() const;
        bool HasParentPath() const;
        bool HasFilename() const;
        bool HasStem() const;
        bool HasExtension() const;

        Path GetRootDirectory() const; // C:/ or ~res:/ or / (Unix)
        Path GetParentPath() const;
        Path GetFilename() const;
        Path GetStem() const;
        Path GetExtension() const;

        void Clear();
        Path RemoveFilename() const;
        Path ReplaceExtension(const Path& newExtension = Path()) const;

        const String& ToStringUtf8() const;
        const WideString& ToWideString() const;
        size_t Hash() const;
    private:
        String pathname;
        mutable WideString copyPathname;
    };

    // exceptions in case of error
    class InputStream
    {
    public:
        virtual ~InputStream();
        virtual void Read(void* data, uint64 size) = 0;
        virtual void Seek(uint64 position) = 0;
        virtual uint64 Tell() = 0;
        virtual uint64 GetSize() = 0;
    };

    class OutputStream
    {
    public:
        virtual ~OutputStream();
        virtual void Write(const void* data, uint64 size) = 0;
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
        virtual Vector<Path> EnumerateFiles(const Path& base = Path()) = 0;
        virtual std::unique_ptr<InputStream> Open(const Path&) = 0;
        virtual std::unique_ptr<OutputStream> Create(const Path&, bool recreate) = 0;
        virtual void RemoveFile(const Path&) = 0;
        virtual void MakeDirectory(const Path&, bool errorIfExist = false) = 0;
        virtual void DeleteDirectory(const Path&, bool withContent = false) = 0;
    };

    // general OS implementation
    class OSFileDevice : public FileDevice
    {
    public:
        explicit OSFileDevice(const Path& base = Path(), int32 priority = 0);

        int32 GetPriority() override;
        State GetState() override;
        bool Exist(const Path&, uint64* fileSize = nullptr) override;
        bool IsFile(const Path&) override;
        bool IsDirectory(const Path&) override;
        Vector<Path> EnumerateFiles(const Path& base = Path()) override;
        std::unique_ptr<InputStream> Open(const Path&) override;
        std::unique_ptr<OutputStream> Create(const Path&, bool recreate) override;
        void RemoveFile(const Path&) override;
        void MakeDirectory(const Path&, bool errorIfExist = false) override;
        void DeleteDirectory(const Path&, bool withContent = false) override;
    private:
        Path base;
        int32 priority = 0;
    };

    // use exception to give client code ability to understand why something not working
    class FileSystem2Impl;

    class FileSystem2
    {
    public:
        FileSystem2();
        ~FileSystem2();

        String ReadFileContentAsString(const Path& pathname) const;
        // open or create stream from mounted pakfile or OS file system
        // or wrapper around std::fstream or android stream or pakfile stream
        std::unique_ptr<InputStream> Open(const Path&) const;
        std::unique_ptr<OutputStream> Create(const Path&, bool recreate) const;
        // works on OS file system
        void RemoveFile(const Path&) const;
        // works on OS file system
        void DeleteDirectory(const Path& path, bool withContent = false) const;
        // works on OS file system
        void MakeDirectory(const Path& path, bool errorIfExist = false) const;
        // works on OS file system
        static Path GetCurrentWorkingDirectory();

        // write path for save, logs ets. "~doc:/logs/today.txt" -> ~doc: == GetPrefPath() == "C:/Users/l_chayka/Documents"
        static Path GetPrefPath();
        // works only for OS file system and using current working directory
        static Path GetAbsolutePath(const Path& p);
        // works only for OS file system and using current working directory
        static Path GetRelativePath(const Path& p);
        static Path GetRelativePath(const Path& file, const Path& relativeDirectory);
        // works only for OS file system
        static void SetCurrentWorkingDirectory(const Path& newWorkingDirectory);

        // test file path if file on FileDevice or OS file system
        bool IsFile(const Path& pathToCheck) const;
        // test if path is directory 
        bool IsDirectory(const Path& pathToCheck) const;
        // find path in FileDevice or OS file system
        bool Exist(const Path& Path) const;
        // copy from OS to OS
        void Copy(const Path& existingFile, const Path& newFile, bool overwriteExisting) const;
        // move file from OS to OS
        void RenameFile(const Path& existingFile, const Path& newFile, bool overwriteExisting) const;
        // copy directory from OS to OS
        void CopyDirectory(const Path& srcDir, const Path& dstDir, bool overwriteExisting) const;
        // virtualName = {~res:/|~doc:/|~web:/|~pak1:/|~[user_string]:/}
        void Mount(const String& virtualName, std::shared_ptr<FileDevice>);
        const Vector<std::shared_ptr<FileDevice>>& GetMountedDevices() const;
        // works on FileDevice or OS
        uint64 GetFileSize(const Path& path) const;

        // can be empty, or >1 (if two sdcard present)
        Vector<Path> AndroidGetExternalStoragePath() const;
        Path AndroidGetInternalStoragePath() const;
        FileDevice::State AndroidGetExternalStorageState() const;
    private:
        std::unique_ptr<FileSystem2Impl> impl;
    };
} // end namespace DAVA

namespace std {
    template <> struct hash<DAVA::Path>
    {
        size_t operator()(const DAVA::Path& p) const
        {
            return p.Hash();
        }
    };
}
