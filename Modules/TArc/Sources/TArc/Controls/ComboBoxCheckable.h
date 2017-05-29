#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Controls/ControlDescriptor.h"
#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Utils/QtConnections.h"

#include <QComboBox>
#include <QString>

class QPaintEvent;
class QEvent;

namespace DAVA
{
namespace TArc
{
QString CreateTextComboCheckable(const Any& value, const EnumMap* enumMap);

class ComboBoxCheckable final : public ControlProxyImpl<QComboBox>
{
public:
    enum class Fields : uint32
    {
        Value,
        IsReadOnly,
        FieldCount
    };

    DECLARE_CONTROL_PARAMS(Fields);
    ComboBoxCheckable(const Params& params, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent = nullptr);
    ComboBoxCheckable(const Params& params, ContextAccessor* accessor, Reflection model, QWidget* parent = nullptr);

private:
    bool eventFilter(QObject* obj, QEvent* e) override;
    void paintEvent(QPaintEvent* event) override;

    void SetupControl();
    void UpdateControl(const ControlDescriptor& changedfields) override;

    void CreateItems(const Reflection& fieldValue);
    void SelectCurrentItems(const Reflection& fieldValue);

    void ApplyChanges();
    void UpdateText();
    void UpdateCheckedState();

    int cachedValue = 0;
    QString text;
};
} // namespace TArc
} // namespace DAVA
