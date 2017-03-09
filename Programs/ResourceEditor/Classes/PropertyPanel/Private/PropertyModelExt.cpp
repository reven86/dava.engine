#include "Classes/PropertyPanel/PropertyModelExt.h"
#include "Classes/SceneManager/SceneData.h"
#include "Classes/Commands2/SetFieldValueCommand.h"

#include <TArc/Utils/ReflectionHelpers.h>

#include <Reflection/Reflection.h>

REModifyPropertyExtension::REModifyPropertyExtension(DAVA::TArc::ContextAccessor* accessor_)
    : accessor(accessor_)
{
}

void REModifyPropertyExtension::ProduceCommand(const DAVA::Reflection::Field& field, const DAVA::Any& newValue)
{
    GetScene()->Exec(std::make_unique<SetFieldValueCommand>(field, newValue));
}

void REModifyPropertyExtension::Exec(std::unique_ptr<DAVA::Command>&& command)
{
    GetScene()->Exec(std::move(command));
}

void REModifyPropertyExtension::EndBatch()
{
    GetScene()->EndBatch();
}

void REModifyPropertyExtension::BeginBatch(const DAVA::String& text, DAVA::uint32 commandCount)
{
    GetScene()->BeginBatch(text, commandCount);
}

SceneEditor2* REModifyPropertyExtension::GetScene() const
{
    using namespace DAVA::TArc;
    DataContext* ctx = accessor->GetActiveContext();
    DVASSERT(ctx != nullptr);

    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);

    return data->GetScene().Get();
}

void EntityChildCreator::ExposeChildren(const std::shared_ptr<const DAVA::TArc::PropertyNode>& parent, DAVA::Vector<std::shared_ptr<DAVA::TArc::PropertyNode>>& children) const
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    if (parent->propertyType == PropertyNode::SelfRoot &&
        parent->cachedValue.GetType() == DAVA::Type::Instance<DAVA::Entity*>())
    {
        DAVA::Reflection::Field f(Any("Entity"), Reflection(parent->field.ref), nullptr);
        children.push_back(allocator->CreatePropertyNode(std::move(f), PropertyNode::GroupProperty));

        {
            DAVA::Reflection componentsField = f.ref.GetField("Components");
            DVASSERT(componentsField.IsValid());

            DAVA::TArc::ForEachField(componentsField, [this, &children](Reflection::Field&& field)
                                     {
                                         if (!field.ref.HasMeta<M::HiddenField>())
                                         {
                                             Any value = field.ref.GetValue();
                                             DAVA::Reflection::Field f(GetValueReflectedType(field.ref)->GetPermanentName(), Reflection(field.ref), nullptr);
                                             children.push_back(allocator->CreatePropertyNode(std::move(f), PropertyNode::RealProperty));
                                         }
                                     });
        }
    }
    else if (parent->propertyType == PropertyNode::GroupProperty &&
             parent->cachedValue.GetType() == DAVA::Type::Instance<DAVA::Entity*>())
    {
        DAVA::TArc::ForEachField(parent->field.ref, [this, &children](Reflection::Field&& field)
                                 {
                                     if (field.ref.GetValueType() != DAVA::Type::Instance<DAVA::Vector<DAVA::Component*>>() && field.ref.HasMeta<M::HiddenField>() == false)
                                     {
                                         children.push_back(allocator->CreatePropertyNode(std::move(field), PropertyNode::RealProperty));
                                     }
                                 });
    }
    else
    {
        ChildCreatorExtension::ExposeChildren(parent, children);
    }
}
