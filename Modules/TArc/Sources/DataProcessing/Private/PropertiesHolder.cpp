#include "DataProcessing/PropertiesHolder.h"
#include "Base/Any.h"

#include <QJsonDocument>
#include <QJsonObject>
#include <QJSonValue>
#include <QJsonArray>

namespace DAVA
{
namespace TArc
{
struct PropertiesHolder::Impl
{
    Impl(const String &name_)
        : name(QString::fromStdString(name_))
    {
        //read document;
    }

    Impl(Impl *impl_, const String &name_)
        : name(QString::fromStdString(name_))
        , parent(impl_)
    {
        jsonObject = parent->jsonObject[name].toObject();
    }

    ~Impl()
    {
        if(parent != nullptr)
        {
            if (wasChanged)
            {
                parent->jsonObject[name] = jsonObject;
            }
        }
        else
        {
            //save document
        }
    }

    void Save(const Any &value, const String &key)
    {
        wasChanged = true;
        //to jsonValue
    }

    Any Load(const String &key, const Any& defaultValue)
    {
        auto iter = jsonObject.find(QString::fromStdString(key));
        if (iter == jsonObject.end())
        {
            return defaultValue;
        }
        //from jsonValue
    }

private:
    QString name;
    Impl *parent = nullptr;
    QJsonObject jsonObject;
    bool wasChanged = false;
    
};

PropertiesHolder::PropertiesHolder(const String &projectName)
    : impl(new Impl(projectName))
{

}

PropertiesHolder::~PropertiesHolder() = default;


PropertiesHolder::PropertiesHolder(PropertiesHolder &&holder)
    : impl(std::move(holder.impl))
{
}

PropertiesHolder& PropertiesHolder::operator = (PropertiesHolder &&holder)
{
    if (this != &holder)
    {
        impl = std::move(holder.impl);
    }
    return *this;
}

PropertiesHolder PropertiesHolder::SubHolder(const String &holderName) const
{
    return PropertiesHolder(*this, holderName);
}

PropertiesHolder::PropertiesHolder(const PropertiesHolder &parent, const String &name)
    : impl(new Impl(parent.impl.get(), name))
{

}

void PropertiesHolder::Save(const Any &value, const String &key)
{
    impl->Save(value, key);
}

Any PropertiesHolder::Load(const String &key, const Any& defaultValue) const
{
    return impl->Load(key, defaultValue);
}

Any PropertiesHolder::Load(const String &key) const
{
    return impl->Load(key, Any());
}

} // namespace TArc
} // namespace DAVA
