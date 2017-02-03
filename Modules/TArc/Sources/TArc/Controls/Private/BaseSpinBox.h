#pragma once

#include "TArc/Controls/ControlProxy.h"
#include "TArc/Utils/QtConnections.h"

#include <QLineEdit>
#include <QAbstractSpinBox>
#include <QString>

namespace DAVA
{
namespace TArc
{
template <typename TBase>
class BaseSpinBox : public ControlProxy<TBase>
{
public:
    BaseSpinBox(const ControlDescriptor& descriptor, DataWrappersProcessor* wrappersProcessor, Reflection model, QWidget* parent)
        : ControlProxy<TBase>(descriptor, wrappersProcessor, model, parent)
    {
        static_assert(std::is_base_of<QAbstractSpinBox, TBase>::value, "TBase should be derived from QAbstractSpinBox");
    }

    BaseSpinBox(const ControlDescriptor& descriptor, ContextAccessor* accessor, Reflection model, QWidget* parent)
        : ControlProxy<TBase>(descriptor, accessor, model, parent)
    {
        static_assert(std::is_base_of<QAbstractSpinBox, TBase>::value, "TBase should be derived from QAbstractSpinBox");
    }

protected:
    void ToEditingState()
    {
        DVASSERT(hasFocus() == true);
        DVASSERT(stateHistory.top() != ControlState::Editing);

        ControlState prevState = stateHistory.top();
        stateHistory.push(ControlState::Editing);
        if (prevState == ControlState::InvalidValue)
        {
            lineEdit()->setText("");
            setButtonSymbols(QAbstractSpinBox::NoButtons);
        }
        else
        {
            setButtonSymbols(QAbstractSpinBox::UpDownArrows);
        }
    }

    void ToInvalidState()
    {
        stateHistory = Stack<ControlState>();
        stateHistory.push(ControlState::InvalidValue);
        setButtonSymbols(QAbstractSpinBox::NoButtons);
        lineEdit()->setText(textFromValue(0));
    }

    void ToValidState()
    {
        stateHistory = Stack<ControlState>();
        stateHistory.push(ControlState::ValidValue);
        setButtonSymbols(QAbstractSpinBox::UpDownArrows);
    }

protected:
    QtConnections connections;
    QString noValueString = QStringLiteral("<multiple values>");

    enum class ControlState
    {
        ValidValue,
        InvalidValue,
        Editing
    };

    Stack<ControlState> stateHistory;

private:
    void keyPressEvent(QKeyEvent* event) override
    {
        ControlProxy<TBase>::keyPressEvent(event);
        int key = event->key();
        if (key == Qt::Key_Enter || key == Qt::Key_Return)
        {
            emit valueChanged(value());
        }
    }

    void focusInEvent(QFocusEvent* event) override
    {
        ControlProxy<TBase>::focusInEvent(event);
        ToEditingState();
    }

    void focusOutEvent(QFocusEvent* event) override
    {
        ControlProxy<TBase>::focusOutEvent(event);
        if (stateHistory.top() == ControlState::Editing)
        {
            stateHistory.pop();
        }
        DVASSERT(stateHistory.empty() == false);
        DVASSERT(stateHistory.top() == ControlState::InvalidValue || stateHistory.top() == ControlState::ValidValue);

        if (stateHistory.top() == ControlState::ValidValue)
        {
            ToValidState();
        }
        else
        {
            ToInvalidState();
        }
    }
};

} // namespace TArc
} // namespace DAVA