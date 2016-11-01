#include "preferencesdialog.h"
#include "filemanager.h"
#include "configdownloader.h"
#include "errormessenger.h"

#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>

namespace PreferencesDialogDetails
{
const char* propertyKey = "urlType";
const char* settingsFileName = "LauncherPreferences.json";
const char* filesDirectoryKey = "storage path";
const QMap<ConfigDownloader::eURLType, QString> urlKeys = {
    { ConfigDownloader::LauncherInfoURL, "launcherInfo url" },
    { ConfigDownloader::StringsURL, "launcher strings url" },
    { ConfigDownloader::FavoritesURL, "favorites url" },
    { ConfigDownloader::AllBuildsURL, "all builds url" }
};
}

void PreferencesDialog::ShowPreferencesDialog(FileManager* fileManager, ConfigDownloader* configDownloader, QWidget* parent)
{
    PreferencesDialog dialog(parent);
    dialog.Init(fileManager, configDownloader);
    if (dialog.exec() == QDialog::Accepted)
    {
        dialog.AcceptData();
    }
}

void PreferencesDialog::SavePreferences(FileManager* fileManager, ConfigDownloader* configDownloader)
{
    QJsonObject rootObject;
    rootObject[PreferencesDialogDetails::filesDirectoryKey] = fileManager->GetFilesDirectory();
    for (auto iter = PreferencesDialogDetails::urlKeys.cbegin(); iter != PreferencesDialogDetails::urlKeys.cend(); ++iter)
    {
        rootObject[iter.value()] = configDownloader->GetURL(iter.key());
    }
    QJsonDocument document(rootObject);
    QString filePath = FileManager::GetDocumentsDirectory() + PreferencesDialogDetails::settingsFileName;
    QFile settingsFile(filePath);
    if (settingsFile.open(QFile::WriteOnly))
    {
        settingsFile.write(document.toJson());
    }
    else
    {
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_FILE, QObject::tr("Can not write to file %1").arg(filePath));
    }
}

void PreferencesDialog::LoadPreferences(FileManager* fileManager, ConfigDownloader* configDownloader)
{
    QString filePath = FileManager::GetDocumentsDirectory() + PreferencesDialogDetails::settingsFileName;
    QFile settingsFile(filePath);
    QJsonDocument document;
    if (settingsFile.exists())
    {
        if (settingsFile.open(QFile::ReadOnly))
        {
            QByteArray data = settingsFile.readAll();
            document = QJsonDocument::fromJson(data);
        }
        else
        {
            ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_FILE, QObject::tr("Can not open file %1").arg(filePath));
        }
    }

    QJsonObject rootObject = document.object();
    for (auto iter = PreferencesDialogDetails::urlKeys.cbegin(); iter != PreferencesDialogDetails::urlKeys.cend(); ++iter)
    {
        QJsonValue value = rootObject[iter.value()];
        if (value.isString())
        {
            configDownloader->SetURL(iter.key(), value.toString());
        }
    }
    QJsonValue filesDirValue = rootObject[PreferencesDialogDetails::filesDirectoryKey];
    if (filesDirValue.isString())
    {
        fileManager->SetFilesDirectory(filesDirValue.toString());
    }
}

PreferencesDialog::PreferencesDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    connect(pushButton_selectStorageDir, &QPushButton::clicked, this, &PreferencesDialog::OnButtonChooseFilesPathClicked);
    connect(lineEdit_launcherDataPath, &QLineEdit::textChanged, this, &PreferencesDialog::ProcessSaveButtonEnabled);
}

void PreferencesDialog::Init(FileManager* fileManager_, ConfigDownloader* configDownloader_)
{
    fileManager = fileManager_;
    configDownloader = configDownloader_;
    Q_ASSERT(fileManager != nullptr);
    Q_ASSERT(configDownloader != nullptr);

    lineEdit_launcherDataPath->setText(fileManager->GetFilesDirectory());

    urlWidgets[ConfigDownloader::LauncherInfoURL] = lineEdit_launcherInfoURL;
    urlWidgets[ConfigDownloader::StringsURL] = lineEdit_metaInfoURL;
    urlWidgets[ConfigDownloader::FavoritesURL] = lineEdit_favoritesURL;
    urlWidgets[ConfigDownloader::AllBuildsURL] = lineEdit_allBuildsURL;

    resetUrlWidgets[ConfigDownloader::LauncherInfoURL] = pushButton_resetLauncherInfoURL;
    resetUrlWidgets[ConfigDownloader::StringsURL] = pushButton_resetMetaInfoURL;
    resetUrlWidgets[ConfigDownloader::FavoritesURL] = pushButton_resetFavoritesURL;
    resetUrlWidgets[ConfigDownloader::AllBuildsURL] = pushButton_resetAllBuildsURL;

    for (int i = 0; i < ConfigDownloader::URLTypesCount; ++i)
    {
        ConfigDownloader::eURLType type = static_cast<ConfigDownloader::eURLType>(i);
        urlWidgets[type]->setProperty(PreferencesDialogDetails::propertyKey, i);
        urlWidgets[type]->setText(configDownloader->GetURL(type));
        resetUrlWidgets[type]->setProperty(PreferencesDialogDetails::propertyKey, i);

        connect(resetUrlWidgets[type], &QPushButton::clicked, this, &PreferencesDialog::OnButtonResetURLClicked);
        connect(urlWidgets[type], &QLineEdit::textChanged, this, &PreferencesDialog::ProcessSaveButtonEnabled);
    }
}

void PreferencesDialog::AcceptData()
{
    Q_ASSERT(fileManager != nullptr);
    Q_ASSERT(configDownloader != nullptr);

    fileManager->SetFilesDirectory(lineEdit_launcherDataPath->text());

    for (int i = 0; i < ConfigDownloader::URLTypesCount; ++i)
    {
        ConfigDownloader::eURLType type = static_cast<ConfigDownloader::eURLType>(i);
        configDownloader->SetURL(type, urlWidgets[type]->text());
    }
}

void PreferencesDialog::OnButtonChooseFilesPathClicked()
{
    QString newPath = QFileDialog::getExistingDirectory(parentWidget(), "Choose new directory", lineEdit_launcherDataPath->text());
    if (!newPath.isEmpty())
    {
        lineEdit_launcherDataPath->setText(newPath);
    }
}

void PreferencesDialog::ProcessSaveButtonEnabled()
{
    bool enabled = true;
    for (QLineEdit* lineEdit : urlWidgets)
    {
        if (lineEdit->text().isEmpty())
        {
            enabled = false;
            break;
        }
    }
    if (enabled)
    {
        QString path = lineEdit_launcherDataPath->text();
        QFileInfo fileInfo(path);
        enabled = fileInfo.isDir();
    }
    QPushButton* button = buttonBox->button(QDialogButtonBox::Save);
    Q_ASSERT(button != nullptr);
    button->setEnabled(enabled);
}

void PreferencesDialog::OnButtonResetURLClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    Q_ASSERT(button != nullptr);
    bool ok;
    int typeInt = button->property(PreferencesDialogDetails::propertyKey).toInt(&ok);
    ConfigDownloader::eURLType type = static_cast<ConfigDownloader::eURLType>(typeInt);
    Q_ASSERT(ok);
    urlWidgets[type]->setText(configDownloader->GetDefaultURL(type));
}
