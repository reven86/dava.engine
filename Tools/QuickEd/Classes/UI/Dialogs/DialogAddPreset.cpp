#include "DialogAddPreset.h"
#include "EditorCore.h"

#include <QCompleter>

using namespace DAVA;

DialogAddPreset::DialogAddPreset(const QString& originalPresetNameArg, QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    comboBox_baseFontPresetName->addItem("");
    comboBox_baseFontPresetName->addItems(GetEditorFontSystem()->GetDefaultPresetNames());
    comboBox_baseFontPresetName->setCurrentText(originalPresetNameArg);
    lineEdit_newFontPresetName->setText(originalPresetNameArg);

    lineEdit_newFontPresetName->setCompleter(new QCompleter(GetEditorFontSystem()->GetDefaultPresetNames(), lineEdit_newFontPresetName));

    connect(lineEdit_newFontPresetName, &QLineEdit::textChanged, this, &DialogAddPreset::OnNewPresetNameChanged);
    connect(comboBox_baseFontPresetName, &QComboBox::currentTextChanged, this, &DialogAddPreset::OnNewPresetNameChanged);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &DialogAddPreset::reject);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &DialogAddPreset::OnAccept);

    OnNewPresetNameChanged();
}

void DialogAddPreset::OnNewPresetNameChanged()
{
    QString baseName = comboBox_baseFontPresetName->currentText();
    QString newName = lineEdit_newFontPresetName->text();
    QString resultText;
    bool enabled = false;
    if (newName.isEmpty())
    {
        enabled = false;
        resultText = tr("enter new preset name");
    }
    else if (baseName == newName)
    {
        enabled = false;
        resultText = tr("names match!");
    }
    else if (GetEditorFontSystem()->GetDefaultPresetNames().contains(newName))
    {
        enabled = false;
        resultText = tr("This preset name already exists in the system");
    }
    else
    {
        enabled = true;
        resultText = tr("New font preset will be created");
    }
    label_info->setText(resultText);
    buttonBox->button(QDialogButtonBox::Ok)->setEnabled(enabled);
}

void DialogAddPreset::OnAccept()
{
    auto editorFontSystem = GetEditorFontSystem();
    if (!editorFontSystem->GetDefaultPresetNames().contains(lineEdit_newFontPresetName->text()))
    {
        editorFontSystem->CreateNewPreset(comboBox_baseFontPresetName->currentText().toStdString(), lineEdit_newFontPresetName->text().toStdString());
        editorFontSystem->SaveLocalizedFonts();
    }
    accept();
}