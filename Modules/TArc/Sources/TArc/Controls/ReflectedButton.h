#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Controls/ControlDescriptor.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/Qt/QtString.h"
#include "TArc/Qt/QtIcon.h"

#include <QToolButton>

namespace DAVA
{
namespace TArc
{
class ReflectedButton : public ControlProxyImpl<QToolButton>
{
public:
    enum class Fields : uint32
    {
        Clicked,
        Result,
        Text, // QString
        Icon, // QIcon
        IconSize, // QSize
        AutoRaise, // bool
        Enabled, // bool
        Visible, // bool
        Tooltip, // QString
        FieldCount,
    };

    DECLARE_CONTROL_PARAMS(Fields);
    ReflectedButton(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    ReflectedButton(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void UpdateControl(const ControlDescriptor& changedfields) override;
    void SetupControl();

    void ButtonReleased();

    QIcon icon;
    QString text;
    bool autoRaise = true;

    QtConnections connections;
};
} // namespace TArc
} // namespace DAVA
