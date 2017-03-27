#pragma once
#include <QObject>
#include "UI/mainwindow.h"

class QComboBox;
class FindItem;
class FindFilter;
class Project;

class MainWindow::ProjectView : public QObject
{
    Q_OBJECT
public:
    ProjectView(MainWindow* mainWindow);

    void SetProjectPath(const QString& projectPath);
    void SetLanguages(const QStringList& availableLangsCodes, const QString& currentLangCode);
    void SetCurrentLanguage(const QString& currentLang);

    void FindControls(std::unique_ptr<FindFilter>&& filter);

    void SetProjectActionsEnabled(bool enable);

    DocumentGroupView* GetDocumentGroupView();

    MainWindow* mainWindow = nullptr;

signals:
    void RtlChanged(bool isRtl);
    void BiDiSupportChanged(bool support);
    void GlobalStyleClassesChanged(const QString& classesStr);
    void CurrentLanguageChanged(const QString& newLangCode);
    void FindFileInProject();
    void JumpToPrototype();
    void FindPrototypeInstances();
    void ProjectChanged(Project* project);

    void JumpToControl(const DAVA::FilePath& packagePath, const DAVA::String& controlName);
    void JumpToPackage(const DAVA::FilePath& packagePath);

public slots:
    void OnProjectChanged(Project* project);

private slots:
    void OnRtlChanged(int arg);
    void OnBiDiSupportChanged(int arg);
    void OnGlobalClassesChanged(const QString& str);
    void OnCurrentLanguageChanged(int newLanguageIndex);

private:
    static QString ConvertLangCodeToString(const QString& langCode);

    void InitPluginsToolBar();

    void InitLanguageBox();
    void InitRtlBox();
    void InitBiDiSupportBox();
    void InitGlobalClasses();

    QComboBox* comboboxLanguage = nullptr;
};
