#include "TArc/Controls/PropertyPanel/Private/BoolComponentValue.h"
#include "TArc/Controls/CheckBox.h"
#include "TArc/Controls/PropertyPanel/DefaultEditorDrawers.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/ScopedValueGuard.h"

#include <Engine/PlatformApi.h>
#include <Reflection/ReflectionRegistrator.h>
#include <Base/FastName.h>

#include <QApplication>
#include <QtEvents>
#include <QStyle>
#include <QStyleOption>

namespace DAVA
{
namespace TArc
{
Qt::CheckState BoolComponentValue::GetCheckState() const
{
    return GetValue().Cast<Qt::CheckState>();
}

void BoolComponentValue::SetCheckState(Qt::CheckState checkState)
{
    SetValue(checkState);
}

QWidget* BoolComponentValue::AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option)
{
    SCOPED_VALUE_GUARD(bool, implCall, false, nullptr);

    ControlDescriptorBuilder<CheckBox::Fields> descr;
    descr[CheckBox::Fields::Checked] = "bool";
    descr[CheckBox::Fields::TextHint] = "textHint";
    CheckBox* checkBox = new CheckBox(descr, GetWrappersProcessor(), GetReflection(), parent);
    checkBox->ForceUpdateControl();
    return checkBox->ToWidgetCast();
}

void BoolComponentValue::ReleaseEditorWidget(QWidget* editor)
{
    SCOPED_VALUE_GUARD(bool, implCall, false, void());

    editor->deleteLater();
}

bool BoolComponentValue::EditorEvent(QEvent* event, const QStyleOptionViewItem& option)
{
    ScopedValueGuard<bool> guard(implCall, true);
    return HitEventAndResend(event, option);
}

String BoolComponentValue::GetTextHint() const
{
    return staticEditor.GetTextHint(GetValue(), &nodes).toStdString();
}

bool BoolComponentValue::IsReadOnly() const
{
    return nodes.front()->field.ref.IsReadonly();
}

bool BoolComponentValue::IsEnabled() const
{
    return true;
}

bool BoolComponentValue::HitEventAndResend(QEvent* event, const QStyleOptionViewItem& option)
{
    QEvent::Type type = event->type();
    if (type != QEvent::MouseButtonRelease)
    {
        return false;
    }

    QMouseEvent* mouseEvent = static_cast<QMouseEvent*>(event);
    if (mouseEvent->button() != Qt::LeftButton)
    {
        return false;
    }

    bool result = false;

    QApplication* app = PlatformApi::Qt::GetApplication();
    StaticEditorDrawer::Params params;
    params.style = option.widget->style();
    params.options = option;
    params.value = GetValue();
    params.nodes = &nodes;

    staticEditor.InitStyleOptions(params);

    QRect textRect = params.style->subElementRect(QStyle::SE_ItemViewItemText, &params.options, params.options.widget);
    QRect checkRect = params.style->subElementRect(QStyle::SE_ItemViewItemCheckIndicator, &params.options, params.options.widget);
    QRect unitedRect = checkRect.united(textRect);

    if (unitedRect.contains(mouseEvent->pos()))
    {
        InteractiveEditorProxy proxy = GetInteractiveEditor();
        QWidget* editor = proxy.AcquireEditorWidget(nullptr, params.options);

        QStyleOptionButton checkBoxOptions;
        checkBoxOptions.initFrom(editor);
        QPoint editorPoint = editor->style()->subElementRect(QStyle::SE_CheckBoxClickRect, &checkBoxOptions, editor).center();

        QMouseEvent pressEvent(QEvent::MouseButtonPress, editorPoint, mouseEvent->button(),
                               mouseEvent->buttons(), mouseEvent->modifiers());
        QMouseEvent releaseEvent(QEvent::MouseButtonRelease, editorPoint, mouseEvent->button(),
                                 mouseEvent->buttons(), mouseEvent->modifiers());
        app->sendEvent(editor, &pressEvent);
        app->sendEvent(editor, &releaseEvent);

        proxy.CommitData();
        proxy.ReleaseEditorWidget(editor);

        result = true;
    }

    return result;
}

DAVA_REFLECTION_IMPL(BoolComponentValue)
{
    ReflectionRegistrator<BoolComponentValue>::Begin()
    .Field("bool", &BoolComponentValue::GetCheckState, &BoolComponentValue::SetCheckState)
    .Field("textHint", &BoolComponentValue::GetTextHint, nullptr)
    .Field("readOnly", &BoolComponentValue::IsReadOnly, nullptr)
    .Field("enabled", &BoolComponentValue::IsEnabled, nullptr)
    .End();
}
}
}
