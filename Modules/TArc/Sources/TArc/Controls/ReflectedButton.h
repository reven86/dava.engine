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
        Text,
        Icon,
        AutoRaise,
        Enabled,
        FieldCount
    };

    ReflectedButton(const ControlDescriptorBuilder<Fields>& fields, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    ReflectedButton(const ControlDescriptorBuilder<Fields>& fields, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;

    void UpdateControl(const ControlDescriptor& changedfields) override;
    void SetupControl();

    void ButtonReleased();

    QIcon icon;
    QString text;
    bool enabled = true;
    bool autoRaise = true;

    QtConnections connections;
};
} // namespace TArc
} // namespace DAVA
