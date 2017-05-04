#include "Classes/PropertyPanel/KeyedArchiveExtensions.h"
#include "Classes/PropertyPanel/PropertyPanelCommon.h"
#include "Classes/PropertyPanel/Private/KeyedArchiveEditors.h"
#include "Classes/Commands2/KeyedArchiveCommand.h"

#include <TArc/Controls/PropertyPanel/BaseComponentValue.h>
#include <TArc/Controls/PropertyPanel/PropertyModelExtensions.h>
#include <TArc/Controls/PropertyPanel/PropertyPanelMeta.h>
#include <TArc/Utils/ReflectionHelpers.h>

#include <QtTools/WidgetHelpers/SharedIcon.h>

#include <FileSystem/KeyedArchive.h>
#include "Project/ProjectManagerData.h"
#include "Deprecated/EditorConfig.h"

namespace KeyedArchiveExtensionDetail
{
class RemoveKeyedArchiveItem : public DAVA::M::CommandProducer
{
public:
    bool IsApplyable(const std::shared_ptr<DAVA::TArc::PropertyNode>& node) const
    {
        return true;
    }

    Info GetInfo() const
    {
        Info info;
        info.icon = SharedIcon(":/QtIcons/keyminus.png");
        info.description = "Remove keyed archive member";
        info.tooltip = QStringLiteral("Remove keyed archive member");

        return info;
    }

    std::unique_ptr<DAVA::Command> CreateCommand(const std::shared_ptr<DAVA::TArc::PropertyNode>& node, const Params& params) const
    {
        std::shared_ptr<DAVA::TArc::PropertyNode> parent = node->parent.lock();
        DVASSERT(parent != nullptr);

        DAVA::KeyedArchive* archive = *parent->field.ref.GetValueObject().GetPtr<DAVA::KeyedArchive*>();
        DAVA::String key = node->field.key.Cast<DAVA::String>();
        if (archive->Count(key) > 0)
        {
            return std::make_unique<KeyeadArchiveRemValueCommand>(archive, key);
        }

        return nullptr;
    }
};
}

KeyedArchiveChildCreator::KeyedArchiveChildCreator()
{
    DAVA::M::CommandProducerHolder holder;
    holder.AddCommandProducer(std::make_shared<KeyedArchiveExtensionDetail::RemoveKeyedArchiveItem>());
    elementsMeta.reset(new DAVA::ReflectedMeta(std::move(holder)));
}

void KeyedArchiveChildCreator::ExposeChildren(const std::shared_ptr<DAVA::TArc::PropertyNode>& parent, DAVA::Vector<std::shared_ptr<DAVA::TArc::PropertyNode>>& children) const
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    if (parent->cachedValue.GetType() == Type::Instance<KeyedArchive*>())
    {
        Vector<Reflection::Field> fields = parent->field.ref.GetFields();
        std::sort(fields.begin(), fields.end(), [](const Reflection::Field& node1, const Reflection::Field& node2)
                  {
                      return node1.key.Cast<String>() < node2.key.Cast<String>();
                  });

        for (Reflection::Field& f : fields)
        {
            f.ref = Reflection::Create(f.ref, elementsMeta.get());
            std::shared_ptr<PropertyNode> node = allocator->CreatePropertyNode(parent, std::move(f), static_cast<int32>(children.size()), PropertyNode::RealProperty);
            node->idPostfix = FastName(node->cachedValue.GetType()->GetName());
            children.push_back(node);
        }
    }
    else
    {
        ChildCreatorExtension::ExposeChildren(parent, children);
    }
}

KeyedArchiveEditorCreator::KeyedArchiveEditorCreator(DAVA::TArc::ContextAccessor* accessor_)
    : accessor(accessor_)
{
}

std::unique_ptr<DAVA::TArc::BaseComponentValue> KeyedArchiveEditorCreator::GetEditor(const std::shared_ptr<const DAVA::TArc::PropertyNode>& node) const
{
    using namespace DAVA;
    using namespace DAVA::TArc;

    if (node->propertyType == PropertyNode::RealProperty && node->cachedValue.GetType() == Type::Instance<KeyedArchive*>())
    {
        return std::make_unique<PropertyPanel::KeyedArchiveEditor>();
    }

    std::shared_ptr<DAVA::TArc::PropertyNode> parent = node->parent.lock();
    if (parent->propertyType == PropertyNode::RealProperty && parent->cachedValue.GetType() == Type::Instance<KeyedArchive*>())
    {
        String key = node->field.key.Cast<String>();

        ProjectManagerData* data = accessor->GetGlobalContext()->GetData<ProjectManagerData>();
        DVASSERT(data);

        const EditorConfig* editorConfig = data->GetEditorConfig();
        int32 presetType = editorConfig->GetPropertyValueType(key);
        if (presetType != VariantType::TYPE_NONE)
        {
            int32 valueVariantType = VariantType::TYPE_NONE;
            const Type* valueType = node->cachedValue.GetType();
            if (valueType == Type::Instance<int32>())
            {
                Vector<Any> allowedValues;
                const Vector<String>& presetValues = editorConfig->GetComboPropertyValues(key);
                if (presetValues.empty() == false)
                {
                    std::for_each(presetValues.begin(), presetValues.end(), [&allowedValues](const String& v)
                                  {
                                      allowedValues.push_back(v);
                                  });
                }

                const Vector<Color>& presetColors = editorConfig->GetColorPropertyValues(key);
                if (presetColors.empty() == false)
                {
                    std::for_each(presetColors.begin(), presetColors.end(), [&allowedValues](const Color& v)
                                  {
                                      allowedValues.push_back(v);
                                  });
                }

                if (allowedValues.empty() == false)
                {
                    return std::make_unique<PropertyPanel::KeyedArchiveComboPresetEditor>(allowedValues);
                }
            }
        }
    }

    return EditorComponentExtension::GetEditor(node);
}
