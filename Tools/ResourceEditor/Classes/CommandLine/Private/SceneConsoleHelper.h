#pragma once

#include "Base/BaseTypes.h"

class SceneConsoleHelper
{
public:
    static DAVA::FilePath CreateQualityPathname(const DAVA::FilePath& qualityPathname, const DAVA::FilePath& targetPathname);
    static void InitializeRendering(const DAVA::FilePath& qualityPathname);
    static void ReleaseRendering();
};
