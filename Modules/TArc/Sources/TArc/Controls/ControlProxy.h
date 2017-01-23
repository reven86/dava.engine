#pragma once

#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataListener.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Controls/ControlDescriptor.h"

#include <QWidget>

namespace DAVA
{
namespace TArc
{
template <typename TBase>
class ControlProxy : protected TBase, protected DataListener
{
public:
    ControlProxy(const ControlDescriptor& descriptor, DataWrappersProcessor* wrappersProcessor, Reflection model_, QWidget* parent)
        : TBase(parent)
        , model(model_)
        , descriptor(descriptor)
    {
        wrapper = wrappersProcessor->CreateWrapper(MakeFunction(this, &ControlProxy<TBase>::GetModel), nullptr);
        wrapper.SetListener(this);
    }

    ControlProxy(const ControlDescriptor& descriptor, ContextAccessor* accessor, Reflection model_, QWidget* parent)
        : TBase(parent)
        , model(model_)
        , descriptor(descriptor)
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
    void OnDataChanged(const DataWrapper& wrapper, const Vector<Any>& fields) override
    {
        DVASSERT(wrapper.HasData());

        if (fields.empty())
        {
            for (ControlDescriptor::Field& f : descriptor.fieldNames)
            {
                if (f.name.IsValid())
                {
                    f.isChanged = true;
                }
            }
        }
        else
        {
            for (const Any& anyFieldName : fields)
            {
                FastName name = anyFieldName.Cast<FastName>();
                auto iter = std::find_if(descriptor.fieldNames.begin(), descriptor.fieldNames.end(), [&name](const ControlDescriptor::Field& f)
                                         {
                                             return f.name == name;
                                         });

                if (iter != descriptor.fieldNames.end())
                {
                    iter->isChanged = true;
                }
            }
        }

        UpdateControl(descriptor);
        for (ControlDescriptor::Field& f : descriptor.fieldNames)
        {
            f.isChanged = false;
        }
    }

    Reflection GetModel(const DataContext* ctx) const
    {
        return model;
    }

    FastName GetFieldName(uint32 fieldMark)
    {
        return descriptor.fieldNames[fieldMark].name;
    }

    virtual void UpdateControl(const ControlDescriptor& descriptor) = 0;

protected:
    Reflection model;
    DataWrapper wrapper;

private:
    ControlDescriptor descriptor;
};

} // namespace TArc
} // namespace DAVA