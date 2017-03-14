#pragma once

#include <FileSystem/FilePath.h>

class QString;
class QAction;
class QWidget;

namespace TestHelpers
{
void CreateTestProjectFolder();
void ClearTestFolder();
DAVA::FilePath GetTestPath();
DAVA::FilePath GetTestProjectPath();

QAction* FindActionInMenus(QWidget* window, const QString& menuName, const QString& actionNname);
} //nemspace TestHelpers
