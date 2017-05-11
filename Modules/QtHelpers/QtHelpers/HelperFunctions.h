#pragma once

#include <functional>

class QString;
namespace QtHelpers
{
void ShowInOSFileManager(const QString& path);

void InvokeInAutoreleasePool(std::function<void()> function);

//in os Windows returns path to executable, in mac os returns bundle path
QString GetApplicationFilePath();
QString GetApplicationDirPath();
} // namespace QtHelpers
