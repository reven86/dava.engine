#pragma once

#include "Base/Introspection.h"
#include "Preferences/PreferencesRegistrator.h"
#include "Functional/SignalBase.h"

#include <QtGui>
#include <QtWidgets>

class Document;
class DocumentGroup;
class FileSystemDockWidget;
class LibraryWidget;
class LocalizationEditorDialog;
class LoggerOutputObject;
class PackageWidget;
class PreviewWidget;
class Project;
class PropertiesWidget;
class SpritesPacker;

namespace DAVA
{
class ResultList;
}

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow, public DAVA::InspBase, public DAVA::TrackedObject
{
    Q_OBJECT

public:
    class ProjectView;
    class DocumentGroupView;

    explicit MainWindow(QWidget* parent = nullptr);

    ~MainWindow() override;

    void SetEditorTitle(const QString& editorTitle);
    void SetRecentProjects(const QStringList& lastProjectsPathes);

    void ShowResultList(const QString& title, const DAVA::ResultList& resultList);

    void ExecDialogReloadSprites(SpritesPacker* packer);

    ProjectView* GetProjectView()
    {
        return projectView;
    }
    DocumentGroupView* GetDocumentGroupView()
    {
        return documentGroupView;
    }

signals:
    void NewProject();
    void OpenProject();
    void CloseProject();
    void Exit();
    void RecentProject(const QString& path);

    void GLWidgedReady();

    void ShowHelp();

    bool CanClose();

    void EmulationModeChanged(bool emulationMode);

private slots:
    void OnRecentMenu(QAction* action);
    void OnPixelizationStateChanged(bool isPixelized);
    void OnLogOutput(DAVA::Logger::eLogLevel ll, const QByteArray& output);
    void OnEditorPreferencesTriggered();

private:
    static QString ConvertLangCodeToString(const QString& langCode);

    void SetProjectPath(const QString& projectPath);

    void SetupShortcuts();
    void ConnectActions();
    void InitEmulationMode();
    void SetupViewMenu();

    void SetupAppStyleMenu();

    void SetupBackgroundMenu();
    void OnPreferencesPropertyChanged(const DAVA::InspMember* member, const DAVA::VariantType& value);

    bool IsPixelized() const;
    void SetPixelized(bool pixelized);

    void UpdateWindowTitle();

    void closeEvent(QCloseEvent* event) override;

    DAVA::String GetState() const;
    void SetState(const DAVA::String& array);

    DAVA::String GetGeometry() const;
    void SetGeometry(const DAVA::String& array);

    DAVA::String GetConsoleState() const;
    void SetConsoleState(const DAVA::String& array);

    std::unique_ptr<Ui::MainWindow> ui;

    QString editorTitle;
    QString projectPath;

    LoggerOutputObject* loggerOutput = nullptr; //will be deleted by logger. Isn't it fun?
    qint64 acceptableLoggerFlags = ~0; //all flags accepted

    QCheckBox* emulationBox = nullptr;

    const DAVA::InspMember* backgroundIndexMember = nullptr;
    DAVA::Set<const DAVA::InspMember*> backgroundColorMembers;
    QActionGroup* backgroundActions = nullptr;

    ProjectView* projectView = nullptr;
    DocumentGroupView* documentGroupView = nullptr;
    //std::unique_ptr<DocumentView> documentView;

public:
    INTROSPECTION(MainWindow,
                  PROPERTY("isPixelized", "MainWindowInternal/IsPixelized", IsPixelized, SetPixelized, DAVA::I_PREFERENCE)
                  PROPERTY("state", "MainWindowInternal/State", GetState, SetState, DAVA::I_PREFERENCE)
                  PROPERTY("geometry", "MainWindowInternal/Geometry", GetGeometry, SetGeometry, DAVA::I_PREFERENCE)
                  PROPERTY("consoleState", "MainWindowInternal/ConsoleState", GetConsoleState, SetConsoleState, DAVA::I_PREFERENCE)
                  )
};
