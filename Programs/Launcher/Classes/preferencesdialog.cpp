#include "preferencesdialog.h"
#include "filemanager.h"
#include "configdownloader.h"
#include "errormessenger.h"
#include "bamanagerclient.h"
#include "configrefresher.h"

#include <QFileDialog>
#include <QJsonObject>
#include <QJsonDocument>
#include <QApplication>
#include <QClipboard>

namespace PreferencesDialogDetails
{
const char* propertyKey = "urlType";
const char* settingsFileName = "LauncherPreferences.json";
const char* filesDirectoryKey = "storage path";
const char* launcherProtocolKey = "BA-manager key";
const char* autorefreshEnabledKey = "autorefresh enabled";
const char* autorefreshTimeoutKey = "autorefresh timeout";
const char* serverHostNameKey = "Ba-manager url";
const char* useTestAPIKey = "use test API";
};

void PreferencesDialog::ShowPreferencesDialog(FileManager* fileManager, ConfigDownloader* configDownloader, ConfigRefresher* refresher, QWidget* parent)
{
    PreferencesDialog dialog(parent);
    dialog.Init(fileManager, configDownloader, refresher);
    if (dialog.exec() == QDialog::Accepted)
    {
        dialog.AcceptData();
    }
}

void SavePreferences(FileManager* fileManager, ConfigDownloader* configDownloader, BAManagerClient* commandListener, ConfigRefresher* refresher)
{
    QJsonObject rootObject;
    rootObject[PreferencesDialogDetails::filesDirectoryKey] = fileManager->GetFilesDirectory();
    rootObject[PreferencesDialogDetails::launcherProtocolKey] = commandListener->GetProtocolKey();
    rootObject[PreferencesDialogDetails::serverHostNameKey] = configDownloader->GetServerHostName();
    rootObject[PreferencesDialogDetails::useTestAPIKey] = configDownloader->IsTestAPIUsed();

    rootObject[PreferencesDialogDetails::autorefreshEnabledKey] = refresher->IsEnabled();
    rootObject[PreferencesDialogDetails::autorefreshTimeoutKey] = refresher->GetTimeout();

    QJsonDocument document(rootObject);
    QString filePath = FileManager::GetDocumentsDirectory() + PreferencesDialogDetails::settingsFileName;
    QFile settingsFile(filePath);
    if (settingsFile.open(QFile::WriteOnly | QFile::Truncate))
    {
        settingsFile.write(document.toJson());
    }
    else
    {
        ErrorMessenger::ShowErrorMessage(ErrorMessenger::ERROR_FILE, QObject::tr("Can not write to file %1").arg(filePath));
    }
}

void LoadPreferences(FileManager* fileManager, ConfigDownloader* configDownloader, BAManagerClient* commandListener, ConfigRefresher* refresher)
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
    QJsonValue serverHostNameValue = rootObject[PreferencesDialogDetails::serverHostNameKey];
    if (serverHostNameValue.isString())
    {
        configDownloader->SetServerHostName(serverHostNameValue.toString());
    }

    QJsonValue canUseTestAPIValue = rootObject[PreferencesDialogDetails::useTestAPIKey];
    if (canUseTestAPIValue.isBool())
    {
        configDownloader->SetUseTestAPI(canUseTestAPIValue.toBool());
    }

    QJsonValue filesDirValue = rootObject[PreferencesDialogDetails::filesDirectoryKey];
    if (filesDirValue.isString())
    {
        fileManager->SetFilesDirectory(filesDirValue.toString());
    }

    QJsonValue protocolKeyValue = rootObject[PreferencesDialogDetails::launcherProtocolKey];
    if (protocolKeyValue.isString())
    {
        commandListener->SetProtocolKey(protocolKeyValue.toString());
    }

    QJsonValue autorefreshEnabledValue = rootObject[PreferencesDialogDetails::autorefreshEnabledKey];
    if (autorefreshEnabledValue.isBool())
    {
        refresher->SetEnabled(autorefreshEnabledValue.toBool());
    }

    QJsonValue autorefreshTimeoutValue = rootObject[PreferencesDialogDetails::autorefreshTimeoutKey];
    if (autorefreshTimeoutValue.isDouble())
    {
        refresher->SetTimeout(autorefreshTimeoutValue.toInt());
    }
}

PreferencesDialog::PreferencesDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    connect(pushButton_selectStorageDir, &QPushButton::clicked, this, &PreferencesDialog::OnButtonChooseFilesPathClicked);
    connect(lineEdit_launcherDataPath, &QLineEdit::textChanged, this, &PreferencesDialog::ProcessSaveButtonEnabled);
    connect(lineEdit_serverHostName, &QLineEdit::textChanged, this, &PreferencesDialog::ProcessSaveButtonEnabled);
}

void PreferencesDialog::Init(FileManager* fileManager_, ConfigDownloader* configDownloader_, ConfigRefresher* refresher_)
{
    fileManager = fileManager_;
    configDownloader = configDownloader_;
    configRefresher = refresher_;
    Q_ASSERT(fileManager != nullptr);
    Q_ASSERT(configDownloader != nullptr);
    Q_ASSERT(configRefresher != nullptr);

    lineEdit_launcherDataPath->setText(fileManager->GetFilesDirectory());

    lineEdit_serverHostName->setText(configDownloader->GetServerHostName());
    connect(lineEdit_serverHostName, &QLineEdit::textChanged, this, &PreferencesDialog::OnServerHostNameChanged);

    label_currentPlatformBuildsLabel->setText(platformString + QString(" builds URL"));

    urlWidgets[ConfigDownloader::LauncherInfoURL] = label_launcherInfoURL;
    urlWidgets[ConfigDownloader::LauncherTestInfoURL] = label_launcherInfoTestURL;
    urlWidgets[ConfigDownloader::StringsURL] = label_launcherMetaInfoURL;
    urlWidgets[ConfigDownloader::FavoritesURL] = label_launcherFavoritesURL;
    urlWidgets[ConfigDownloader::AllBuildsCurrentPlatformURL] = label_currentPlatformBuildsURL;
    urlWidgets[ConfigDownloader::AllBuildsIOSURL] = label_androidBuildsURL;
    urlWidgets[ConfigDownloader::AllBuildsAndroidURL] = label_IOSBuildsURL;
    urlWidgets[ConfigDownloader::AllBuildsUWPURL] = label_UWPBuildsURL;

    copyURLWidgets[ConfigDownloader::LauncherInfoURL] = pushButton_copyLauncherInfoURL;
    copyURLWidgets[ConfigDownloader::LauncherTestInfoURL] = pushButton_copyLauncherTestInfoURL;
    copyURLWidgets[ConfigDownloader::StringsURL] = pushButton_copyMetaInfoURL;
    copyURLWidgets[ConfigDownloader::FavoritesURL] = pushButton_copyFavoritesURL;
    copyURLWidgets[ConfigDownloader::AllBuildsCurrentPlatformURL] = pushButton_copyCurrentPlatformBuildsURL;
    copyURLWidgets[ConfigDownloader::AllBuildsAndroidURL] = pushButton_copyAndroidBuildsURL;
    copyURLWidgets[ConfigDownloader::AllBuildsIOSURL] = pushButton_copyIOSBuildsURL;
    copyURLWidgets[ConfigDownloader::AllBuildsUWPURL] = pushButton_copyUWPBuildsURL;

    for (int i = ConfigDownloader::LauncherInfoURL; i < ConfigDownloader::URLTypesCount; ++i)
    {
        ConfigDownloader::eURLType type = static_cast<ConfigDownloader::eURLType>(i);
        urlWidgets[type]->setProperty(PreferencesDialogDetails::propertyKey, i);
        copyURLWidgets[type]->setProperty(PreferencesDialogDetails::propertyKey, i);

        connect(copyURLWidgets[type], &QPushButton::clicked, this, &PreferencesDialog::OnButtonCopyURLClicked);
    }
    checkBox_useTestAPI->setChecked(configDownloader->IsTestAPIUsed());

    checkBox_autorefreshEnabled->setChecked(configRefresher->IsEnabled());
    int minTimeout = configRefresher->GetMinimumTimeout();
    int maxTimeout = configRefresher->GetMaximumTimeout();
    spinBox_autorefreshTimeout->setRange(minTimeout, maxTimeout);
    spinBox_autorefreshTimeout->setEnabled(true);
    spinBox_autorefreshTimeout->setValue(configRefresher->GetTimeout());
    OnServerHostNameChanged(lineEdit_serverHostName->text());
}

void PreferencesDialog::AcceptData()
{
    Q_ASSERT(fileManager != nullptr);
    Q_ASSERT(configDownloader != nullptr);

    fileManager->SetFilesDirectory(lineEdit_launcherDataPath->text());

    configDownloader->SetServerHostName(lineEdit_serverHostName->text());
    configDownloader->SetUseTestAPI(checkBox_useTestAPI->isChecked());

    configRefresher->SetEnabled(checkBox_autorefreshEnabled->isChecked());
    configRefresher->SetTimeout(spinBox_autorefreshTimeout->value());
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
    bool enabled = (lineEdit_serverHostName->text().isEmpty() == false);
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

void PreferencesDialog::OnServerHostNameChanged(const QString& name)
{
    for (int i = ConfigDownloader::LauncherInfoURL; i < ConfigDownloader::URLTypesCount; ++i)
    {
        ConfigDownloader::eURLType type = static_cast<ConfigDownloader::eURLType>(i);
        QString text = name + configDownloader->GetURL(type);

        urlWidgets[type]->setText("<a href=\"" + text + "\">" + text + "</a>");
    }
}

void PreferencesDialog::OnButtonCopyURLClicked()
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    Q_ASSERT(button != nullptr);
    bool ok;
    int typeInt = button->property(PreferencesDialogDetails::propertyKey).toInt(&ok);
    ConfigDownloader::eURLType type = static_cast<ConfigDownloader::eURLType>(typeInt);
    Q_ASSERT(ok);
    Q_ASSERT(urlWidgets.contains(type));
    QString text = lineEdit_serverHostName->text() + configDownloader->GetURL(type);
    QApplication::clipboard()->setText(text);
}
