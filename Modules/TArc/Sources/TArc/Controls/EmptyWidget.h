#pragma once

#include "TArc/Controls/ControlProxy.h"

#include <QWidget>

namespace DAVA
{
namespace TArc
{
class EmptyWidget : public ControlProxyImpl<QWidget>
{
    using TBase = ControlProxyImpl<QWidget>;

public:
    enum class Fields : uint32
    {
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    EmptyWidget(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    EmptyWidget(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

protected:
    void UpdateControl(const ControlDescriptor& descriptor) override;

    void mousePressEvent(QMouseEvent* e) override;
    void mouseReleaseEvent(QMouseEvent* e) override;
    void mouseMoveEvent(QMouseEvent* e) override;
};
} // namespace TArc
} // namespace DAVA