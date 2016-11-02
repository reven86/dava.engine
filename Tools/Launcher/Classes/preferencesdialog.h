#pragma once

#include "ui_preferencesdialog.h"
#include "configdownloader.h"

class FileManager;
class ConfigDownloader;

class PreferencesDialog : public QDialog, private Ui::PreferencesDialog
{
    Q_OBJECT

public:
    static void ShowPreferencesDialog(FileManager* fileManager, ConfigDownloader* configDownloader, QWidget* parent = nullptr);

private slots:
    void OnButtonResetURLClicked();
    void OnButtonChooseFilesPathClicked();
    void ProcessSaveButtonEnabled();

private:
    PreferencesDialog(QWidget* parent = nullptr);
    void Init(FileManager* fileManager, ConfigDownloader* configDownloader);
    void AcceptData();

    FileManager* fileManager = nullptr;
    ConfigDownloader* configDownloader = nullptr;

    QMap<ConfigDownloader::eURLType, QLineEdit*> urlWidgets;
    QMap<ConfigDownloader::eURLType, QPushButton*> resetUrlWidgets;
};

void SavePreferences(FileManager* fileManager, ConfigDownloader* configDownloader);
void LoadPreferences(FileManager* fileManager, ConfigDownloader* configDownloader);
