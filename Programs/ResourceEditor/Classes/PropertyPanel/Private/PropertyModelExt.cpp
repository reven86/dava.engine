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
    if (ctx == nullptr)
    {
        return;
    }

    SceneData* data = ctx->GetData<SceneData>();
    DVASSERT(data != nullptr);

    DAVA::RefPtr<SceneEditor2> scene = data->GetScene();
    scene->BeginBatch("Set property value", objects.size());
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
        parent->cachedValue.GetType() == Type::Instance<DAVA::Entity*>())
    {
        DAVA::Reflection::Field f;
        f.key = "Entity";
        f.ref = parent->field.ref;
        children.push_back(allocator->CreatePropertyNode(std::move(f), PropertyNode::GroupProperty));

        {
            Reflection componentsField = f.ref.GetField("Components");
            DVASSERT(componentsField.IsValid());

            Vector<Reflection::Field> fields = componentsField.GetFields();
            for (Reflection::Field& field : fields)
            {
                Reflection::Field f;
                f.ref = field.ref;
                f.key = field.key;
                children.push_back(allocator->CreatePropertyNode(std::move(f), PropertyNode::RealProperty));
            }
        }
    }
    else if (parent->propertyType == PropertyNode::GroupProperty &&
             parent->cachedValue.GetType() == Type::Instance<DAVA::Entity*>())
    {
        Vector<Reflection::Field> fields = parent->field.ref.GetFields();
        for (Reflection::Field& field : fields)
        {
            if (field.ref.GetValueType() != Type::Instance<Vector<Component*>>())
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
