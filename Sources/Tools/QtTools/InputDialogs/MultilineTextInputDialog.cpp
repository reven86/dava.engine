#include "QtTools/InputDialogs/MultilineTextInputDialog.h"
#include "Preferences/PreferencesStorage.h"
#include "Preferences/PreferencesRegistrator.h"

#include <QEvent>
#include <QShowEvent>
#include <QKeyEvent>

namespace MultilineTextInputDialogDetails
{
const DAVA::FastName settingsKey("multilineTextInputDialogGeometry");
GlobalValuesRegistrator registrator(settingsKey, DAVA::VariantType(DAVA::String()));
}

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

    DAVA::String geometryStr = PreferencesStorage::Instance()->GetValue(MultilineTextInputDialogDetails::settingsKey).AsString();
    QByteArray geometry = QByteArray::fromStdString(geometryStr);
    dialog.restoreGeometry(QByteArray::fromBase64(geometry));

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

    geometry = dialog.saveGeometry().toBase64();
    PreferencesStorage::Instance()->SetValue(MultilineTextInputDialogDetails::settingsKey, DAVA::VariantType(geometry.toStdString()));

    bool isAccepted = (ret == QDialog::Accepted);
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
