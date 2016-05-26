/*
 *  GameCore.cpp
 *  TemplateProjectMacOS
 *
 *  Created by Vitaliy  Borodovsky on 3/19/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include "GameCore.h"

using namespace DAVA;

GameCore::GameCore()
{
}

GameCore::~GameCore()
{
}

void GameCore::OnAppStarted()
{
}

void GameCore::OnAppFinished()
{
}

void GameCore::OnSuspend()
{
    //prevent going to suspend
    //ApplicationCore::OnSuspend();
}

void GameCore::OnBackground()
{
    //ApplicationCore::OnBackground();
}
