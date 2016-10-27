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

class MainWindow
: public QMainWindow
  ,
  public DAVA::InspBase
  ,
  public DAVA::TrackedObject
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);

    ~MainWindow() override;

    void SetProjectTitle(const QString& projectTitle);
    void SetProjectPath(const QString& projectPath);

    void SetRecentProjects(const QStringList& lastProjectsPathes);
    void SetLanguages(const QStringList& availableLangsCodes, const QString& currentLangCode);
    void SetCurrentLanguage(const QString& currentLang);

    void SetProjectActionsEnabled(bool enable);

    PreviewWidget* GetPreviewWidget();
    PropertiesWidget* GetPropertiesWidget();
    FileSystemDockWidget* GetFileSystemWidget();
    PackageWidget* GetPackageWidget();
    LibraryWidget* GetLibraryWidget();

    void SetDocumentGroupActionsEnable(bool enable);

    void ShowResultList(const QString& title, const DAVA::ResultList& resultList);

    void AttachDocumentGroup(DocumentGroup* documentGroup);
    void DetachDocumentGroup(DocumentGroup* documentGroup);

    void ExecDialogReloadSprites(SpritesPacker* packer);

signals:
    void NewProject();
    void OpenProject();
    void CloseProject();
    void Exit();
    void RecentProject(const QString& path);
    void ReloadSprites();

    void GLWidgedReady();

    void ShowHelp();

    void OpenPackageFile(QString path);

    void RtlChanged(bool isRtl);
    void BiDiSupportChanged(bool support);
    void GlobalStyleClassesChanged(const QString& classesStr);
    void EmulationModeChanged(bool emulationMode);

    bool CanClose();

    void CurrentLanguageChanged(const QString& newLangCode);

public slots:
    void OnDocumentChanged(Document* document);

private slots:
    void OnRecentMenu(QAction* action);

    void OnPixelizationStateChanged(bool isPixelized);

    void OnRtlChanged(int arg);
    void OnBiDiSupportChanged(int arg);
    void OnGlobalClassesChanged(const QString& str);
    void OnLogOutput(DAVA::Logger::eLogLevel ll, const QByteArray& output);
    void OnEditorPreferencesTriggered();
    void OnCurrentLanguageChanged(int newLanguageIndex);

private:
    static QString ConvertLangCodeToString(const QString& langCode);

    void SetupShortcuts();
    void ConnectActions();

    void InitPluginsToolBar();

    void InitLanguageBox();
    void InitRtlBox();
    void InitBiDiSupportBox();
    void InitGlobalClasses();
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

    QString projectTitle;
    QString projectPath;

    LoggerOutputObject* loggerOutput = nullptr; //will be deleted by logger. Isn't it fun?
    qint64 acceptableLoggerFlags = ~0; //all flags accepted

    std::unique_ptr<QCheckBox> emulationBox;
    std::unique_ptr<QComboBox> comboboxLanguage;

    const DAVA::InspMember* backgroundIndexMember = nullptr;
    DAVA::Set<const DAVA::InspMember*> backgroundColorMembers;
    QActionGroup* backgroundActions = nullptr;

public:
    INTROSPECTION(MainWindow,
                  PROPERTY("isPixelized", "MainWindowInternal/IsPixelized", IsPixelized, SetPixelized, DAVA::I_PREFERENCE)
                  PROPERTY("state", "MainWindowInternal/State", GetState, SetState, DAVA::I_PREFERENCE)
                  PROPERTY("geometry", "MainWindowInternal/Geometry", GetGeometry, SetGeometry, DAVA::I_PREFERENCE)
                  PROPERTY("consoleState", "MainWindowInternal/ConsoleState", GetConsoleState, SetConsoleState, DAVA::I_PREFERENCE)
                  )
};
