#include "Test/TestHelpers.h"

#include <Engine/Engine.h>
#include <FileSystem/FileSystem.h>
#include <UnitTests/UnitTests.h>

#include <QString>
#include <QMainWindow>
#include <QWidget>
#include <QMenuBar>
#include <QMenu>

void TestHelpers::CreateFolder(const DAVA::FilePath& folderName)
{
    const DAVA::EngineContext* context = DAVA::GetEngineContext();
    DAVA::FileSystem* fs = context->fileSystem;

    fs->CreateDirectory(folderName, true);
}

void TestHelpers::ClearTestFolder()
{
    DAVA::FilePath folder(GetTestPath());

    DVASSERT(folder.IsDirectoryPathname());

    const DAVA::EngineContext* context = DAVA::GetEngineContext();
    DAVA::FileSystem* fs = context->fileSystem;
    fs->DeleteDirectoryFiles(folder, true);
    fs->DeleteDirectory(folder, true);
}

DAVA::FilePath TestHelpers::GetTestPath()
{
    return DAVA::FilePath("~doc:/Test/");
}

QAction* TestHelpers::FindActionInMenus(QWidget* window, const QString& menuName, const QString& actionNname)
{
    QMainWindow* mainWnd = qobject_cast<QMainWindow*>(window);
    TEST_VERIFY(mainWnd != nullptr);

    QMenuBar* menu = mainWnd->menuBar();
    QMenu* fileMenu = menu->findChild<QMenu*>(menuName);

    QAction* closeProjectAction = nullptr;
    QList<QAction*> actions = fileMenu->actions();
    foreach (QAction* action, actions)
    {
        if (action->objectName() == actionNname)
        {
            return action;
        }
    }
    TEST_VERIFY(false);
    return nullptr;
}
