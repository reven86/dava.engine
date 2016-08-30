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
        : name(name_)
    {

    }

    Impl(const Impl &impl_, const String &name_)
        : name(name_)
    {

    }

    Impl(const Impl &&impl)
    {
        name = std::move(impl.name);
    }
    String name;
    QJsonObject data;
    QJSonValueRef valueRef;
};

PropertiesHolder::PropertiesHolder(const String &projectName)
    : impl(new Impl(projectName))
{

}

PropertiesHolder::PropertiesHolder(const PropertiesHolder &parent, const String &name)
    : impl(new Impl(parent.impl.get(), name))
{

}

PropertiesHolder::~PropertiesHolder() = default;

PropertiesHolder::PropertiesHolder(const PropertiesHolder &holder)
    : impl(holder.impl)
{
}

PropertiesHolder::PropertiesHolder(PropertiesHolder &&holder)
    : impl(std::move(holder.impl))
{
}

PropertiesHolder& PropertiesHolder::operator = (const PropertiesHolder &holder)
{
}

PropertiesHolder& PropertiesHolder::operator = (PropertiesHolder &&holder)
{
}

PropertiesHolder PropertiesHolder::SubHolder(const String &holderName) const
{
    return PropertiesHolder(*this, holderName);
}

void PropertiesHolder::Save(const Any &value, const String &key)
{

}

Any PropertiesHolder::Load(const String &key, const Any& defaultValue) const
{
    return Any();
}

Any PropertiesHolder::Load(const String &key) const
{
    return Any();
}

} // namespace TArc
} // namespace DAVA
