#ifndef MAINWINDOW_H
#define MAINWINDOW_H


#include "Logger/Logger.h"
#include "ui_mainwindow.h"

#include "EditorSettings.h"
#include <QtGui>
#include <QtWidgets>

class PackageWidget;
class PropertiesWidget;
class LibraryWidget;
class PreviewWidget;

class LocalizationEditorDialog;
class Document;
class DocumentGroup;
class SpritesPacker;
class LoggerOutputObject;
class Project;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override;
    void AttachDocumentGroup(DocumentGroup* documentGroup);

    void OnProjectOpened(const DAVA::ResultList& resultList, const Project* project);
    void ExecDialogReloadSprites(SpritesPacker* packer);
    bool IsInEmulationMode() const;
    QComboBox* GetComboBoxLanguage();

protected:
    void closeEvent(QCloseEvent* event) override;

signals:
    void CloseProject();
    void ActionExitTriggered();
    void RecentMenuTriggered(QAction*);
    void ActionOpenProjectTriggered(QString projectPath);
    void OpenPackageFile(QString path);
    bool CloseRequested();
    void RtlChanged(bool isRtl);
    void BiDiSupportChanged(bool support);
    void GlobalStyleClassesChanged(const QString& classesStr);
    void ReloadSprites(DAVA::eGPUFamily gpu);
    void EmulationModeChanged(bool emulationMode);

public slots:
    void OnDocumentChanged(Document* document);

private slots:
    void OnShowHelp();

    void OnOpenProject();

    void RebuildRecentMenu();

    void OnBackgroundCustomColorClicked();

    void OnPixelizationStateChanged(bool isPixelized);

    void OnRtlChanged(int arg);
    void OnBiDiSupportChanged(int arg);
    void OnGlobalClassesChanged(const QString& str);
    void OnLogOutput(DAVA::Logger::eLogLevel ll, const QByteArray& output);

private:
    void InitLanguageBox();
    void FillComboboxLanguages(const Project* core);
    void InitRtlBox();
    void InitBiDiSupportBox();
    void InitGlobalClasses();
    void InitEmulationMode();
    void InitMenu();
    void SetupViewMenu();
    void SetupBackgroundMenu();
    void UpdateProjectSettings(const QString& filename);

    // Save/restore positions of DockWidgets and main window geometry
    void SaveMainWindowState();
    void RestoreMainWindowState();

    QCheckBox* emulationBox = nullptr;
    LoggerOutputObject* loggerOutput = nullptr; //will be deleted by logger. Isn't it fun?
    qint64 acceptableLoggerFlags = ~0; //all flags accepted

    QComboBox* comboboxLanguage = nullptr;
    QAction* previousBackgroundColorAction = nullptr; //need to store it to undo custom color action
};

#endif // MAINWINDOW_H
