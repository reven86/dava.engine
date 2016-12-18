#pragma once
#include <QObject>
#include "UI/mainwindow.h"
#include "EditorSystems/SelectionContainer.h"

class SpritesPacker;
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

    void SetResourceDirectory(const QString& path);
    void SelectFile(const QString& filePath);
    void SelectControl(const DAVA::String& controlPath);
    void FindControls(std::unique_ptr<FindFilter> filter);

    void ExecDialogReloadSprites(SpritesPacker* packer);

    void SetProjectActionsEnabled(bool enable);

    DocumentGroupView* GetDocumentGroupView();

    MainWindow* mainWindow = nullptr;

signals:
    void RtlChanged(bool isRtl);
    void BiDiSupportChanged(bool support);
    void GlobalStyleClassesChanged(const QString& classesStr);
    void CurrentLanguageChanged(const QString& newLangCode);
    void ReloadSprites();
    void FindFileInProject();
    void JumpToPrototype();
    void FindPrototypeInstances();
    void SelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);
    void ProjectChanged(Project* project);

public slots:
    void OnProjectChanged(Project* project);

private slots:
    void OnRtlChanged(int arg);
    void OnBiDiSupportChanged(int arg);
    void OnGlobalClassesChanged(const QString& str);
    void OnCurrentLanguageChanged(int newLanguageIndex);
    void OnSelectionChanged(const SelectedNodes& selected, const SelectedNodes& deselected);

private:
    static QString ConvertLangCodeToString(const QString& langCode);

    void InitPluginsToolBar();

    void InitLanguageBox();
    void InitRtlBox();
    void InitBiDiSupportBox();
    void InitGlobalClasses();

    QComboBox* comboboxLanguage = nullptr;
};
