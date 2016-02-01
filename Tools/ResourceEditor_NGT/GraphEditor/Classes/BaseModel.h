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

#ifndef __GRAPHEDITOR_BASEMODEL_H__
#define __GRAPHEDITOR_BASEMODEL_H__

#include <core_data_model/i_list_model.hpp>
#include <core_data_model/i_item.hpp>
#include <core_data_model/i_item_role.hpp>

template <typename T>
class BaseModel : public IListModel
{
    class BaseItem;
    using TCollection = std::vector<BaseItem*>;
    using TItemIter = typename TCollection::iterator;
    using TConstItemIter = typename TCollection::const_iterator;

    class BaseItem : public IItem
    {
    public:
        BaseItem(ObjectHandleT<T> object_)
            : object(object_)
        {
        }

        const char* getDisplayText(int /*column*/) const override
        {
            return nullptr;
        }

        ThumbnailData getThumbnail(int /*column*/) const override
        {
            return nullptr;
        }

        Variant getData(int /*column*/, size_t roleId) const override
        {
            if (ValueRole::roleId_ == roleId)
                return ObjectHandle(object);

            return Variant();
        }

        bool setData(int /*column*/, size_t /*roleId*/, const Variant& /*data*/) override
        {
            return false;
        }

        T* GetObject()
        {
            return object.get();
        }

    private:
        ObjectHandleT<T> object;
    };

public:
    BaseModel() = default;

    BaseModel(std::vector<ObjectHandleT<T>>&& object)
    {
        for (ObjectHandleT<T> const& item : object)
            items.push_back(new BaseItem(item));
    }

    void AddItem(ObjectHandleT<T> const& object)
    {
        size_t index = items.size();
        notifyPreItemsInserted(nullptr, index, 1);
        items.push_back(new BaseItem(object));
        notifyPostItemsInserted(nullptr, index, 1);
    }

    void RemoveItem(ObjectHandleT<T> const& object)
    {
        TItemIter iter = std::find_if(items.begin(), items.end(), [&object](BaseItem* item) {
            return item->GetObject() == object.get();
        });

        if (iter == items.end())
        {
            return;
        }

        size_t index = std::distance(items.begin(), iter);
        notifyPreItemsRemoved(nullptr, index, 1);
        items.erase(iter);
        notifyPostItemsRemoved(nullptr, index, 1);
    }

    ~BaseModel()
    {
        for_each(items.begin(), items.end(), [](BaseItem* item) { delete item; });
        items.clear();
    }

    IItem* item(size_t index) const override
    {
        assert(index < items.size());
        return items[index];
    }

    size_t index(const IItem* item) const override
    {
        TConstItemIter iter = std::find(items.begin(), items.end(), item);
        assert(iter != items.end());
        return std::distance(items.begin(), iter);
    }

    bool empty() const override
    {
        return items.empty();
    }

    size_t size() const override
    {
        return items.size();
    }

    int columnCount() const override
    {
        return 1;
    }

protected:
    TCollection items;
};

#endif // __GRAPHEDITOR_BASEMODEL_H__
