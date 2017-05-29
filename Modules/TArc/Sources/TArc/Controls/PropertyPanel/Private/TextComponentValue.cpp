#include "TextComponentValue.h"
#include "TArc/Controls/LineEdit.h"
#include "TArc/Controls/Widget.h"
#include "TArc/Controls/PlainTextEdit.h"
#include "TArc/Controls/QtBoxLayouts.h"
#include "TArc/Controls/PropertyPanel/Private/ComponentStructureWrapper.h"
#include "TArc/Controls/PropertyPanel/PropertyPanelMeta.h"
#include "TArc/Controls/CommonStrings.h"

#include <QtTools/WidgetHelpers/SharedIcon.h>

#include <Reflection/ReflectionRegistrator.h>
#include <Reflection/ReflectedMeta.h>

#include <QBoxLayout>
#include <QToolButton>
#include <QPushButton>
#include <QDialogButtonBox>

namespace DAVA
{
namespace TArc
{
namespace TextComponentValueDetail
{
class MultilineEditDialog : public QDialog
{
public:
    MultilineEditDialog(QWidget* parent, ContextAccessor* accessor, UI* ui, const WindowKey& wndKey, DataWrappersProcessor* processor, const String& initValue)
        : QDialog(parent)
        , value(initValue)
    {
        QVBoxLayout* mainLayout = new QVBoxLayout(this);
        mainLayout->setMargin(2);
        mainLayout->setSpacing(4);

        QtHBoxLayout* layout = new QtHBoxLayout();
        layout->setMargin(0);
        layout->setSpacing(0);

        PlainTextEdit::Params params(accessor, ui, wndKey);
        params.fields[PlainTextEdit::Fields::Text] = "text";
        PlainTextEdit* edit = new PlainTextEdit(params, processor, Reflection::Create(ReflectedObject(this)), this);
        edit->ForceUpdate();
        layout->AddControl(edit);

        mainLayout->addLayout(layout);

        QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
        QObject::connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &QDialog::accept);
        QObject::connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QDialog::reject);

        mainLayout->addWidget(buttonBox);
    }

    const String& GetText() const
    {
        return value;
    }

private:
    String value;

    DAVA_REFLECTION(MultilineEditDialog);
};

DAVA_REFLECTION_IMPL(MultilineEditDialog)
{
    ReflectionRegistrator<MultilineEditDialog>::Begin()
    .Field("text", &MultilineEditDialog::value)
    .End();
}
}

Any TextComponentValue::GetMultipleValue() const
{
    static Any multValue = String(MultipleValuesString);
    return multValue;
}

bool TextComponentValue::IsValidValueToSet(const Any& newValue, const Any& currentValue) const
{
    if (currentValue.IsEmpty())
        return true;

    if (newValue.IsEmpty())
        return false;

    String newStrignValue = newValue.Cast<String>();
    String currentStringValue = currentValue.Cast<String>();
    return newStrignValue != GetMultipleValue().Cast<String>() && newStrignValue != currentStringValue;
}

String TextComponentValue::GetText() const
{
    return GetValue().Cast<String>();
}

void TextComponentValue::SetText(const String& text)
{
    SetValue(text);
}

ControlProxy* TextComponentValue::CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor)
{
    LineEdit::Params params(GetAccessor(), GetUI(), GetWindowKey());
    params.fields[LineEdit::Fields::Text] = "text";
    params.fields[LineEdit::Fields::IsReadOnly] = readOnlyFieldName;
    return new LineEdit(params, wrappersProcessor, model, parent);
}

DAVA_VIRTUAL_REFLECTION_IMPL(TextComponentValue)
{
    ReflectionRegistrator<TextComponentValue>::Begin(CreateComponentStructureWrapper<TextComponentValue>())
    .Field("text", &TextComponentValue::GetText, &TextComponentValue::SetText)[M::ProxyMetaRequire()]
    .End();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////

DAVA::TArc::ControlProxy* MultiLineTextComponentValue::CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor)
{
    Widget* w = new Widget(parent);
    QHBoxLayout* layout = new QHBoxLayout();
    layout->setMargin(1);
    layout->setSpacing(2);
    w->SetLayout(layout);

    QToolButton* button = new QToolButton(w->ToWidgetCast());
    button->setIcon(SharedIcon(":/QtIcons/pencil.png"));
    button->setToolTip("Open multiline editor");
    button->setIconSize(toolButtonIconSize);
    button->setAutoRaise(false);
    connections.AddConnection(button, &QToolButton::clicked, MakeFunction(this, &MultiLineTextComponentValue::OpenMultiLineEdit));
    layout->addWidget(button);

    ControlProxy* baseControl = TextComponentValue::CreateEditorWidget(w->ToWidgetCast(), model, wrappersProcessor);
    w->AddControl(baseControl);
    QWidget* widget = w->ToWidgetCast();
    QWidget* controlWidget = baseControl->ToWidgetCast();
    widget->setFocusProxy(controlWidget);
    widget->setFocusPolicy(controlWidget->focusPolicy());

    return w;
}

void MultiLineTextComponentValue::OpenMultiLineEdit()
{
    TextComponentValueDetail::MultilineEditDialog dlg(realWidget, GetAccessor(), GetUI(), GetWindowKey(), GetDataProcessor(), GetText());
    if (dlg.exec() == QDialog::Accepted)
    {
        SetText(dlg.GetText());
    }
}
}
}
