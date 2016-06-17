#include "ChangePropertyValueCommand.h"

#include "Model/PackageHierarchy/PackageNode.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "Model/ControlProperties/AbstractProperty.h"

#include <QString>
using namespace DAVA;

ChangePropertyValueCommand::ChangePropertyValueCommand(PackageNode* root_, const Vector<ChangePropertyAction>& propertyActions_, size_t hash_)
    : QECommand("ChangePropertyValue")
    , root(SafeRetain(root_))
    , hash(hash_)
{
    for (const auto& action : propertyActions_)
    {
        AbstractProperty* prop = action.property;
        changedProperties.emplace_back(
        action.node,
        prop,
        GetValueFromProperty(prop),
        action.value
        );
    }
    Init();
}

ChangePropertyValueCommand::ChangePropertyValueCommand(PackageNode* root_, ControlNode* node, AbstractProperty* prop, const VariantType& newVal, size_t hash_)
    : QECommand("ChangePropertyValue")
    , root(SafeRetain(root_))
    , hash(hash_)
{
    changedProperties.emplace_back(node, prop, GetValueFromProperty(prop), newVal);
    Init();
}

ChangePropertyValueCommand::ChangePropertyValueCommand(PackageNode* root_, ControlNode* node, AbstractProperty* prop)
    : QECommand("ChangePropertyValue")
    , root(SafeRetain(root_))
{
    changedProperties.emplace_back(node, prop, GetValueFromProperty(prop), VariantType());
    Init();
}

void ChangePropertyValueCommand::Init()
{
    QString text = "changed:";
    std::hash<void*> ptrHash;
    for (const auto& action : changedProperties)
    {
        AbstractProperty* prop = action.property;
        hash = ptrHash(prop) ^ (hash << 1);
        text += QString(" %1").arg(prop->GetName().c_str());
    }
    SetText(text.toUtf8().data());
}

ChangePropertyValueCommand::~ChangePropertyValueCommand()
{
    SafeRelease(root);
}

void ChangePropertyValueCommand::Redo()
{
    for (const auto& action : changedProperties)
    {
        ControlNode* node = action.node;
        const VariantType& newValue = action.newValue;
        AbstractProperty* property = action.property;

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

void ChangePropertyValueCommand::Undo()
{
    for (const auto& action : changedProperties)
    {
        ControlNode* node = action.node;
        const VariantType& oldValue = action.oldValue;
        AbstractProperty* property = action.property;

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

VariantType ChangePropertyValueCommand::GetValueFromProperty(AbstractProperty* property)
{
    return property->IsOverriddenLocally() ? property->GetValue() : VariantType();
}
