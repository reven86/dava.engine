#include "GameCore.h"

#include "FileSystem/ResourceArchive.h"
#include "StringConstants.h"
#include "version.h"

using namespace DAVA;

GameCore::GameCore()
{
}

GameCore::~GameCore()
{
}

void GameCore::OnAppStarted()
{
    Renderer::SetDesiredFPS(60);

    Renderer::GetOptions()->SetOption(RenderOptions::LAYER_OCCLUSION_STATS, true);
    DynamicBufferAllocator::SetPageSize(16 * 1024 * 1024); // 16 mb

    UIControlSystem::Instance()->SetClearColor(Color(.3f, .3f, .3f, 1.f));
}

void GameCore::OnAppFinished()
{
}

void GameCore::OnSuspend()
{
    //prevent going to suspend
}

void GameCore::OnResume()
{
    ApplicationCore::OnResume();
}

void GameCore::OnBackground()
{
    //prevent going to background
}

void GameCore::BeginFrame()
{
    ApplicationCore::BeginFrame();
}

void GameCore::Update(float32 timeElapsed)
{
    ApplicationCore::Update(timeElapsed);
}

void GameCore::Draw()
{
    ApplicationCore::Draw();
}
