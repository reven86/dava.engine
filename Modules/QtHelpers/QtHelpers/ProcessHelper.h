#pragma once

#include <QString>

namespace ProcessHelper
{
bool IsProcessRuning(const QString& path);
void SetActiveProcess(const QString& path);

//open application executable file
void RunProcess(const QString& path);

//open application bundle file
void OpenApplication(const QString& path);
};
