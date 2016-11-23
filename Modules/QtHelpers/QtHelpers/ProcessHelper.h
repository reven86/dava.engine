#pragma once

#include <QString>

namespace ProcessHelper
{
    bool IsProcessRuning(const QString& path);
    void SetActiveProcess(const QString& path);
    void RunProcess(const QString& path);
};
