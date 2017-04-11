#pragma once

#include "UIScreens/BaseScreen.h"

class UIViewScreen : public BaseScreen
{
public:
    UIViewScreen(DAVA::Window* window);

    void LoadResources() override;
    void UnloadResources() override;

private:
    void OnWindowSizeChanged(DAVA::Window* window, DAVA::Size2f size, DAVA::Size2f surfaceSize);

    void SetupEnvironment();
    void ClearEnvironment();

    void SetupUI();

    void PrintError(const DAVA::String& errorMessage);

    DAVA::Window* window = nullptr;
};
