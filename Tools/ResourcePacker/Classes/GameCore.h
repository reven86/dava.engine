/*
 *  GameCore.h
 *  TemplateProjectMacOS
 *
 *  Created by Vitaliy  Borodovsky on 3/19/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */
#ifndef __GAMECORE_H__
#define __GAMECORE_H__

#include "DAVAEngine.h"

class GameCore : public DAVA::ApplicationCore
{
protected:
    virtual ~GameCore();

public:
    GameCore();

    virtual void OnAppStarted();
    virtual void OnAppFinished();

    virtual void OnSuspend();
    virtual void OnBackground();

private:
};



#endif // __GAMECORE_H__