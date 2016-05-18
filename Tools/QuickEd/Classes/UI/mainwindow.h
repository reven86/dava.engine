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

#include "Base/Introspection.h"
#include "Logger/Logger.h"
#include "Render/RenderBase.h"
#include "ui_mainwindow.h"

#include "Preferences/PreferencesRegistrator.h"

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

class MainWindow : public QMainWindow, public Ui::MainWindow, public DAVA::InspBase, public DAVA::TrackedObject
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
    void RebuildRecentMenu(const QStringList& lastProjectsPathes);

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
    void OnOpenProjectAction();
    void OnPixelizationStateChanged(bool isPixelized);

    void OnRtlChanged(int arg);
    void OnBiDiSupportChanged(int arg);
    void OnGlobalClassesChanged(const QString& str);
    void OnLogOutput(DAVA::Logger::eLogLevel ll, const QByteArray& output);
    void OnEditorPreferencesTriggered();

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
    void UpdateProjectSettings();
    void OnPreferencesPropertyChanged(const DAVA::InspMember* member, const DAVA::VariantType& value);

    bool IsPixelized() const;
    void SetPixelized(bool pixelized);

    DAVA::String GetState() const;
    void SetState(const DAVA::String& array);

    DAVA::String GetGeometry() const;
    void SetGeometry(const DAVA::String& array);

    DAVA::String GetConsoleState() const;
    void SetConsoleState(const DAVA::String& array);

    QCheckBox* emulationBox = nullptr;
    LoggerOutputObject* loggerOutput = nullptr; //will be deleted by logger. Isn't it fun?
    qint64 acceptableLoggerFlags = ~0; //all flags accepted

    QComboBox* comboboxLanguage = nullptr;
    QString currentProjectPath;

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

#endif // MAINWINDOW_H
