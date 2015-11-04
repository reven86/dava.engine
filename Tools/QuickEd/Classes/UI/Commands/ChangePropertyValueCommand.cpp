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

using namespace DAVA;

ChangePropertyValueCommand::ChangePropertyValueCommand(PackageNode* root_, const Vector<std::tuple<ControlNode*, AbstractProperty*, VariantType>>& properties_, size_t hash_, QUndoCommand* parent /*= nullptr*/)
    : QUndoCommand(parent)
    , root(SafeRetain(root_))
    , hash(hash_)
{
    for (auto property : properties_)
    {
        AbstractProperty* prop = std::get<PROPERTY>(property);
        changedProperties.emplace_back(
        std::get<NODE>(property),
        prop,
        std::get<NEW_VALUE>(property),
        GetValueFromProperty(prop));
    }
    Init();
}

ChangePropertyValueCommand::ChangePropertyValueCommand(PackageNode* root_, ControlNode* node, AbstractProperty* prop, const VariantType& newVal, size_t hash_, QUndoCommand* parent /*= nullptr*/)
    : QUndoCommand(parent)
    , root(SafeRetain(root_))
    , hash(hash_)
{
    changedProperties.emplace_back(
    node, prop, newVal, GetValueFromProperty(prop));
    Init();
}

ChangePropertyValueCommand::ChangePropertyValueCommand(PackageNode* root_, ControlNode* node, AbstractProperty* prop, QUndoCommand* parent /*= nullptr*/)
    : QUndoCommand(parent)
    , root(SafeRetain(root_))
{
    changedProperties.emplace_back(
    node, prop, VariantType(), GetValueFromProperty(prop));
    Init();
}

void ChangePropertyValueCommand::Init()
{
    QString text = "changed:";
    std::hash<void*> ptrHash;
    for (const auto& propertyValue : changedProperties)
    {
        AbstractProperty* prop = std::get<PROPERTY>(propertyValue);
        hash = ptrHash(prop) ^ (hash << 1);
        text += QString(" %1").arg(prop->GetName().c_str());
    }
    setText(text);
}

ChangePropertyValueCommand::~ChangePropertyValueCommand()
{
    SafeRelease(root);
}

void ChangePropertyValueCommand::redo()
{
    for (const auto& propertyValue : changedProperties)
    {
        ControlNode* node = std::get<NODE>(propertyValue);
        const VariantType& newValue = std::get<NEW_VALUE>(propertyValue);
        AbstractProperty* property = std::get<PROPERTY>(propertyValue);

        if (newValue.GetType() == VariantType::TYPE_NONE)
        {
            root->ResetControlProperty(node, property);
        }
        else
        {
            root->SetControlProperty(node, property, newValue);
        }
    }
}

void ChangePropertyValueCommand::undo()
{
    for (const auto& propertyValue : changedProperties)
    {
        ControlNode* node = std::get<NODE>(propertyValue);
        const VariantType& oldValue = std::get<OLD_VALUE>(propertyValue);
        AbstractProperty* property = std::get<PROPERTY>(propertyValue);

        if (oldValue.GetType() == VariantType::TYPE_NONE)
        {
            root->ResetControlProperty(node, property);
        }
        else
        {
            root->SetControlProperty(node, property, oldValue);
        }
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
    for (int i = 0, k = changedProperties.size(); i < k; ++i)
    {
        std::get<NEW_VALUE>(changedProperties.at(i)) = std::get<NEW_VALUE>(otherCommand->changedProperties.at(i));
    }
    return true;
}

VariantType ChangePropertyValueCommand::GetValueFromProperty(AbstractProperty* property)
{
    return property->IsOverriddenLocally() ? property->GetValue() : VariantType();
}
