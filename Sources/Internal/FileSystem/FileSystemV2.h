
// 1. all path utf8 strings
// 2. path not change filesystem, only path object
// examples:
// ~res:/folder1/file1.txt
// ~doc:/folder2/file2.txt
// c:/mingw32/bin/gcc.exe
// /Users/l_chayka/job/dava.framework

// Grammar for portable generic path strings (from BOOST docs)
// The grammar is specified in extended BNF, with terminal symbols in quotes:
//     path ::= [root] [relative-path]  // an empty path is valid
//     root ::= [root-name] [root-directory]
//     root-directory ::= separator
//     relative-path ::= path-element { separator path-element } [separator]
//     path-element ::= name | parent-directory | directory-placeholder
//     name ::= char { char }
//     directory-placeholder ::= "."
//     parent-directory ::= ".."
//     separator ::= "/"  // an implementation may define additional separators

namespace DAVA
{

class Path_impl;
class Path final
{
public:
    Path();
    Path(const Path& path);
    Path(Path&& path);
    Path(const String& sourcePathUtf8);
    Path(const WideString& src)
    Path(const char* sourcePathUtf8);
    ~Path();

    Path& operator=(const Path&);
    Path& operator=(Path&&);
    Path operator+=(const Path&);
    Path operator+(const Path&)

    bool operator==(const Path& path) const;
    bool operator!=(const Path& path) const;

    bool IsEmpty() const;
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
private:
	Path_impl* impl = nullptr;
};

// exceptions? or -1 == error, >= 0 - size of last read
class InputStream
{
public:
	virtual ~InputStream();
	virtual int64 Read(void* data, int64 size) = 0;
	virtual int64 Seek(int64 position) = 0;
	virtual int64 Tell() = 0;
	virtual int64 GetSize() = 0;
};

class OutputStream
{
public:
	virtual ~OutputStream();
	virtual int64 Write(void* data, int64 size) = 0;
	virtual int64 Seek(int64 position) = 0;
	virtual int64 Tell() = 0;
	virtual int64 GetSize() = 0;
};

// 1. thread safe for pakfile too
// 2. use exception to give client code ability to understend why something not working
// 3. do we need priority for pakfiles and os file system
// 4. TODO make shure function names not collide with windows.h and coloa framework ets...

namespace FileSystemV2
{
    // open or create stream from mounted pakfile or OS filesysem
    // internal use memory mepped files to read as fast as OS can
    // or wrapper around FILE* or android stream or pakfile stream
    std::unique_ptr<InputStream> OpenFile(const Path&);
    std::unique_ptr<OutputStream> CreateFile(const Path&);

    bool DeleteFile(const Path&);
    bool DeleteDirectory(const Path& path, bool isRecursive);
    void DeleteDirectoryFiles(const Path& path, bool isRecursive);

    bool CreateDirectory(const Path& path, bool isRecursive);

    Path GetCurrentWorkingDirectory();
    Path GetExecutableDirectory(); // do we need it?

	Path GetUserDocumentsPath();
	Path GetPublicDocumentsPath();
	Path GetUserHomePath();
	// resources path base dir used for "~res:/folder/image.png" -> ~res: == GetAppDataPath() == "C:/Users/l_chayka/game/Data"
	Path GetAppDataPath();
	// write path for save, logs ets. "~doc:/logs/today.txt" -> ~doc: == GetPrefPath() == "C:/Users/l_chayka/Documents"
	Path GetPrefPath();

	Path GetAbsolutePath(const Path& p, const Path& base = GetCurrentWorkingDirectory());
	Path GetRelativePath(const Path& p, const Path& base = GetCurrentWorkingDirectory());

    bool SetCurrentWorkingDirectory(const Path& newWorkingDirectory);
    bool SetCurrentDocumentsDirectory(const Path& newDocDirectory);
    
    bool IsFile(const Path& pathToCheck);
    bool IsDirectory(const Path& pathToCheck);
    bool IsExist(const Path& Path);

    bool CopyFile(const Path& existingFile, const Path& newFile, bool overwriteExisting);
    bool MoveFile(const Path& existingFile, const Path& newFile, bool overwriteExisting);
    bool CopyDirectory(const Path& srcDir, const Path& dstDir, bool overwriteExisting);

    Vector<uint8> ReadFileContentAsVector(const Path& pathname);
    String ReadFileContentsAsString(const Path& pathname);

    // priority != 0 - OS file system priority == 0, <0 after OS fs, >0 before OS
    void MountReadOnlyVirtualFS(const Path& pakfileName, const Path& mountTo, int32 priority);
    // works on virtual FS too
    bool GetFileSize(const Path& path, size_t& size);

    // can be empty
    Path AndroidGetExternalStoragePath();
    Path AndroidGetInternalStoragePath();
    enum class StorageState
    {
        UNAVAILABLE,
        READ_ONLY,
        WRITE_ONLY,
        READ_WRITE
    };
    StorageState AndroidGetExternalStorageState();
} // end namespace FileSystemV2

} // end namespace DAVA