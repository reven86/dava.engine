#pragma once

#include "Base/BaseTypes.h"
#include <QKeySequence>

class QObject;
class QKeyEvent;

class ShortcutChecker
{
public:
    ShortcutChecker(QObject* shortcutsContainer);

    bool TryCallShortcut(QKeyEvent* event);

private:
    template <typename T>
    bool TryCallShortcutImpl(const QKeySequence& inputSequence, QKeyEvent* event, const QList<T*>& actions);

private:
    QObject* shortcutsContainer;
    DAVA::UnorderedMap<int, int> keyTranslateTable;

    QKeySequence lastInputSequence;
    DAVA::uint64 lastShortcutTimestamp = 0;
};