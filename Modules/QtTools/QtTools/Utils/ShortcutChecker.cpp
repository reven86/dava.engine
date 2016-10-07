#include "ShortcutChecker.h"

#include "Logger/Logger.h"

#include "QtTools/Utils/DavaQtKeyboard.h"

#include <QObject>
#include <QKeyEvent>
#include <QShortcut>
#include <QAction>
#include <QMenu>

namespace ShortcutCheckerDetail
{
QList<QKeySequence> GetSequences(const QShortcut* shortcut)
{
    QList<QKeySequence> sequences;
    sequences.push_back(shortcut->key());
    return sequences;
}

QList<QKeySequence> GetSequences(const QAction* action)
{
    return action->shortcuts();
}

bool SkipMatchedShortcut(const QShortcut* /*shortcut*/, const QKeyEvent* /*event*/)
{
    return false;
}

bool SkipMatchedShortcut(const QAction* action, const QKeyEvent* event)
{
    // we should skip actions that have ControlModifier and associated with menu, because
    // operation system will trigger this action anyway
    Qt::KeyboardModifiers mod = event->modifiers();
    foreach (QWidget* w, action->associatedWidgets())
    {
        if (qobject_cast<QMenu*>(w) != nullptr && mod == Qt::ControlModifier)
        {
            return true;
        }
    }

    return false;
}

void TriggerAction(QShortcut* shortcut)
{
    shortcut->activated();
}

void TriggerAction(QAction* action)
{
    action->trigger();
}
}

ShortcutChecker::ShortcutChecker(QObject* shortcutsContainer_)
    : shortcutsContainer(shortcutsContainer_)
{
    keyTranslateTable[1049] = 81;
    keyTranslateTable[1062] = 87;
    keyTranslateTable[1059] = 69;
    keyTranslateTable[1050] = 82;
    keyTranslateTable[1045] = 84;
    keyTranslateTable[1053] = 89;
    keyTranslateTable[1043] = 85;
    keyTranslateTable[1064] = 73;
    keyTranslateTable[1065] = 79;
    keyTranslateTable[1047] = 80;
    keyTranslateTable[1064] = 91;
    keyTranslateTable[1066] = 93;
    keyTranslateTable[1060] = 65;
    keyTranslateTable[1067] = 83;
    keyTranslateTable[1042] = 68;
    keyTranslateTable[1040] = 70;
    keyTranslateTable[1055] = 71;
    keyTranslateTable[1056] = 72;
    keyTranslateTable[1054] = 74;
    keyTranslateTable[1051] = 75;
    keyTranslateTable[1044] = 76;
    keyTranslateTable[1046] = 59;
    keyTranslateTable[1069] = 39;
    keyTranslateTable[1071] = 90;

    keyTranslateTable[1063] = 88;
    keyTranslateTable[1057] = 67;
    keyTranslateTable[1052] = 86;
    keyTranslateTable[1048] = 66;
    keyTranslateTable[1058] = 78;
    keyTranslateTable[1068] = 77;
    keyTranslateTable[1041] = 44;
    keyTranslateTable[1070] = 46;
    keyTranslateTable[46] = 47;
}

bool ShortcutChecker::TryCallShortcut(QKeyEvent* e)
{
    int key = e->key();
    auto iter = keyTranslateTable.find(key);
    if (iter != keyTranslateTable.end())
    {
        key = iter->second;
    }
    QKeySequence inputSequence(key + e->modifiers());
    DAVA::uint64 currentTimestamp = e->timestamp();

    // Qt can send seria of same events from different places.
    // TryShortcut, Menu handler and other.
    // We should skip events with same key sequence and almost same timestamp.
    // time stamp in milliseconds.
    // currentTimestamp can be zero, if event was sent by CocoaMenuDelegate.
    bool equalSequences = lastInputSequence == inputSequence;
    if (currentTimestamp == 0 || (equalSequences && currentTimestamp - lastShortcutTimestamp < 100))
    {
        return true;
    }

    if (TryCallShortcutImpl(inputSequence, e, shortcutsContainer->findChildren<QShortcut*>()))
    {
        return true;
    }

    return TryCallShortcutImpl(inputSequence, e, shortcutsContainer->findChildren<QAction*>());
}

template <typename T>
bool ShortcutChecker::TryCallShortcutImpl(const QKeySequence& inputSequence, QKeyEvent* event, const QList<T*>& actions)
{
    foreach (T* action, actions)
    {
        QList<QKeySequence> sequences = ShortcutCheckerDetail::GetSequences(action);
        foreach (QKeySequence seq, sequences)
        {
            if (seq.matches(inputSequence) == QKeySequence::ExactMatch && !ShortcutCheckerDetail::SkipMatchedShortcut(action, event))
            {
                lastInputSequence = inputSequence;
                lastShortcutTimestamp = event->timestamp();
                ShortcutCheckerDetail::TriggerAction(action);
                // in some cases we can get KeyPressed (Ctrl + D), but not get KeyUnpressed
                // to fix this we will clear keyboard state in Dava if we found shortcut
                DAVA::DavaQtKeyboard::ClearAllKeys();
                event->accept();
                return true;
            }
        }
    }

    return false;
}