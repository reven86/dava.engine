/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "Base/Result.h"
#include "ui_mainwindow.h"

#include "EditorSettings.h"
#include <QtGui>
#include <QtWidgets>


class PackageWidget;
class PropertiesWidget;
class LibraryWidget;
class PreviewWidget;

class DavaGLWidget;
class LocalizationEditorDialog;
class DialogReloadSprites;

class MainWindow : public QMainWindow, public Ui::MainWindow
{
    Q_OBJECT
    
public:
    struct TabState{
        TabState(QString arg = QString()) 
            : tabText(arg)
            , isModified(false)
        {
        }
        QString tabText;
        bool isModified;
    };
    explicit MainWindow(QWidget *parent = 0);

    ~MainWindow();
    void CreateUndoRedoActions(const QUndoGroup *undoGroup);
    int CloseTab(int index);
    void SetCurrentTab(int index);
    void OnProjectOpened(const DAVA::ResultList &resultList, QString projectPath);
    int AddTab(const DAVA::FilePath &scenePath);
    void OnCleanChanged(int index, bool val);
    DavaGLWidget* GetGLWidget();
    DialogReloadSprites* GetDialogReloadSprites();
    QCheckBox* GetCheckboxEmulation();
protected:
    void closeEvent(QCloseEvent *event) override;
signals:
    void TabClosed(int tab);
    void CloseProject();
    void ActionExitTriggered();
    void RecentMenuTriggered(QAction *);
    void ActionOpenProjectTriggered(QString projectPath);
    void OpenPackageFile(QString path);
    void SaveAllDocuments();
    void SaveDocument(int index);
    void CurrentTabChanged(int index);
    void CloseRequested();
    void RtlChanged(bool isRtl);
    void GlobalStyleClassesChanged(const QString &classesStr);
    void ReloadSprites(DAVA::eGPUFamily gpu);
public slots:
    void OnProjectIsOpenChanged(bool arg);
    void OnCountChanged(int count);
private slots:
    void OnCurrentIndexChanged(int arg);
    void OnSaveDocument();
    void OnOpenFontManager();
    void OnShowHelp();
    
    void OnOpenProject();
    
    void RebuildRecentMenu();

    void SetBackgroundColorMenuTriggered(QAction* action);

    // Pixelization.
    void OnPixelizationStateChanged();
    
    void OnRtlChanged(int arg);
    void OnGlobalClassesChanged(const QString &str);
private:
    void InitLanguageBox();
    void InitRtlBox();
    void InitGlobalClasses();
    void InitEmulationMode();
	void InitMenu();
    void SetupViewMenu();
    void DisableActions();
    void UpdateProjectSettings(const QString& filename);

    // Save/restore positions of DockWidgets and main window geometry
    void SaveMainWindowState();
    void RestoreMainWindowState();
private:
    // Background Frame Color menu actions.
    QList<QAction*> backgroundFramePredefinedColorActions;
    QAction* backgroundFrameUseCustomColorAction = nullptr;
    QAction* backgroundFrameSelectCustomColorAction = nullptr;
    LocalizationEditorDialog *localizationEditorDialog = nullptr;
    DialogReloadSprites *dialogReloadSprites = nullptr;
    QCheckBox *emulationBox = nullptr;
};

Q_DECLARE_METATYPE(MainWindow::TabState*);

#endif // MAINWINDOW_H
