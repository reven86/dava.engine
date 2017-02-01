#pragma once

#include <functional>

class QString;
namespace QtHelpers
{
void ShowInOSFileManager(const QString& path);

void InvokeInAutoreleasePool(std::function<void()> function);
} // namespace QtHelpers