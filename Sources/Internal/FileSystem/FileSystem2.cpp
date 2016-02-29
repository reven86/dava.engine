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

    WideString Path::ToWideString() const
    {
        return UTF8Utils::EncodeToWideString(pathname);
    }

    size_t Path::Hash() const
    {
        return std::hash<String>()(pathname);
    }

} // end namespace DAVA