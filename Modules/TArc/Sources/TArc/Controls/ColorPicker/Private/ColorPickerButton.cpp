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
    : ControlProxyImpl<QToolButton>(params, ControlDescriptor(params.fields), wrappersProcessor, model, parent)
{
    SetupControl();
}

ColorPickerButton::ColorPickerButton(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent)
    : ControlProxyImpl<QToolButton>(params, ControlDescriptor(params.fields), accessor, model, parent)
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
    Reflection fieldValue = model.GetField(changedFields.GetName(Fields::Color));
    DVASSERT(fieldValue.IsValid());

    if (changedFields.IsChanged(Fields::Range) == true)
    {
        rangeMeta = GetFieldValue<const M::Range*>(Fields::Range, nullptr);
    }

    if (rangeMeta == nullptr)
    {
        rangeMeta = fieldValue.GetMeta<M::Range>();
    }

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

    ColorPickerDialog cp(controlParams.accessor, this);
    cp.setWindowTitle("Select color");

    Color prevValue = cachedColor.Get<Color>();
    cp.SetDavaColor(prevValue);

    bool changed = cp.Exec();
    if (changed)
    {
        Color colorValue = cp.GetDavaColor();
        if (rangeMeta != nullptr)
        {
            auto clampValue = [](float32 prevV, float32 newV, float32 minV, float32 maxV, float32 stepV)
            {
                float32 halfStep = stepV / 2.0f;
                if (newV > prevV)
                {
                    newV += halfStep;
                }
                else
                {
                    newV -= halfStep;
                }

                int32 stepCount = static_cast<int32>((newV - prevV) / stepV);
                newV = prevV + stepCount * stepV;

                return Clamp(newV, minV, maxV);
            };

            Color minValue = rangeMeta->minValue.Get<Color>();
            Color maxValue = rangeMeta->maxValue.Get<Color>();
            Color stepValue = rangeMeta->step.Get<Color>();

            colorValue.r = clampValue(prevValue.r, colorValue.r, minValue.r, maxValue.r, stepValue.r);
            colorValue.g = clampValue(prevValue.g, colorValue.g, minValue.g, maxValue.g, stepValue.g);
            colorValue.b = clampValue(prevValue.b, colorValue.b, minValue.b, maxValue.b, stepValue.b);
            colorValue.a = clampValue(prevValue.a, colorValue.a, minValue.a, maxValue.a, stepValue.a);
        }

        cachedColor = colorValue;
        setIcon(cachedColor.Cast<QIcon>(QIcon()));

        wrapper.SetFieldValue(GetFieldName(Fields::Color), cachedColor);
    }
}

} // namespace TArc
} // namespace DAVA
