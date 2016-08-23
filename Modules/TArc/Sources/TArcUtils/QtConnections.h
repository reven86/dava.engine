#pragma once

#include "Base/BaseTypes.h"

#include <QMetaObject>
#include <QPointer>

namespace DAVA
{
namespace TArc
{
class QtConnections
{
public:
    ~QtConnections()
    {
        for (QMetaObject::Connection& connection : connections)
        {
            QObject::disconnect(connection);
        }

        connections.clear();
    }

    template <typename Func1, typename Func2>
    void AddConnection(const typename QtPrivate::FunctionPointer<Func1>::Object* sender, Func1 signal, Func2 slot)
    {
        connections.push_back(QObject::connect(sender, signal, slot));
    }

private:
    Vector<QMetaObject::Connection> connections;
};
} // namespace TArc
} // namespace DAVA
