#include "Classes/PropertyPanel/PropertyModelExt.h"
#include "Classes/SceneManager/SceneData.h"
#include "Classes/Commands2/SetFieldValueCommand.h"

#include <Reflection/Reflection.h>

REModifyPropertyExtension::REModifyPropertyExtension(DAVA::TArc::ContextAccessor* accessor_)
    : accessor(accessor_)
{
}

void REModifyPropertyExtension::ProduceCommand(const DAVA::Vector<DAVA::Reflection::Field>& objects, const DAVA::Any& newValue)
{
    using namespace DAVA::TArc;
    DataContext* ctx = accessor->GetActiveContext();
    DVASSERT(ctx != nullptr);

    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);

    DAVA::RefPtr<SceneEditor2> scene = data->GetScene();
    scene->BeginBatch("Set property value", static_cast<DAVA::uint32>(objects.size()));
    for (const DAVA::Reflection::Field& field : objects)
    {
        scene->Exec(std::make_unique<SetFieldValueCommand>(field, newValue));
    }
    scene->EndBatch();
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

            DAVA::Vector<DAVA::Reflection::Field> fields = componentsField.GetFields();
            for (DAVA::Reflection::Field& field : fields)
            {
                if (!field.ref.HasMeta<M::HiddenField>())
                {
                    //const Type* type = field.ref.GetValueType();
                    //const ReflectedType* type1 = field.ref.GetValueObject().GetReflectedType();
                    DAVA::Reflection::Field f(Any(field.key), Reflection(field.ref), nullptr);
                    children.push_back(allocator->CreatePropertyNode(std::move(f), PropertyNode::RealProperty));
                }
            }
        }
    }
    else if (parent->propertyType == PropertyNode::GroupProperty &&
             parent->cachedValue.GetType() == DAVA::Type::Instance<DAVA::Entity*>())
    {
        DAVA::Vector<DAVA::Reflection::Field> fields = parent->field.ref.GetFields();
        for (DAVA::Reflection::Field& field : fields)
        {
            if (field.ref.GetValueType() != DAVA::Type::Instance<DAVA::Vector<DAVA::Component*>>() && field.ref.HasMeta<M::HiddenField>() == false)
            {
                children.push_back(allocator->CreatePropertyNode(std::move(field), PropertyNode::RealProperty));
            }
        }
    }
    else
    {
        ChildCreatorExtension::ExposeChildren(parent, children);
    }
}
