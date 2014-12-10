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

#include "DAVAEngine.h"
#include "Database/MongodbClient.h"

#include "../rhi_Base.h"

using namespace DAVA;

class GameCore : public ApplicationCore
{
protected:
    virtual ~GameCore();
public: 
    GameCore();

    static GameCore * Instance() 
    { 
        return (GameCore*) DAVA::Core::GetApplicationCore();
    };
    
    virtual void OnAppStarted();
    virtual void OnAppFinished();
    
    virtual void OnSuspend();
    virtual void OnResume();

#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)
    virtual void OnBackground();
    virtual void OnForeground();
    virtual void OnDeviceLocked();
#endif //#if defined (__DAVAENGINE_IPHONE__) || defined (__DAVAENGINE_ANDROID__)

    virtual void BeginFrame();
    virtual void EndFrame();
    
    
protected:

    void CreateDocumentsFolder();

    struct
    VertexP
    {
        float   x,y,z;
    };

    rhi::Handle vb;
    rhi::Handle ps;
    rhi::Handle fp_cb;

};



#endif // __GAMECORE_H__