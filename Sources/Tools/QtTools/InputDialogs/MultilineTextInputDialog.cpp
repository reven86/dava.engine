#include "QtTools/InputDialogs/MultilineTextInputDialog.h"

#include <QEvent>
#include <QShowEvent>
#include <QKeyEvent>

bool MultilineTextInputDialog::eventFilter(QObject* obj, QEvent* event)
{
    if (event->type() == QEvent::KeyPress)
    {
        QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);
        int key = keyEvent->key();
        Qt::KeyboardModifiers modifiers = keyEvent->modifiers();
        if ((key == Qt::Key_Enter || key == Qt::Key_Return) && (modifiers & Qt::CTRL || modifiers & Qt::ALT))
        {
            accept();
        }
    }
    return false;
}

QString MultilineTextInputDialog::GetMultiLineText(QWidget* parent, const QString& title, const QString& label, const QString& text /*= QString()*/, bool* ok /*= Q_NULLPTR*/, Qt::WindowFlags flags /*= Qt::WindowFlags()*/, Qt::InputMethodHints inputMethodHints /*= Qt::ImhNone*/)
{
    MultilineTextInputDialog dialog(parent, flags);
    dialog.setOptions(QInputDialog::UsePlainTextEditForTextInput);
    dialog.setWindowTitle(title);
    dialog.setLabelText(label);
    dialog.setTextValue(text);
    dialog.setInputMethodHints(inputMethodHints);

    for (QObject* child : dialog.children())
    {
        child->installEventFilter(&dialog);
    }

    int ret = dialog.exec();
    bool isAccepted = ret == (QDialog::Accepted);
    if (ok)
        *ok = isAccepted;
    if (isAccepted)
    {
        return dialog.textValue();
    }
    else
    {
        return QString();
    }
}

MultilineTextInputDialog::MultilineTextInputDialog(QWidget* parent /*= nullptr*/, Qt::WindowFlags flags /*= Qt::WindowFlags()*/)
    : QInputDialog(parent, flags)
{
}
