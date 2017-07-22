#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Controls/ControlDescriptor.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/WindowSubSystem/UI.h"

#include <Math/Color.h>

#include <QToolButton>

namespace DAVA
{
namespace TArc
{
class ColorPickerButton : public ControlProxyImpl<QToolButton>
{
public:
    enum class Fields : uint32
    {
        Color,
        IsReadOnly,
        Range,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);

    ColorPickerButton(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    ColorPickerButton(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

    void UpdateControl(const ControlDescriptor& changedfields) override;
    void SetupControl();

    void ButtonReleased();

    const M::Range* rangeMeta = nullptr;
    Any cachedColor;
    bool readOnly = false;
    QtConnections connections;
};
} // namespace TArc
} // namespace DAVA
