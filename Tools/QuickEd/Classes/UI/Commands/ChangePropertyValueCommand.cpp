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


#include "ChangePropertyValueCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/AbstractProperty.h"

ChangePropertyValueCommand::ChangePropertyValueCommand(PackageNode* root_, ControlNode* node_, const DAVA::Vector<std::pair<AbstractProperty*, DAVA::VariantType>>& properties_, QUndoCommand* parent /*= nullptr*/)
    : QUndoCommand(parent)
    , root(SafeRetain(root_))
    , node(SafeRetain(node_))
{
    for (auto property : properties_)
    {
        properties.emplace_back(SafeRetain(property.first));
        newValues.emplace_back(property.second);
    }
    init();
}

ChangePropertyValueCommand::ChangePropertyValueCommand(PackageNode* root_, ControlNode* node_, AbstractProperty* prop, const DAVA::VariantType& newVal, QUndoCommand* parent /*= nullptr*/)
    : QUndoCommand(parent)
    , root(SafeRetain(root_))
    , node(SafeRetain(node_))
{
    properties.emplace_back(SafeRetain(prop));
    newValues.emplace_back(newVal);
    init();
}

ChangePropertyValueCommand::ChangePropertyValueCommand(PackageNode* root_, ControlNode* node_, AbstractProperty* prop, QUndoCommand* parent /*= nullptr*/)
    : QUndoCommand(parent)
    , root(SafeRetain(root_))
    , node(SafeRetain(node_))
{
    properties.push_back(SafeRetain(prop));
    newValues.push_back(DAVA::VariantType());
    init();
}

void ChangePropertyValueCommand::init()
{
    QString text = "changed:";
    std::hash<void*> ptrHash;
    for (AbstractProperty* property : properties)
    {
        hash = ptrHash(property) ^ (hash << 1);
        if (property->IsOverriddenLocally())
        {
            oldValues.push_back(property->GetValue());
        }
        else
        {
            oldValues.push_back(DAVA::VariantType());
        }
        text += QString(" %1").arg(property->GetName().c_str());
    }
    setText(text);
    DVASSERT(newValues.size() == oldValues.size() && newValues.size() == properties.size());
}

ChangePropertyValueCommand::~ChangePropertyValueCommand()
{
    SafeRelease(root);
    SafeRelease(node);
    for (AbstractProperty* property : properties)
    {
        SafeRelease(property);
    }
}

void ChangePropertyValueCommand::redo()
{
    size_t size = properties.size();
    for (size_t i = 0; i < size; ++i)
    {
        const auto newValue = newValues.at(i);
        auto property = properties.at(i);
        if (newValue.GetType() == DAVA::VariantType::TYPE_NONE)
            root->ResetControlProperty(node, property);
        else
            root->SetControlProperty(node, property, newValue);
    }
}

void ChangePropertyValueCommand::undo()
{
    size_t size = properties.size();
    for (size_t i = 0; i < size; ++i)
    {
        const auto oldValue = oldValues.at(i);
        auto property = properties.at(i);
        if (oldValue.GetType() == DAVA::VariantType::TYPE_NONE)
            root->ResetControlProperty(node, property);
        else
            root->SetControlProperty(node, property, oldValue);
    }
}

int ChangePropertyValueCommand::id() const
{
    return static_cast<int>(hash);
}

bool ChangePropertyValueCommand::mergeWith(const QUndoCommand* other)
{
    if (other->id() != id())
        return false;
    const ChangePropertyValueCommand* otherCommand = static_cast<const ChangePropertyValueCommand*>(other);
    newValues = otherCommand->newValues;
    return true;
}