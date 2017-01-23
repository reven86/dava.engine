#pragma once

#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataListener.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"

#include <QWidget>

namespace DAVA
{
namespace TArc
{
template <typename TBase>
class ControlProxy : protected TBase, protected DataListener
{
public:
    ControlProxy(DataWrappersProcessor* wrappersProcessor, Reflection model_, QWidget* parent)
        : TBase(parent)
        , model(model_)
    {
        wrapper = wrappersProcessor->CreateWrapper(MakeFunction(this, &ControlProxy<TBase>::GetModel), nullptr);
        wrapper.SetListener(this);
    }

    ControlProxy(ContextAccessor* accessor, Reflection model_, QWidget* parent)
        : TBase(parent)
        , model(model_)
    {
        wrapper = accessor->CreateWrapper(MakeFunction(this, &ControlProxy<TBase>::GetModel));
        wrapper.SetListener(this);
    }

    ~ControlProxy()
    {
        wrapper.SetListener(nullptr);
    }

    QWidget* ToWidgetCast()
    {
        return this;
    }

protected:
    Reflection GetModel(const DataContext* ctx) const
    {
        return model;
    }

protected:
    Reflection model;
    DataWrapper wrapper;
};

} // namespace TArc
} // namespace DAVA