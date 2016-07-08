#include "GameCore.h"
#include "AppScreens.h"
#include "TestScreen.h"

using namespace DAVA;

GameCore::GameCore()
{
}

GameCore::~GameCore()
{
}

void GameCore::OnAppStarted()
{
    cursor = 0;
    RenderManager::Instance()->SetFPS(60);

    testScreen = new TestScreen();

    UIScreenManager::Instance()->RegisterScreen(SCREEN_TEST, testScreen);

    UIScreenManager::Instance()->SetFirst(SCREEN_TEST);
}

void GameCore::OnAppFinished()
{
    SafeRelease(cursor);

    SafeRelease(testScreen);
}

void GameCore::OnSuspend()
{
//    Logger::Debug("GameCore::OnSuspend");
#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
    ApplicationCore::OnSuspend();
#endif
}

void GameCore::OnResume()
{
    //    Logger::Debug("GameCore::OnResume");
    ApplicationCore::OnResume();
}

void GameCore::OnBackground()
{
    //    Logger::Debug("GameCore::OnBackground");
}

#if defined(__DAVAENGINE_IPHONE__) || defined(__DAVAENGINE_ANDROID__)
void GameCore::OnDeviceLocked()
{
    //    Logger::Debug("GameCore::OnDeviceLocked");
    Core::Instance()->Quit();
}
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

void GameCore::BeginFrame()
{
    ApplicationCore::BeginFrame();
    RenderManager::Instance()->ClearWithColor(0, 0, 0, 0);
}

void GameCore::Update(float32 timeElapsed)
{
    //	if (!cursor)
    //	{
    //		cursor = Cursor::Create("~res:/Cursor/cursor1.png", Vector2(6, 0));
    //		RenderManager::Instance()->SetCursor(cursor);
    //	}
    ApplicationCore::Update(timeElapsed);
}

void GameCore::Draw()
{
    ApplicationCore::Draw();
}