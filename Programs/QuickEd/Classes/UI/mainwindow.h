#pragma once

#include <TArc/DataProcessing/DataNode.h>

#if defined(__DAVAENGINE_MACOS__)
#include <QtTools/Utils/ShortcutChecker.h>
#endif //__DAVAENGINE_MACOS__
#include <QtTools/Utils/QtDelayedExecutor.h>

#include <Base/Introspection.h>
#include <Preferences/PreferencesRegistrator.h>

#include <QMainWindow>

namespace Ui
{
class MainWindow;
}

class PackageWidget;
class StyleSheetInspectorWidget;

namespace DAVA
{
class ResultList;
namespace TArc
{
class ContextAccessor;
class UI;
}
}

class QCheckBox;
class QActionGroup;
class QEvent;

class MainWindow : public QMainWindow, public DAVA::InspBase, public DAVA::TrackedObject
{
    Q_OBJECT

public:
    class ProjectView;

    explicit MainWindow(DAVA::TArc::ContextAccessor* accessor, DAVA::TArc::UI* ui, QWidget* parent = nullptr);

    ~MainWindow() override;

    void SetEditorTitle(const QString& editorTitle);

    ProjectView* GetProjectView() const;
    PackageWidget* GetPackageWidget() const;

signals:
    void EmulationModeChanged(bool emulationMode);

private slots:
    void OnPixelizationStateChanged(bool isPixelized);
    void OnEditorPreferencesTriggered();

private:
    void SetProjectPath(const QString& projectPath);

    void ConnectActions();
    void InitEmulationMode();
    void SetupViewMenu();

    void SetupAppStyleMenu();

    void SetupBackgroundMenu();
    void OnPreferencesPropertyChanged(const DAVA::InspMember* member, const DAVA::VariantType& value);

    bool IsPixelized() const;
    void SetPixelized(bool pixelized);

    void UpdateWindowTitle();

    bool eventFilter(QObject* object, QEvent* event) override;

    std::unique_ptr<Ui::MainWindow> ui;

    QString editorTitle;
    QString projectPath;

    QCheckBox* emulationBox = nullptr;

    const DAVA::InspMember* backgroundIndexMember = nullptr;
    DAVA::Set<const DAVA::InspMember*> backgroundColorMembers;
    QActionGroup* backgroundActions = nullptr;

#if defined(__DAVAENGINE_MACOS__)
    ShortcutChecker shortcutChecker;
#endif //__DAVAENGINE_MACOS__

    QtDelayedExecutor delayedExecutor;

    ProjectView* projectView = nullptr;

public:
    INTROSPECTION(MainWindow,
                  PROPERTY("isPixelized", "MainWindowInternal/IsPixelized", IsPixelized, SetPixelized, DAVA::I_PREFERENCE)
                  )
};
