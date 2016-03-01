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

#include "FileSystem/FileSystem2.h"
#include "Utils/UTF8Utils.h"

#include <cctype>
#include <fstream>

#ifdef __DAVAENGINE_WINDOWS__
#include <Shlwapi.h>
#include <shellapi.h>
#endif

namespace DAVA
{
    static void ValidatePath(String& pathname)
    {
        if (!pathname.empty())
        {
            std::replace(begin(pathname), end(pathname), '\\', '/');

            if (pathname.find("//") != String::npos)
            {
                throw std::runtime_error("double // inside path");
            }

            if (std::count(begin(pathname), end(pathname), '~') > 1)
            {
                throw std::runtime_error("~ char more then once");
            }
        }
    }
    
    Path::Path()
    {
    }

    Path::Path(const Path& path):pathname(path.pathname)
    {
    }

    Path::Path(Path&& path):pathname(std::move(path.pathname))
    {
    }

    Path::Path(const String& sourcePathUtf8):pathname(sourcePathUtf8)
    {
        ValidatePath(pathname);
    }

    Path::Path(const WideString& src)
    {
        pathname = UTF8Utils::EncodeToUTF8(src);
        ValidatePath(pathname);
    }

    Path::Path(const char* sourcePathUtf8)
    {
        if (sourcePathUtf8 != nullptr)
        {
            pathname = sourcePathUtf8;
            ValidatePath(pathname);
        }
    }

    Path::Path(const wchar_t* sourcePath)
    {
        if (sourcePath != nullptr)
        {
            pathname = UTF8Utils::EncodeToUTF8(WideString(sourcePath));
            ValidatePath(pathname);
        }
    }

    Path::~Path()
    {

    }

    Path& Path::operator=(const Path& p)
    {
        pathname = p.pathname;
        return *this;
    }

    Path& Path::operator=(Path&& p)
    {
        pathname = std::move(p.pathname);
        return *this;
    }

    Path& Path::operator+=(const Path& p)
    {
        if (p.IsAbsolute())
        {
            throw std::runtime_error("can't concatenate absolute path");
        }

        if (p.IsEmpty())
        {
            return *this;
        } 
        
        if (HasFilename())
        {
            pathname += "/";
            pathname += p.pathname;
        
        } else
        {
            pathname += p.pathname;
        }

        return *this;
    }

    Path Path::operator+(const Path& p) const
    {
        Path current(*this);
        current += p;
        return current;
    }

    bool Path::operator==(const Path& path) const
    {
        return pathname == path.pathname;
    }

    bool Path::operator!=(const Path& path) const
    {
        return pathname != path.pathname;
    }

    bool Path::IsEmpty() const
    {
        return pathname.empty();
    }

    bool Path::IsVirtual() const
    {
        return !IsEmpty() && pathname[0] == '~';
    }

    bool Path::IsAbsolute() const
    {
        return !IsEmpty() && HasRootDirectory();
    }

    bool Path::IsRelative() const
    {
        return !IsEmpty() && !HasRootDirectory();
    }

    static bool IsWindowsRoot(const String& pathname)
    {
        return pathname.size() >= 3 && std::isalpha(pathname[0]) && pathname[1] == ':' && pathname[2] == '/';
    }

    bool Path::HasRootDirectory() const
    {
        return !IsEmpty() && (IsVirtual() || pathname[0] == '/' || IsWindowsRoot(pathname));
    }

    bool Path::HasParentPath() const
    {
        size_t numOfSlashes = std::count(begin(pathname), end(pathname), '/');
        bool hasRoot = HasRootDirectory();
        return (hasRoot && numOfSlashes >= 2) || numOfSlashes >= 1;
    }

    bool Path::HasFilename() const
    {
        auto firstSlashPosFromEnd = pathname.rfind('/');
        if (firstSlashPosFromEnd != String::npos)
        {
            return  (firstSlashPosFromEnd + 1) != pathname.size();
        }
        return !IsEmpty();
    }

    bool Path::HasStem() const
    {
        return HasFilename();
    }

    bool Path::HasExtension() const
    {
        auto lastDot = pathname.rfind('.');
        auto slashPresentAfterLastDot = pathname.find('/', lastDot);
        return lastDot != String::npos && slashPresentAfterLastDot == String::npos;
    }

    Path Path::GetRootDirectory() const
    {
        if (!HasRootDirectory())
        {
            // return empty path, may be throw std::runtime_error("no root directory");?
            return Path();
        }

        auto firstSlash = pathname.find('/');
        auto root = pathname.substr(0, firstSlash + 1);
        return root;
    }

    Path Path::GetParentPath() const
    {
        // on absolute path
        // ~res:/folder/file.txt        => ~res:/folder
        // c:/folder/file.txt           => c:/folder
        // /Users/l_chayka/job/file.txt => /Users/l_chayka/job
        // on relative path
        // folder/job/file.txt          => folder/job
        // folder/job                   => folder

        if (!HasParentPath())
        {
            return Path();
        }

        auto lastSlash = pathname.rfind('/');

        return pathname.substr(0, lastSlash);
    }

    Path Path::GetFilename() const
    {
        if (!HasFilename())
        {
            return Path();
        }

        auto lastSlash = pathname.rfind('/');

        if (lastSlash != String::npos)
        {
            return pathname.substr(lastSlash + 1);
        }
        return *this;
    }

    Path Path::GetStem() const
    {
        if (!HasStem())
        {
            return Path();
        }

        Path filename = GetFilename();
        auto lastDot = filename.pathname.rfind('.');
        if (lastDot != String::npos)
        {
            return filename.pathname.substr(0, lastDot);
        }
        return filename;
    }

    Path Path::GetExtension() const
    {
        if (HasExtension())
        {
            return Path();
        }

        auto lastDot = pathname.rfind('.');
        return pathname.substr(lastDot);
    }

    void Path::Clear()
    {
        pathname.clear();
    }

    Path Path::RemoveFilename() const
    {
        if (!HasFilename())
        {
            return Path();
        }

        auto lastSlash = pathname.rfind('/');
        if (lastSlash != String::npos)
        {
            return pathname.substr(0, lastSlash);
        }

        return Path();
    }

    Path Path::ReplaceExtension(const Path& newExtension /*= Path()*/) const
    {
        if (HasExtension())
        {
            String newPath = pathname;
            auto lastDot = newPath.rfind('.');
            newPath.replace(lastDot, newExtension.pathname.size() - lastDot, newExtension.GetExtension().pathname);
            return newPath;
        }
        return *this + newExtension;
    }

    const String& Path::ToStringUtf8() const
    {
        return pathname;
    }

    const WideString& Path::ToWideString() const
    {
        UTF8Utils::EncodeToWideString(reinterpret_cast<const uint8*>(pathname.c_str()), pathname.size(), copyPathname);
        return copyPathname;
    }

    size_t Path::Hash() const
    {
        return std::hash<String>()(pathname);
    }


    InputStream::~InputStream()
    {
    }

    OutputStream::~OutputStream()
    {
    }

    FileDevice::~FileDevice()
    {
    }

    class OSInputStream : public InputStream
    {
    public:
        explicit OSInputStream(const Path& p);
        void Read(void* data, uint64 size) override;
        void Seek(uint64 position) override;
        uint64 Tell() override;
        uint64 GetSize() override;
    private:
        std::ifstream f;
    };

    class OSOutputStream : public OutputStream
    {
    public:
        explicit OSOutputStream(const Path& p, bool recreate);
        void Write(const void* data, uint64 size) override;
        void Seek(uint64 position) override;
        uint64 Tell() override;
        uint64 GetSize() override;
    private:
        std::ofstream f;
    };

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
        std::unique_ptr<InputStream> OpenFile(const Path&) override;
        std::unique_ptr<OutputStream> CreateFile(const Path&, bool recreate) override;
        void DeleteFile(const Path&) override;
        void CreateDirectory(const Path&, bool errorIfExist = false) override;
        void DeleteDirectory(const Path&, bool withContent = false) override;
    private:
        Path base;
        int32 priority = 0;
    };

    OSInputStream::OSInputStream(const Path& p)
    {
        f.open(p.ToStringUtf8(), std::ifstream::binary | std::ifstream::in);
        if (!f)
        {
            const char* err = std::strerror(errno);
            std::string errMsg("can't open: ");
            errMsg += p.ToStringUtf8();
            errMsg += " course: ";
            errMsg += err;
            throw std::runtime_error(errMsg);
        }
    }

    void OSInputStream::Read(void* data, uint64 size)
    {
        f.read(static_cast<char*>(data), static_cast<std::streamsize>(size));
    }

    void OSInputStream::Seek(uint64 position)
    {
        f.seekg(static_cast<std::streamoff>(position), f.beg);
    }

    uint64 OSInputStream::Tell()
    {
        return static_cast<uint64>(f.tellg());
    }

    uint64 OSInputStream::GetSize()
    {
        uint64 currPos = Tell();
        f.seekg(0, f.end);
        uint64 result = static_cast<uint64>(f.tellg());
        Seek(currPos);
        return result;
    }

    OSOutputStream::OSOutputStream(const Path& p, bool recreate)
    {
        std::ios_base::openmode mode;
        if (recreate)
        {
            mode = std::ifstream::binary | std::ifstream::out | std::ifstream::trunc;
        } else
        {
            mode = std::ifstream::binary | std::ifstream::out;
        }

        f.open(p.ToStringUtf8(), mode);
        if (!f)
        {
            const char* err = std::strerror(errno);
            std::string errMsg("can't open: ");
            errMsg += p.ToStringUtf8();
            errMsg += " course: ";
            errMsg += err;
            throw std::runtime_error(errMsg);
        }
    }

    void OSOutputStream::Write(const void* data, uint64 size)
    {
        f.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
    }

    void OSOutputStream::Seek(uint64 position)
    {
        f.seekp(static_cast<std::streamoff>(position));
    }

    uint64 OSOutputStream::Tell()
    {
        return static_cast<uint64>(f.tellp());
    }

    uint64 OSOutputStream::GetSize()
    {
        uint64 currPos = Tell();
        f.seekp(0, f.end);
        uint64 result = static_cast<uint64>(f.tellp());
        Seek(currPos);
        return result;
    }

    OSFileDevice::OSFileDevice(const Path& base_ /*= Path()*/, int32 priority_ /*= 0*/):
        base(base_)
        , priority(priority_)
    {
    }

    int32 OSFileDevice::GetPriority()
    {
        return priority;
    }


    FileDevice::State OSFileDevice::GetState()
    {
        return State::READ_WRITE;
    }


    bool OSFileDevice::Exist(const Path& p, uint64* fileSize /*= nullptr*/)
    {
        Path absolute = p.IsAbsolute() ? p : base + p;

#ifdef __DAVAENGINE_WINDOWS__
        BOOL result = ::PathFileExistsW(absolute.ToWideString().c_str());
        return result == TRUE;
#endif
    }


    bool OSFileDevice::IsFile(const Path& p)
    {
        Path absolute = p.IsAbsolute() ? p : base + p;

#ifdef __DAVAENGINE_WINDOWS__
        DWORD fileAttibures = ::GetFileAttributesW(absolute.ToWideString().c_str());
        if (fileAttibures != INVALID_FILE_ATTRIBUTES)
        {
            return (fileAttibures & FILE_ATTRIBUTE_DIRECTORY) == 0;
        }
        return false;
#endif
    }


    bool OSFileDevice::IsDirectory(const Path& p)
    {
        Path absolute = p.IsAbsolute() ? p : base + p;

#ifdef __DAVAENGINE_WINDOWS__
        DWORD fileAttibures = ::GetFileAttributesW(absolute.ToWideString().c_str());
        if (fileAttibures != INVALID_FILE_ATTRIBUTES)
        {
            return (fileAttibures & FILE_ATTRIBUTE_DIRECTORY) != 0;
        }
        return false;
#endif
    }


    Vector<Path> OSFileDevice::EnumerateFiles(const Path& basePath /*= Path()*/)
    {
        Path absolute = basePath.IsAbsolute() ? basePath : base + basePath;

#ifdef __DAVAENGINE_WINDOWS__
        Vector<Path> outVec;
        WIN32_FIND_DATAW findData;
        HANDLE findHandle = ::FindFirstFileW(absolute.ToWideString().c_str(), &findData);
        if (findHandle != INVALID_HANDLE_VALUE)
        {
            do
            {
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) == 0)
                {
                    outVec.emplace_back(findData.cFileName);
                }
            } while (FindNextFile(findHandle, &findData) != 0);
        }
        return outVec;
#endif
    }


    std::unique_ptr<InputStream> OSFileDevice::OpenFile(const Path& p)
    {
        Path absolute = p.IsAbsolute() ? p : base + p;
        std::unique_ptr<InputStream> result(new OSInputStream(absolute));
        return result;
    }


    std::unique_ptr<OutputStream> OSFileDevice::CreateFile(const Path& p, bool recreate)
    {
        Path absolute = p.IsAbsolute() ? p : base + p;
        std::unique_ptr<OutputStream> result(new OSOutputStream(absolute, recreate));
        return result;
    }

    void OSFileDevice::DeleteFile(const Path& p)
    {
        Path absolute = p.IsAbsolute() ? p : base + p;

        int result = std::remove(absolute.ToStringUtf8().c_str());
        if (result != 0)
        {
            const char* err = strerror(errno);
            String msg("can't remove file: ");
            msg += p.ToStringUtf8();
            msg += " cause: ";
            msg += err;
            throw std::runtime_error(msg);
        }
    }


    void OSFileDevice::CreateDirectory(const Path& p, bool errorIfExist)
    {
#ifdef __DAVAENGINE_WINDOWS__
        Path absolute = p.IsAbsolute() ? p : base + p;
        int result = ::SHCreateDirectoryExW(nullptr, absolute.ToWideString().c_str(), nullptr);
        String err;
        switch(result)
        {
        case ERROR_SUCCESS:
            break;
        case ERROR_BAD_PATHNAME:
            err = "ERROR_BAD_PATHNAME";
            break;
        case ERROR_FILENAME_EXCED_RANGE:
            err = "ERROR_FILENAME_EXCED_RANGE";
            break;
        case ERROR_PATH_NOT_FOUND:
            err = "ERROR_PATH_NOT_FOUND";
            break;
        case ERROR_FILE_EXISTS: // no break
        case ERROR_ALREADY_EXISTS:
            if (errorIfExist)
            {
                err = "ERROR_FILE_EXISTS";
            }
            break;
        default:
            err += " system error code: " + std::to_string(result);
            break;
        };
        if (!err.empty())
        {
            String msg("can't create directory: ");
            msg += p.ToStringUtf8();
            msg += " cause: ";
            msg += err;
            throw std::runtime_error(msg);
        }
#else
#error "implement it"
#endif
    }


    void OSFileDevice::DeleteDirectory(const Path& p, bool withContent)
    {
        Path absolute = p.IsAbsolute() ? p : base + p;

#ifdef __DAVAENGINE_WINDOWS__
        if (withContent)
        {
            // https://msdn.microsoft.com/en-us/library/bb759795%28v=vs.85%29.aspx
            WideString doubleZerroStr(absolute.ToWideString());
            doubleZerroStr.push_back(L'\0');
            doubleZerroStr.push_back(L'\0');
            SHFILEOPSTRUCTW fileOp = {
                nullptr,
                FO_DELETE,
                doubleZerroStr.c_str(),
                L"",
                FOF_NOCONFIRMATION | FOF_NOERRORUI | FOF_SILENT,
                false,
                nullptr,
                L""
            };
            int result = ::SHFileOperationW(&fileOp);
            if (result != 0)
            {
                String msg("can't delete directory: ");
                msg += p.ToStringUtf8();
                throw std::runtime_error(msg);
            }
        } else
        {
            BOOL result = RemoveDirectoryW(absolute.ToWideString().c_str());
            if (result == FALSE)
            {
                uint32 sysErrorCode = GetLastError();
                String msg("can't delete directory: ");
                msg += p.ToStringUtf8();
                msg += " cause: system error code ";
                msg += std::to_string(sysErrorCode);
                throw std::runtime_error(msg);
            }
        }
#endif
    }

    class FileSystem2Impl
    {
    public:
        std::shared_ptr<FileDevice> FindDeviceByPath(const Path& p)
        {
            std::shared_ptr<FileDevice> result;

            auto it = std::find_if(begin(devicesSorted), end(devicesSorted), [&p](std::shared_ptr<FileDevice>& device)
            {
                return device->Exist(p);
            });

            if (it != end(devicesSorted))
            {
                result = *it;
            }

            return result;
        }
        // use ONLY std::stable_sort to preserv adding order
        Vector<std::shared_ptr<FileDevice>> devicesSorted;
        UnorderedMap<Path, Vector<std::shared_ptr<FileDevice>>> devicesByRootName;
    };

    FileSystem2::FileSystem2()
    {
        impl.reset(new FileSystem2Impl());
        Mount("", std::make_shared<OSFileDevice>());
    }

    FileSystem2::~FileSystem2()
    {
    }

    String FileSystem2::ReadFileContentAsString(const Path& pathname) const
    {
        String output;

        auto devicePtr = impl->FindDeviceByPath(pathname);

        if (devicePtr)
        {
            auto file = devicePtr->OpenFile(pathname);
            auto size = file->GetSize();
            output.resize(static_cast<std::size_t>(size));
            file->Read(&output[0], size);
            return output;
        }

        String msg("can't read file: ");
        msg += pathname.ToStringUtf8();
        throw std::runtime_error(msg);
    }


    std::unique_ptr<InputStream> FileSystem2::OpenFile(const Path& p) const
    {
        auto devicePtr = impl->FindDeviceByPath(p);

        if (devicePtr)
        {
            return devicePtr->OpenFile(p);
        }

        String msg("can't read file: ");
        msg += p.ToStringUtf8();
        throw std::runtime_error(msg);
    }


    std::unique_ptr<OutputStream> FileSystem2::CreateFile(const Path& p, bool recreate)
    {
        std::unique_ptr<OutputStream> result;
        auto devicePtr = impl->FindDeviceByPath(p);

        if (devicePtr)
        {
            return devicePtr->CreateFile(p, recreate);
        }

        auto& devices = impl->devicesByRootName[p.GetRootDirectory()];
        if (!devices.empty())
        {
            for(auto& device : devices)
            {
                auto state = device->GetState();
                if (FileDevice::State::WRITE_ONLY == state ||
                    FileDevice::State::READ_WRITE == state)
                {
                    auto file = device->CreateFile(p, recreate);
                }
            }
        }
    }

} // end namespace DAVA