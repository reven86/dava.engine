#pragma once

#include "Base/BaseTypes.h"
#include <QKeySequence>

//QT_BUG this file is created to prevent Qt bug https://bugreports.qt.io/browse/QTBUG-50865.
//QT_BUG recheck it after change Qt version to 5.7 or later

class QObject;
class QKeyEvent;
class QtDelayedExecutor;

class ShortcutChecker
{
public:
    ShortcutChecker(QObject* shortcutsContainer);
    ~ShortcutChecker();

    bool TryCallShortcut(QKeyEvent* event);

private:
    template <typename T>
    bool TryCallShortcutImpl(const QKeySequence& inputSequence, QKeyEvent* event, const QList<T*>& actions);

private:
    QObject* shortcutsContainer;
    DAVA::UnorderedMap<int, int> keyTranslateTable;

    QKeySequence lastInputSequence;
    DAVA::uint64 lastShortcutTimestamp = 0;
    std::unique_ptr<QtDelayedExecutor> delayedExecutor;
};