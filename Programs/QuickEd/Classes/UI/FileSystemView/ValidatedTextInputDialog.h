#ifndef __QUICKED_VALIDATED_TEXT_INPUT_DIALOG_H__
#define __QUICKED_VALIDATED_TEXT_INPUT_DIALOG_H__

#include <QInputDialog>
#include <functional>

class QLineEdit;
class QPushButton;

class ValidatedTextInputDialog : public QInputDialog
{
    Q_OBJECT

public:
    ValidatedTextInputDialog(QWidget* parent = nullptr);
    void SetValidator(std::function<bool(const QString&)> validator);
    void SetWarningMessage(const QString& message);
    void setLabelText(const QString& labelText);
private slots:
    void OnTextChanged(const QString& text);

private:
    using QInputDialog::setLabelText;
    void ExtractInternalWidgets();

    QLineEdit* lineEdit = nullptr;
    QPushButton* buttonOk = nullptr;
    std::function<bool(const QString&)> validateFunction;
    QString warningMessage;
    QString labelText;
};

#endif //__QUICKED_VALIDATED_TEXT_INPUT_DIALOG_H__