#include "DialogConfigurePreset.h"

#include "Render/2D/FTFont.h"
#include "Helpers/ResourcesManageHelper.h"
#include "FileSystem/LocalizationSystem.h"
#include "Project/EditorFontSystem.h"
#include "ui_DialogConfigurePreset.h"
#include <QFileInfo>
#include <QMessageBox>

using namespace DAVA;

DialogConfigurePreset::DialogConfigurePreset(EditorFontSystem* editorFontSystem_, const QString& originalPresetName_, QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::DialogConfigurePreset())
    , originalPresetName(originalPresetName_)
    , editorFontSystem(editorFontSystem_)
{
    ui->setupUi(this);
    ui->pushButton_resetLocale->setIcon(QIcon(":/Icons/edit_undo.png"));
    ui->pushButton_resetLocale->setToolTip(tr("Reset font for locale"));

    ui->lineEdit_currentFontPresetName->setText(originalPresetName);
    QStringList fontsList = ResourcesManageHelper::GetFontsList();
    ui->comboBox_defaultFont->addItems(fontsList);
    ui->comboBox_localizedFont->addItems(fontsList);

    ui->comboBox_locale->addItems(editorFontSystem->GetAvailableFontLocales());

    ui->comboBox_locale->setCurrentText(QString::fromStdString(LocalizationSystem::Instance()->GetCurrentLocale()));

    initPreset();

    connect(ui->comboBox_defaultFont, &QComboBox::currentTextChanged, this, &DialogConfigurePreset::OnDefaultFontChanged);
    connect(ui->spinBox_defaultFontSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &DialogConfigurePreset::OnDefaultFontSizeChanged);
    connect(ui->comboBox_localizedFont, &QComboBox::currentTextChanged, this, &DialogConfigurePreset::OnLocalizedFontChanged);
    connect(ui->spinBox_localizedFontSize, static_cast<void (QSpinBox::*)(int)>(&QSpinBox::valueChanged), this, &DialogConfigurePreset::OnLocalizedFontSizeChanged);
    connect(ui->comboBox_locale, &QComboBox::currentTextChanged, this, &DialogConfigurePreset::OnCurrentLocaleChanged);
    connect(ui->pushButton_resetLocale, &QPushButton::clicked, this, &DialogConfigurePreset::OnResetLocale);
    connect(ui->pushButton_applyDefaultToAllLocales, &QPushButton::clicked, this, &DialogConfigurePreset::OnApplyToAllLocales);
    connect(ui->pushButton_ok, &QPushButton::clicked, this, &DialogConfigurePreset::OnOk);
    connect(ui->pushButton_cancel, &QPushButton::clicked, this, &DialogConfigurePreset::OnCancel);
}

DialogConfigurePreset::~DialogConfigurePreset() = default;

void DialogConfigurePreset::initPreset()
{
    UpdateDefaultFontWidgets();
    UpdateLocalizedFontWidgets();
}

void DialogConfigurePreset::OnDefaultFontChanged(const QString& arg)
{
    SetFont(arg, ui->spinBox_defaultFontSize->value(), editorFontSystem->GetDefaultFontLocale());
}

void DialogConfigurePreset::OnDefaultFontSizeChanged(int size)
{
    SetFont(ui->comboBox_defaultFont->currentText(), size, editorFontSystem->GetDefaultFontLocale());
}

void DialogConfigurePreset::OnLocalizedFontChanged(const QString& arg)
{
    SetFont(arg, ui->spinBox_localizedFontSize->value(), ui->comboBox_locale->currentText());
}

void DialogConfigurePreset::OnLocalizedFontSizeChanged(int size)
{
    SetFont(ui->comboBox_localizedFont->currentText(), size, ui->comboBox_locale->currentText());
}

void DialogConfigurePreset::OnCurrentLocaleChanged(const QString& arg)
{
    UpdateLocalizedFontWidgets();
}

void DialogConfigurePreset::OnResetLocale()
{
    SetFont(ui->comboBox_defaultFont->currentText(), ui->spinBox_defaultFontSize->value(), ui->comboBox_locale->currentText());
    UpdateLocalizedFontWidgets();
}

void DialogConfigurePreset::OnApplyToAllLocales()
{
    for (const auto& locale : editorFontSystem->GetAvailableFontLocales())
    {
        SetFont(ui->comboBox_defaultFont->currentText(), ui->spinBox_defaultFontSize->value(), locale);
        UpdateLocalizedFontWidgets();
    }
}

void DialogConfigurePreset::OnOk()
{
    editorFontSystem->SaveLocalizedFonts();
    accept();
}

void DialogConfigurePreset::OnCancel()
{
    editorFontSystem->LoadLocalizedFonts();
    reject();
}

void DialogConfigurePreset::UpdateDefaultFontWidgets()
{
    ui->spinBox_defaultFontSize->blockSignals(true);
    ui->comboBox_defaultFont->blockSignals(true);
    Font* font = editorFontSystem->GetFont(ui->lineEdit_currentFontPresetName->text().toStdString(), editorFontSystem->GetDefaultFontLocale());
    ui->spinBox_defaultFontSize->setValue(font->GetSize());

    DVASSERT(font->GetFontType() == Font::TYPE_FT);
    FTFont* ftFont = static_cast<FTFont*>(font);
    QFileInfo fileInfo(QString::fromStdString(ftFont->GetFontPath().GetFrameworkPath()));
    ui->comboBox_defaultFont->setCurrentText(fileInfo.fileName());
    ui->spinBox_defaultFontSize->blockSignals(false);
    ui->comboBox_defaultFont->blockSignals(false);
}

void DialogConfigurePreset::UpdateLocalizedFontWidgets()
{
    ui->spinBox_localizedFontSize->blockSignals(true);
    ui->comboBox_localizedFont->blockSignals(true);
    Font* font = editorFontSystem->GetFont(ui->lineEdit_currentFontPresetName->text().toStdString(), ui->comboBox_locale->currentText().toStdString());
    ui->spinBox_localizedFontSize->setValue(font->GetSize());

    DVASSERT(font->GetFontType() == Font::TYPE_FT);
    FTFont* ftFont = static_cast<FTFont*>(font);
    QFileInfo fileInfo = QFileInfo(QString::fromStdString(ftFont->GetFontPath().GetFrameworkPath()));
    ui->comboBox_localizedFont->setCurrentText(fileInfo.fileName());
    ui->spinBox_localizedFontSize->blockSignals(false);
    ui->comboBox_localizedFont->blockSignals(false);
}

void DialogConfigurePreset::SetFont(const QString& fontType, const int fontSize, const QString& locale)
{
    QString fontPath = ResourcesManageHelper::GetFontRelativePath(fontType);
    Font* font = FTFont::Create(fontPath.toStdString());
    if (nullptr == font)
    {
        QMessageBox::warning(this, tr("Font creation error"), tr("Can not create font from %1").arg(fontPath));
        return;
    }
    font->SetSize(fontSize);
    editorFontSystem->SetFont(ui->lineEdit_currentFontPresetName->text().toStdString(), locale.toStdString(), font);
}
