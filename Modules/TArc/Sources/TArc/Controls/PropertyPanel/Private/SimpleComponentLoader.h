#pragma once

class QQmlComponent;
class QQmlEngine;
class QUrl;

namespace DAVA
{
namespace TArc
{
class SimpleComponentLoader
{
public:
    SimpleComponentLoader(QQmlEngine* engine, const QUrl& componentUrl);
    QQmlComponent* GetComponent();

private:
    QQmlComponent* component = nullptr;
};

} // namespace TArc
} // namespace DAVA