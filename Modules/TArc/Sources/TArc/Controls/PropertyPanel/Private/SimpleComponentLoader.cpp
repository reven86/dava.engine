#include "SimpleComponentLoader.h"

#include "Logger/Logger.h"

#include <QQmlComponent>
#include <QtGlobal>

namespace DAVA
{
namespace TArc
{
SimpleComponentLoader::SimpleComponentLoader(QQmlEngine* engine, const QUrl& componentUrl)
{
    component = new QQmlComponent(engine, componentUrl);
    if (component->isError())
    {
        DAVA::Logger::Warning("Couldn't load component %s", componentUrl.toString().toStdString().c_str());
        foreach (QQmlError error, component->errors())
        {
            DAVA::Logger::Warning("%s", error.toString().toStdString().c_str());
        }
    }
}

QQmlComponent* SimpleComponentLoader::GetComponent()
{
    return component;
}

} // namespace TArc
} // namespace DAVA
