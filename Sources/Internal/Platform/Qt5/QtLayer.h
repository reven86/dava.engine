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


#ifndef __DAVAENGINE_QT_LAYER_H__
#define __DAVAENGINE_QT_LAYER_H__

#include "Base/Singleton.h"
#include "Base/BaseTypes.h"

#include "UI/UIEvent.h"

namespace DAVA 
{

class QtLayerDelegate
{
public:
	virtual ~QtLayerDelegate() {}

	virtual void Quit() = 0;
};


class QtLayer
	: public Singleton<QtLayer>
{
public:
    QtLayer();
    virtual ~QtLayer();
    
    void OnSuspend();
    void OnResume();
	
    void AppStarted();
    void AppFinished();

    void InitializeGlWindow(uint64 glContextId);
    
	void Resize(int32 width, int32 height);
    void ProcessFrame();

    void * CreateAutoreleasePool();
    void ReleaseAutoreleasePool(void *pool);

    void Quit();
    void SetDelegate(QtLayerDelegate *delegate);

    bool IsDAVAEngineEnabled() const { return isDAVAEngineEnabled; };
    
    void KeyPressed(char16 key, int32 count, uint64 timestamp);
    void KeyReleased(char16 key);

    void MouseEvent(const UIEvent & event);

#ifdef __DAVAENGINE_MACOS__
    static void MakeAppForeground( bool foreground = true );
    static void RestoreMenuBar();
#endif
    
protected:
    void CopyEvents(UIEvent & newEvent, const UIEvent & sourceEvent);
    void MoveTouchsToVector(const UIEvent &event, Vector<UIEvent> &outTouches);
    
    QtLayerDelegate *delegate;
    Vector<UIEvent> events_;
    bool isDAVAEngineEnabled;
};

}


#endif // __DAVAENGINE_QT_LAYER_H__
