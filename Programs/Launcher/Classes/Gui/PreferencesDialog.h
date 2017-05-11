#pragma once

#include "ui_preferencesdialog.h"
#include "Core/ConfigDownloader.h"

class FileManager;
class ConfigDownloader;
class BAManagerClient;
class ConfigRefresher;

class PreferencesDialog : public QDialog, private Ui::PreferencesDialog
{
    Q_OBJECT

public:
    static void ShowPreferencesDialog(FileManager* fileManager, ConfigDownloader* configDownloader, ConfigRefresher* configRefresher, QWidget* parent = nullptr);

private slots:
    void OnButtonCopyURLClicked();
    void OnButtonChooseFilesPathClicked();
    void ProcessSaveButtonEnabled();
    void OnServerHostNameChanged(const QString& name);

private:
    PreferencesDialog(QWidget* parent = nullptr);
    void Init(FileManager* fileManager, ConfigDownloader* configDownloader, ConfigRefresher* configRefresher);
    void AcceptData();

    FileManager* fileManager = nullptr;
    ConfigDownloader* configDownloader = nullptr;
    ConfigRefresher* configRefresher = nullptr;

    QMap<ConfigDownloader::eURLType, QLabel*> urlWidgets;
    QMap<ConfigDownloader::eURLType, QPushButton*> copyURLWidgets;
};

void SavePreferences(FileManager* fileManager, ConfigDownloader* configDownloader, BAManagerClient* commandListener, ConfigRefresher* configRefresher);
void LoadPreferences(FileManager* fileManager, ConfigDownloader* configDownloader, BAManagerClient* commandListener, ConfigRefresher* configRefresher);
