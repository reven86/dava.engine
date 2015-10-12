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

#include "FileSystemModel.h"

#include "FileSystem/Logger.h"
#include "Debug/DVAssert.h"

FileSystemModel::FileSystemModel(std::string const & rootPath, TCheckFile const & checkFileFn_)
    : checkFileFn(checkFileFn_)
{
    IFileSystem * fileSystem = Context::queryInterface<IFileSystem>();
    if (fileSystem == nullptr)
    {
        DAVA::Logger::Error("Can't query IFileSystem interface");
        return;
    }

    if (rootPath.empty() || !fileSystem->exists(rootPath.c_str()))
        return;

    root.reset(new Item(fileSystem->getFileInfo(rootPath.c_str()), *fileSystem, checkFileFn));
}

FileSystemModel::~FileSystemModel()
{
}

IItem * FileSystemModel::item(size_t index, const IItem * parent) const
{
    if (parent == nullptr)
    {
        DVASSERT(index == 0);
        return root.get();
    }
    return static_cast<Item const *>(parent)->getChild(index);
}

ITreeModel::ItemIndex FileSystemModel::index(const IItem * item) const
{
    Item const * i = static_cast<Item const *>(item);
    return i->getIndex();
}

size_t FileSystemModel::size(const IItem * item) const
{
    if (item == nullptr)
        return root != nullptr ? 1 : 0;

    Item const * i = static_cast<Item const *>(item);

    return i->getChildCount();
}

/////////////////////////////////////////////////////////////////////

FileSystemModel::Item::Item(FileInfo const & info, IFileSystem & fileSystem, TCheckFile const & checkFileFn)
    : fileInfo(info)
{
    CollectChildren(fileSystem, checkFileFn);
}

FileSystemModel::Item::Item(FileInfo && info, IFileSystem & fileSystem, TCheckFile const & checkFileFn, Item * parent_, int index_)
    : fileInfo(std::move(info))
    , parent(parent_)
    , index(index_)
{
    CollectChildren(fileSystem, checkFileFn);
}

void FileSystemModel::Item::CollectChildren(IFileSystem & fileSystem, TCheckFile const & checkFileFn)
{
    if (!fileInfo.isDirectory())
        return;

    fileSystem.enumerate(fileInfo.fullPath.c_str(), [this, &fileSystem, &checkFileFn](FileInfo && info)
    {
        if (!info.isDots() && (info.isDirectory() || checkFileFn(info.fullPath)))
            children.emplace_back(new Item(std::move(info), fileSystem, checkFileFn, this, children.size()));
        
        return true;
    });
}

int FileSystemModel::Item::columnCount() const
{
    return 1;
}

const char * FileSystemModel::Item::getDisplayText(int column) const
{
    if (fileInfo.isDirectory())
    {
        size_t pos = fileInfo.fullPath.rfind(FileInfo::kDirectorySeparator, fileInfo.fullPath.size() - 2);
        if (pos == std::string::npos)
            pos = fileInfo.fullPath.rfind(FileInfo::kAltDirectorySeparator, fileInfo.fullPath.size() - 2);

        if (pos != std::string::npos)
            return &fileInfo.fullPath.c_str()[pos + 1];

        return "";
    }
    return fileInfo.name();
}

ThumbnailData FileSystemModel::Item::getThumbnail(int column) const
{
    return nullptr;
}

Variant FileSystemModel::Item::getData(int column, size_t roleId) const
{
    return Variant();
}

bool FileSystemModel::Item::setData(int column, size_t roleId, const Variant & data)
{
    return false;
}

size_t FileSystemModel::Item::getChildCount() const
{
    return children.size();
}

FileSystemModel::Item * FileSystemModel::Item::getChild(size_t index) const
{
    DVASSERT(index < children.size());
    return children[index].get();
}

ITreeModel::ItemIndex FileSystemModel::Item::getIndex() const
{
    return ITreeModel::ItemIndex(index, parent);
}
