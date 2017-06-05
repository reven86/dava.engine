#pragma once

#include "UIScreens/BaseScreen.h"

#include <FileSystem/FilePath.h>

namespace DAVA
{
class ProgramOptions;
}

class UIViewScreen : public BaseScreen
{
public:
    UIViewScreen(DAVA::Window* window, DAVA::ProgramOptions* options);

    void LoadResources() override;
    void UnloadResources() override;

private:
    void OnWindowSizeChanged(DAVA::Window* window, DAVA::Size2f size, DAVA::Size2f surfaceSize);

    void SetupEnvironment();
    void ClearEnvironment();

    void SetupUI();

    void PrintError(const DAVA::String& errorMessage);

    DAVA::Window* window = nullptr;
    DAVA::ProgramOptions* options = nullptr;

    DAVA::FilePath projectPath;
};
