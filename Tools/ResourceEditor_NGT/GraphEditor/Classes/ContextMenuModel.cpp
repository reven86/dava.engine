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

#include "ContextMenuModel.h"

#include <core_data_model/i_item_role.hpp>
#include <core_data_model/i_item.hpp>
#include <core_qt_common/helpers/qt_helpers.hpp>

#include <memory>
#include <algorithm>

namespace
{
class ActionItem : public IItem
{
public:
    ActionItem(ObjectHandle action_)
        : action(std::move(action_))
    {
    }

    int columnCount() const override
    {
        return 1;
    }
    const char* getDisplayText(int column) const override
    {
        return nullptr;
    }
    ThumbnailData getThumbnail(int column) const override
    {
        return nullptr;
    }

    Variant getData(int column, size_t roleId) const override
    {
        if (ValueRole::roleId_ == roleId)
            return action;

        return Variant();
    }

    bool setData(int column, size_t roleId, const Variant& data) override
    {
        return false;
    }

private:
    ObjectHandle action;
};
} // namespace

ContextMenuModel::ContextMenuModel(std::vector<ObjectHandle>&& actions)
{
    items.reserve(actions.size());
    for (size_t i = 0; i < actions.size(); ++i)
        items.push_back(new ActionItem(actions[i]));

    actions.clear();
}

ContextMenuModel::~ContextMenuModel()
{
    for_each(items.begin(), items.end(), [](IItem* item) { delete item; });
    items.clear();
}

IItem* ContextMenuModel::item(size_t index) const
{
    return items[index];
}

size_t ContextMenuModel::index(const IItem* item) const
{
    auto iter = std::find(items.begin(), items.end(), item);
    assert(iter != items.end());
    return std::distance(items.begin(), iter);
}

bool ContextMenuModel::empty() const
{
    return items.empty();
}

size_t ContextMenuModel::size() const
{
    return items.size();
}
