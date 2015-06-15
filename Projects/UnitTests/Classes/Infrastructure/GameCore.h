/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __GAMECORE_H__
#define __GAMECORE_H__

#include "Base/BaseTypes.h"
#include "Core/Core.h"

#include "TeamcityOutput/TeamCityTestsOutput.h"

class GameCore : public DAVA::ApplicationCore
{
protected:
    virtual ~GameCore() = default;

public:    
    GameCore() = default;

    static GameCore* Instance()
    { 
        return static_cast<GameCore*>(DAVA::Core::GetApplicationCore());
    };
    
    void OnAppStarted() override;
    void OnAppFinished() override;

    void OnSuspend() override;
    void OnResume() override;
    
#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    void OnBackground() override {}
    void OnForeground() override;
    void OnDeviceLocked() override {}
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

    void Update(DAVA::float32 update) override;

private:
    void ProcessCommandLine();
    void ProcessTests(DAVA::float32 timeElapsed);
    void FinishTests();

    void OnError();

    void OnTestClassStarted(const DAVA::String& testClassName);
    void OnTestClassFinished(const DAVA::String& testClassName);
    void OnTestClassDisabled(const DAVA::String& testClassName);
    void OnTestStarted(const DAVA::String& testClassName, const DAVA::String& testName);
    void OnTestFinished(const DAVA::String& testClassName, const DAVA::String& testName);
    void OnTestFailed(const DAVA::String& testClassName, const DAVA::String& testName, const DAVA::String& condition, const char* filename, int lineno, const DAVA::String& userMessage);

private:
    DAVA::TeamcityTestsOutput teamCityOutput;
};

#endif // __GAMECORE_H__
