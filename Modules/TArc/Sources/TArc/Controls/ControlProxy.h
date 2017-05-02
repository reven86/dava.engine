#pragma once

#include "TArc/Core/ContextAccessor.h"
#include "TArc/DataProcessing/DataListener.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/Controls/ControlDescriptor.h"

#include <QWidget>
#include <QObject>

namespace DAVA
{
namespace TArc
{
class ControlProxy
{
public:
    virtual ~ControlProxy() = default;

    virtual void ForceUpdate() = 0;
    virtual void TearDown() = 0;
    virtual QWidget* ToWidgetCast() = 0;
};

template <typename TBase>
class ControlProxyImpl : protected TBase, public ControlProxy, protected DataListener
{
public:
    ControlProxyImpl(const ControlDescriptor& descriptor_, DataWrappersProcessor* wrappersProcessor, Reflection model_, QWidget* parent)
        : TBase(parent)
        , descriptor(descriptor_)
        , model(model_)
    {
        SetupControl(wrappersProcessor);
    }

    ControlProxyImpl(const ControlDescriptor& descriptor_, ContextAccessor* accessor, Reflection model_, QWidget* parent)
        : TBase(parent)
        , descriptor(descriptor_)
        , model(model_)
    {
        SetupControl(accessor);
    }

    ~ControlProxyImpl() override
    {
        wrapper.SetListener(nullptr);
    }

    void SetObjectName(const QString& objName)
    {
        TBase::setObjectName(objName);
    }

    QWidget* ToWidgetCast() override
    {
        return this;
    }

    void ForceUpdate() override
    {
        OnDataChanged(wrapper, Vector<Any>());
    }

    void TearDown() override
    {
        wrapper.SetListener(nullptr);
        wrapper = DataWrapper();
    }

protected:
    template <typename TPrivate>
    ControlProxyImpl(const ControlDescriptor& descriptor_, DataWrappersProcessor* wrappersProcessor, Reflection model_, TPrivate&& d, QWidget* parent)
        : TBase(std::move(d), parent)
        , descriptor(descriptor_)
        , model(model_)
    {
        SetupControl(wrappersProcessor);
    }

    template <typename TPrivate>
    ControlProxyImpl(const ControlDescriptor& descriptor_, ContextAccessor* accessor, Reflection model_, TPrivate&& d, QWidget* parent)
        : TBase(std::move(d), parent)
        , descriptor(descriptor_)
        , model(model_)
    {
        SetupControl(accessor);
    }

    void SetupControl(DataWrappersProcessor* wrappersProcessor)
    {
        wrapper = wrappersProcessor->CreateWrapper(MakeFunction(this, &ControlProxyImpl<TBase>::GetModel), nullptr);
        wrapper.SetListener(this);
    }

    void SetupControl(ContextAccessor* accessor)
    {
        wrapper = accessor->CreateWrapper(MakeFunction(this, &ControlProxyImpl<TBase>::GetModel));
        wrapper.SetListener(this);
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

    template <typename Enum>
    FastName GetFieldName(Enum fieldMark) const
    {
        return descriptor.fieldNames[static_cast<size_t>(fieldMark)].name;
    }

    virtual void UpdateControl(const ControlDescriptor& descriptor) = 0;

    template <typename Enum>
    bool IsValueReadOnly(const ControlDescriptor& descriptor, Enum valueRole, Enum readOnlyRole) const
    {
        DAVA::Reflection fieldValue = model.GetField(descriptor.GetName(valueRole));
        DVASSERT(fieldValue.IsValid());

        bool readOnlyFieldValue = false;
        FastName readOnlyFieldName = descriptor.GetName(readOnlyRole);
        if (readOnlyFieldName.IsValid())
        {
            DAVA::Reflection fieldReadOnly = model.GetField(readOnlyFieldName);
            if (fieldReadOnly.IsValid())
            {
                readOnlyFieldValue = fieldReadOnly.GetValue().Cast<bool>();
            }
        }

        return fieldValue.IsReadonly() == true ||
        fieldValue.GetMeta<DAVA::M::ReadOnly>() != nullptr ||
        readOnlyFieldValue == true;
    }

    template <typename CastType, typename Enum>
    CastType GetFieldValue(Enum role, const CastType& defaultValue) const
    {
        const FastName& fieldName = GetFieldName(role);
        if (fieldName.IsValid() == true)
        {
            DAVA::Reflection field = model.GetField(fieldName);
            if (field.IsValid())
            {
                return field.GetValue().Cast<CastType>(defaultValue);
            }
        }

        return defaultValue;
    }

protected:
    Reflection model;
    DataWrapper wrapper;

private:
    ControlDescriptor descriptor;
};

} // namespace TArc
} // namespace DAVA