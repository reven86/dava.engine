#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Controls/Private/ValidationUtils.h"
#include "TArc/Controls/CommonStrings.h"
#include "TArc/Utils/QtConnections.h"

#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QKeyEvent>
#include <QLineEdit>
#include <QString>
#include <QToolTip>

namespace DAVA
{
namespace TArc
{
template <typename TBase, typename TEditableType>
class BaseSpinBox : public ControlProxyImpl<TBase>
{
public:
    enum BaseFields : uint32
    {
        Value,
        IsReadOnly,
        IsEnabled,
        Range
    };

    BaseSpinBox(const ControlDescriptor& descriptor, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent);
    BaseSpinBox(const ControlDescriptor& descriptor, ContextAccessor* accessor, Reflection model, QWidget* parent);

protected:
    bool event(QEvent* e) override;
    void UpdateControl(const ControlDescriptor& changedFields) override;
    void SetupSpinBoxBase();
    void UpdateRange();

    void ValueChanged(TEditableType val);

    void ToEditingState();
    void ToInvalidState();
    void ToValidState();

    virtual bool FromText(const QString& input, TEditableType& output) const = 0;
    virtual QString ToText(const TEditableType output) const = 0;
    virtual bool IsEqualValue(TEditableType v1, TEditableType v2) const = 0;
    virtual QValidator::State TypeSpecificValidate(const QString& str) const = 0;

    void fixup(QString& str) const override;
    QValidator::State validate(QString& input, int& pos) const override;

protected:
    QtConnections connections;
    QString noValueString = QString(MultipleValuesString);

    enum class ControlState
    {
        ValidValue,
        InvalidValue,
        Editing
    };

    Stack<ControlState> stateHistory;

private:
    QString textFromValue(TEditableType val) const override;
    TEditableType valueFromText(const QString& text) const override;
    void keyPressEvent(QKeyEvent* event) override;
    void focusInEvent(QFocusEvent* event) override;
    void focusOutEvent(QFocusEvent* event) override;
};

} // namespace TArc
} // namespace DAVA