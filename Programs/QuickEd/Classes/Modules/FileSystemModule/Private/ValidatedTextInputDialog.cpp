#include "Debug/DVAssert.h"
#include "ValidatedTextInputDialog.h"
#include <QPushButton>
#include <QDialogButtonBox>
#include <QApplication>

ValidatedTextInputDialog::ValidatedTextInputDialog(QWidget* parent)
    : QInputDialog(parent)
{
    setTextEchoMode(QLineEdit::Normal);
    ExtractInternalWidgets();

    connect(this, &QInputDialog::textValueChanged, this, &ValidatedTextInputDialog::OnTextChanged);
}

void ValidatedTextInputDialog::SetValidator(std::function<bool(const QString&)> validator)
{
    validateFunction = validator;
}

void ValidatedTextInputDialog::SetWarningMessage(const QString& message)
{
    warningMessage = message;
}

void ValidatedTextInputDialog::setLabelText(const QString& arg)
{
    labelText = arg;
    QInputDialog::setLabelText(arg);
}

void ValidatedTextInputDialog::OnTextChanged(const QString& text)
{
    QPalette palette(lineEdit->palette());
    bool enabled = true;
    if (text.isEmpty() || !validateFunction(text))
    {
        QInputDialog::setLabelText(warningMessage);
        palette.setColor(QPalette::Text, Qt::red);
        enabled = false;
    }
    else
    {
        QInputDialog::setLabelText(labelText);
        QColor globalTextColor = qApp->palette().color(QPalette::Text);
        palette.setColor(QPalette::Text, globalTextColor);
        enabled = true;
    }
    lineEdit->setPalette(palette);
    buttonOk->setEnabled(enabled);
}

void ValidatedTextInputDialog::ExtractInternalWidgets()
{
    okButtonText(); //force ensure layout of dialog

    const QObjectList& children = this->children();
    auto iter = std::find_if(children.begin(), children.end(), [](const QObject* obj) {
        return qobject_cast<const QLineEdit*>(obj) != nullptr;
    });
    if (iter == children.end())
    {
        DVASSERT(false && "ValidatedTextInputDialog: can not find line edit");
        return;
    }
    lineEdit = qobject_cast<QLineEdit*>(*iter);
    iter = std::find_if(children.begin(), children.end(), [](const QObject* obj) {
        return qobject_cast<const QDialogButtonBox*>(obj) != nullptr;
    });
    if (iter == children.end())
    {
        DVASSERT(false && "ValidatedTextInputDialog: can not find button box");
        return;
    }
    QDialogButtonBox* buttonBox = qobject_cast<QDialogButtonBox*>(*iter);
    buttonOk = buttonBox->button(QDialogButtonBox::Ok);
}
