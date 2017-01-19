#include "BaseScreen.h"

uint32 BaseScreen::SCREEN_INDEX = 0;

BaseScreen::BaseScreen()
    : currentScreenIndex(-1)
{
}

void BaseScreen::RegisterScreen()
{
    UIScreenManager::Instance()->RegisterScreen(SCREEN_INDEX, this);
    UIScreenManager::Instance()->SetScreen(SCREEN_INDEX);

    currentScreenIndex = SCREEN_INDEX;
    SCREEN_INDEX++;
}

bool BaseScreen::IsRegistered() const
{
    return this == UIScreenManager::Instance()->GetScreen(currentScreenIndex);
}

bool BaseScreen::IsFinished() const
{
    return false;
}
