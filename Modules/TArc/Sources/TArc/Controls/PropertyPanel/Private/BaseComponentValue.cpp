#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectedPropertyModel.h"
#include "TArc/Controls/PropertyPanel/StaticEditorDrawer.h"

#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/ScopedValueGuard.h"

#include <Engine/PlatformApi.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Logger/Logger.h>

#include <QStyle>
#include <QStyleOption>
#include <QPainter>
#include <QApplication>

namespace DAVA
{
namespace TArc
{
BaseComponentValue::BaseComponentValue()
{
    thisValue = this;
}

BaseComponentValue::~BaseComponentValue()
{
    if (editorWidget != nullptr)
    {
        editorWidget->TearDown();
        editorWidget->ToWidgetCast()->deleteLater();
        editorWidget = nullptr;
    }
}

void BaseComponentValue::Init(ReflectedPropertyModel* model_)
{
    model = model_;
}

void BaseComponentValue::Draw(QWidget* parent, QPainter* painter, const QStyleOptionViewItem& opt)
{
    UpdateEditorGeometry(parent, opt.rect);
    painter->drawPixmap(opt.rect, editorWidget->ToWidgetCast()->grab());
}

void BaseComponentValue::UpdateGeometry(QWidget* parent, const QStyleOptionViewItem& opt)
{
    UpdateEditorGeometry(parent, opt.rect);
}

bool BaseComponentValue::HasHeightForWidth(const QWidget* parent) const
{
    EnsureEditorCreated(parent);
    return editorWidget->ToWidgetCast()->hasHeightForWidth();
}

int BaseComponentValue::GetHeightForWidth(const QWidget* parent, int width) const
{
    EnsureEditorCreated(parent);
    return editorWidget->ToWidgetCast()->heightForWidth(width);
}

int BaseComponentValue::GetHeight(const QWidget* parent) const
{
    EnsureEditorCreated(parent);
    return editorWidget->ToWidgetCast()->sizeHint().height();
}

QWidget* BaseComponentValue::AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option)
{
    UpdateEditorGeometry(parent, option.rect);
    return editorWidget->ToWidgetCast();
}

void BaseComponentValue::ReleaseEditorWidget(QWidget* editor)
{
    DVASSERT(editorWidget->ToWidgetCast() == editor);
}

bool BaseComponentValue::EditorEvent(QWidget* parent, QEvent* event, const QStyleOptionViewItem& option)
{
    SCOPED_VALUE_GUARD(bool, isEditorEvent, true, false);
    UpdateEditorGeometry(parent, option.rect);
    switch (event->type())
    {
    case QEvent::MouseButtonPress:
    case QEvent::MouseButtonRelease:
    {
        QMouseEvent* e = static_cast<QMouseEvent*>(event);
        QPoint pos = editorWidget->ToWidgetCast()->mapFrom(parent, e->pos());
        QMouseEvent newEvent(e->type(), pos, e->screenPos(), e->button(), e->buttons(), e->modifiers());
        return PlatformApi::Qt::GetApplication()->sendEvent(editorWidget->ToWidgetCast(), &newEvent);
    }
    default:
        break;
    }
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

bool BaseComponentValue::IsReadOnly() const
{
    Reflection r = nodes.front()->field.ref;
    return r.IsReadonly() || r.HasMeta<M::ReadOnly>();
}

DAVA::Any BaseComponentValue::GetValue() const
{
    Any value = nodes.front()->cachedValue;
    for (const std::shared_ptr<const PropertyNode>& node : nodes)
    {
        if (value != node->cachedValue)
        {
            return GetMultipleValue();
        }
    }

    return value;
}

void BaseComponentValue::SetValue(const Any& value)
{
    if (IsValidValueToSet(value, GetValue()))
    {
        model->GetExtensionChain<ModifyExtension>()->ModifyPropertyValue(nodes, value);
    }
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

void BaseComponentValue::EnsureEditorCreated(const QWidget* parent) const
{
    if (editorWidget == nullptr)
    {
        editorWidget = CreateEditorWidget(const_cast<QWidget*>(parent),
                                          Reflection::Create(&const_cast<BaseComponentValue*>(this)->thisValue),
                                          &const_cast<ReflectedPropertyModel*>(model)->wrappersProcessor);
        editorWidget->ForceUpdate();
    }

    DVASSERT(editorWidget->ToWidgetCast()->parent() == parent);
}

void BaseComponentValue::UpdateEditorGeometry(const QWidget* parent, const QRect& geometry) const
{
    EnsureEditorCreated(parent);
    QWidget* w = editorWidget->ToWidgetCast();
    if (w->geometry() != geometry)
    {
        w->setGeometry(geometry);
    }
}

const char* BaseComponentValue::readOnlyFieldName = "isReadOnly";

DAVA_VIRTUAL_REFLECTION_IMPL(BaseComponentValue)
{
    ReflectionRegistrator<BaseComponentValue>::Begin()
    .Field(readOnlyFieldName, &BaseComponentValue::IsReadOnly, nullptr)
    .End();
}
}
}
