#pragma once

#include <Functional/Function.h>
#include <QObject>

class QtDelayedExecutor : public QObject
{
public:
    QtDelayedExecutor(QObject* parent = nullptr);

    void DelayedExecute(const DAVA::Function<void()>& functor);

protected:
    bool event(QEvent* e) override;
};
