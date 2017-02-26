#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/StaticEditorDrawer.h"

#include "TArc/DataProcessing/DataWrappersProcessor.h"

#include <Reflection/ReflectionRegistrator.h>

#include <QStyle>
#include <QStyleOption>

namespace DAVA
{
namespace TArc
{
StaticEditorProxy::StaticEditorProxy(BaseComponentValue* valueComponent, const StaticEditorDrawer* drawer_)
    : value(valueComponent)
    , drawer(drawer_)
{
}

uint32 StaticEditorProxy::GetHeight(QStyle* style, const QStyleOptionViewItem& options) const
{
    StaticEditorDrawer::Params params;
    params.options = options;
    params.style = style;
    params.value = value->GetValue();
    params.nodes = &value->nodes;
    return drawer->GetHeight(params);
}

void StaticEditorProxy::Draw(QStyle* style, QPainter* painter, const QStyleOptionViewItem& options) const
{
    StaticEditorDrawer::Params params;
    params.options = options;
    params.style = style;
    params.value = value->GetValue();
    params.nodes = &value->nodes;
    drawer->Draw(painter, params);
}

InteractiveEditorProxy::InteractiveEditorProxy(BaseComponentValue* valueComponent)
    : value(valueComponent)
{
}

QWidget* InteractiveEditorProxy::AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option)
{
    value->UpdateCachedValue();
    return value->AcquireEditorWidget(parent, option);
}

void InteractiveEditorProxy::ReleaseEditorWidget(QWidget* editor)
{
    value->ReleaseEditorWidget(editor);
    value->ClearCachedValue();
}

void InteractiveEditorProxy::CommitData()
{
    value->CommitData();
}

QRect InteractiveEditorProxy::GetEditorRect(QStyle* style, const QStyleOptionViewItem& option)
{
    QStyleOptionViewItem opt = option;
    opt.showDecorationSelected = true;
    return style->subElementRect(QStyle::SE_ItemViewItemText, &opt, option.widget);
}

//////////////////////////////////////////////////////////////////////////////

BaseComponentValue::BaseComponentValue()
{
    thisValue = this;
}

void BaseComponentValue::Init(ReflectedPropertyModel* model_)
{
    model = model_;
}

StaticEditorProxy BaseComponentValue::GetStaticEditor()
{
    return StaticEditorProxy(this, GetStaticEditorDrawer());
}

InteractiveEditorProxy BaseComponentValue::GetInteractiveEditor()
{
    return InteractiveEditorProxy(this);
}

void BaseComponentValue::ReleaseEditorWidget(QWidget* editor)
{
    editor->deleteLater();
}

bool BaseComponentValue::EditorEvent(QEvent* event, const QStyleOptionViewItem& option)
{
    return false;
}

QString BaseComponentValue::GetPropertyName() const
{
    const Reflection& r = nodes.front()->field.ref;
    const M::DisplayName* displayName = r.GetMeta<M::DisplayName>();
    if (displayName != nullptr)
    {
        return QString::fromStdString(displayName->displayName);
    }

    return nodes.front()->field.key.Cast<QString>();
}

int32 BaseComponentValue::GetPropertiesNodeCount() const
{
    return static_cast<int32>(nodes.size());
}

std::shared_ptr<const PropertyNode> BaseComponentValue::GetPropertyNode(int32 index) const
{
    DVASSERT(static_cast<size_t>(index) < nodes.size());
    return nodes[static_cast<size_t>(index)];
}

void BaseComponentValue::SetValue(const Any& value)
{
    cachedValue = value;
}

void BaseComponentValue::AddPropertyNode(const std::shared_ptr<PropertyNode>& node)
{
    nodes.push_back(node);
}

void BaseComponentValue::RemovePropertyNode(const std::shared_ptr<PropertyNode>& node)
{
    auto iter = std::find(nodes.begin(), nodes.end(), node);
    if (iter == nodes.end())
    {
        DVASSERT(false);
        return;
    }

    nodes.erase(iter);
}

void BaseComponentValue::RemovePropertyNodes()
{
    nodes.clear();
}

std::shared_ptr<ModifyExtension> BaseComponentValue::GetModifyInterface()
{
    return model->GetExtensionChain<ModifyExtension>();
}

void BaseComponentValue::UpdateCachedValue()
{
    cachedValue = GetValue();
}

void BaseComponentValue::ClearCachedValue()
{
    cachedValue = Any();
}

void BaseComponentValue::CommitData()
{
    if (IsValidValueToSet(cachedValue, GetValue()))
    {
        GetModifyInterface()->ModifyPropertyValue(nodes, cachedValue);
    }
}

DataWrappersProcessor* BaseComponentValue::GetWrappersProcessor()
{
    return &model->wrappersProcessor;
}

DAVA::Reflection BaseComponentValue::GetReflection()
{
    return Reflection::Create(&thisValue);
}

DAVA_VIRTUAL_REFLECTION_IMPL(BaseComponentValue)
{
    ReflectionRegistrator<BaseComponentValue>::Begin()
    .End();
}
}
}
