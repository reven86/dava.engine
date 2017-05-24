#pragma once

#include "ui_preferencesdialog.h"
#include "Core/UrlsHolder.h"

class FileManager;
class UrlsHolder;
class BAManagerClient;
class ConfigRefresher;

class PreferencesDialog : public QDialog, private Ui::PreferencesDialog
{
    Q_OBJECT

public:
    static void ShowPreferencesDialog(FileManager* fileManager, UrlsHolder* configDownloader, ConfigRefresher* configRefresher, QWidget* parent = nullptr);

private slots:
    void OnButtonCopyURLClicked();
    void OnButtonChooseFilesPathClicked();
    void ProcessSaveButtonEnabled();
    void OnServerHostNameChanged(const QString& name);

private:
    PreferencesDialog(QWidget* parent = nullptr);
    void Init(FileManager* fileManager, UrlsHolder* configDownloader, ConfigRefresher* configRefresher);
    void AcceptData();

    FileManager* fileManager = nullptr;
    UrlsHolder* configDownloader = nullptr;
    ConfigRefresher* configRefresher = nullptr;

    QMap<UrlsHolder::eURLType, QLabel*> urlWidgets;
    QMap<UrlsHolder::eURLType, QPushButton*> copyURLWidgets;
};

void SavePreferences(FileManager* fileManager, UrlsHolder* configDownloader, BAManagerClient* commandListener, ConfigRefresher* configRefresher);
void LoadPreferences(FileManager* fileManager, UrlsHolder* configDownloader, BAManagerClient* commandListener, ConfigRefresher* configRefresher);
