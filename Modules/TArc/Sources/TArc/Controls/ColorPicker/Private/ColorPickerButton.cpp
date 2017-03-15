#include "TArc/Controls/ColorPicker/ColorPickerButton.h"
#include "TArc/Controls/ColorPicker/ColorPickerDialog.h"

#include <Base/FastName.h>
#include <Reflection/ReflectedMeta.h>

#include <QColor>
#include <QPalette>

namespace DAVA
{
namespace TArc
{
ColorPickerButton::ColorPickerButton(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QToolButton>(ControlDescriptor(params.fields), wrappersProcessor, model, parent)
    , ui(params.ui)
    , wndKey(params.wndKey)
    , contextAccessor(params.accessor)
{
    SetupControl();
}

ColorPickerButton::ColorPickerButton(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QToolButton>(ControlDescriptor(params.fields), accessor, model, parent)
    , ui(params.ui)
    , wndKey(params.wndKey)
    , contextAccessor(params.accessor)
{
    DVASSERT(accessor == params.accessor);
    SetupControl();
}

void ColorPickerButton::SetupControl()
{
    setToolButtonStyle(Qt::ToolButtonIconOnly);
    connections.AddConnection(this, &QToolButton::released, MakeFunction(this, &ColorPickerButton::ButtonReleased));
}

void ColorPickerButton::UpdateControl(const ControlDescriptor& changedFields)
{
    DAVA::Reflection fieldValue = model.GetField(changedFields.GetName(Fields::Color));
    DVASSERT(fieldValue.IsValid());

    readOnly = IsValueReadOnly(changedFields, Fields::Color, Fields::IsReadOnly);
    if (changedFields.IsChanged(Fields::Color) == true)
    {
        cachedColor = fieldValue.GetValue();
        setIcon(cachedColor.Cast<QIcon>(QIcon()));
    }
}

void ColorPickerButton::mousePressEvent(QMouseEvent* e)
{
    if (readOnly == false)
    {
        QToolButton::mousePressEvent(e);
    }
}

void ColorPickerButton::mouseReleaseEvent(QMouseEvent* e)
{
    if (readOnly == false)
    {
        QToolButton::mouseReleaseEvent(e);
    }
}

void ColorPickerButton::ButtonReleased()
{
    if (readOnly)
    {
        return;
    }

    ColorPickerDialog cp(contextAccessor, this);
    cp.setWindowTitle("Select color");
    cp.SetDavaColor(cachedColor.Get<Color>());

    bool changed = cp.Exec();
    if (changed)
    {
        cachedColor = cp.GetDavaColor();
        setIcon(cachedColor.Cast<QIcon>(QIcon()));

        wrapper.SetFieldValue(GetFieldName(Fields::Color), cachedColor);
    }
}

} // namespace TArc
} // namespace DAVA
