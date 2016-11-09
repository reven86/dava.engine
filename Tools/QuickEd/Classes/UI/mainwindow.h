#pragma once

#include "Base/Introspection.h"
#include "Preferences/PreferencesRegistrator.h"
#include "Functional/SignalBase.h"

#if defined(__DAVAENGINE_MACOS__)
#include "QtTools/Utils/ShortcutChecker.h"
#endif //__DAVAENGINE_MACOS__

#include "QtTools/Utils/QtDelayedExecutor.h"

#include <QtGui>
#include <QtWidgets>

namespace DAVA
{
class RenderWidget;
}

class LoggerOutputObject;
class Project;

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
    void InjectRenderWidget(DAVA::RenderWidget* renderWidget);
    void OnWindowCreated();

    void ShowResultList(const QString& title, const DAVA::ResultList& resultList);

    ProjectView* GetProjectView()
    {
        return projectView;
    }

signals:
    void NewProject();
    void OpenProject();
    void CloseProject();
    void Exit();
    void RecentProject(const QString& path);

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

    bool eventFilter(QObject* object, QEvent* event) override;

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

#if defined(__DAVAENGINE_MACOS__)
    ShortcutChecker shortcutChecker;
#endif //__DAVAENGINE_MACOS__

    QtDelayedExecutor delayedExecutor;

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
