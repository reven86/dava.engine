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

#ifndef PLG_HELLO_WIDGET_FILESYSTEMMODEL_H
#define PLG_HELLO_WIDGET_FILESYSTEMMODEL_H

#include "core_data_model/i_tree_model.hpp"
#include "core_serialization/interfaces/i_file_system.hpp"

#include <functional>

class FileSystemModel : public ITreeModel
{
public:
    using TCheckFile = std::function < bool(std::string const &) > ;
    explicit FileSystemModel(std::string const & rootPath, TCheckFile const & checkFileFn);
    ~FileSystemModel();

    IItem * item(size_t index, const IItem * parent) const override;
    ItemIndex index(const IItem * item) const override;
    size_t size(const IItem * item) const override;

    class Item : public IItem
    {
    public:
        using TCheckFile = FileSystemModel::TCheckFile;

        Item(FileInfo const & info, IFileSystem & fileSystem, TCheckFile const & checkFileFn);

        int columnCount() const override;
        const char * getDisplayText(int column) const override;
        ThumbnailData getThumbnail(int column) const override;
        Variant getData(int column, size_t roleId) const override;
        bool setData(int column, size_t roleId, const Variant & data) override;

        std::string const & getFilePath() const { return fileInfo.fullPath; }

        size_t getChildCount() const;
        Item * getChild(size_t index) const;
        ITreeModel::ItemIndex getIndex() const;

    private:
        Item(FileInfo && info, IFileSystem & fileSystem, TCheckFile const & checkFileFn, Item * parent, int index);
        void CollectChildren(IFileSystem & fileSystem, TCheckFile const & checkFileFn);

    private:
        FileInfo fileInfo;
        std::vector<std::unique_ptr<Item> > children;
        Item * parent = nullptr;
        int index = 0;
    };

private:
    std::unique_ptr<Item> root;
    TCheckFile checkFileFn;
};

#endif // PLG_HELLO_WIDGET_FILESYSTEMMODEL_H
