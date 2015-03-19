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



#ifndef __DAVAENGINE_UI_PARTICLES__
#define __DAVAENGINE_UI_PARTICLES__

#include "UI/UIControl.h"

namespace DAVA 
{

class ParticleEffectComponent;
class ParticleEffectSystem;
class Camera;

class UIParticles : public UIControl 
{
protected:
    virtual ~UIParticles();
public:
    UIParticles(const Rect &rect = Rect());

    void Update(float32 timeElapsed) override;
    void Draw(const UIGeometricData &geometricData) override;

    void WillAppear() override;

    UIParticles* Clone() override;
    void CopyDataFrom(UIControl *srcControl) override;

    void LoadFromYamlNode(const YamlNode * node, UIYamlLoader * loader) override;
    YamlNode* SaveToYamlNode(UIYamlLoader * loader) override;

    /*methods analogical to once in ParticleEffectComponent*/
    void Start();
    void Stop(bool isDeleteAllParticles = true);
    void Restart(bool isDeleteAllParticles = true);
    bool IsStopped() const;
    void Pause(bool isPaused = true);
    bool IsPaused() const;

    void SetEffectPath(const FilePath& path);
    const FilePath& GetEffectPath() const;
    void ReloadEffect();

    void SetAutostart(bool value);
    bool IsAutostart() const;

    // Start delay, in seconds.
    float32 GetStartDelay() const;
    void SetStartDelay(float32 value);

    //external
    void SetExtertnalValue(const String& name, float32 value);

protected:
    void LoadEffect(const FilePath& path);
    void UnloadEffect();

    // Start the playback in case Autostart flag is set.
    void HandleAutostart();

    // Handle the delayed action if requested.
    void HandleDelayedAction(float32 timeElapsed);

    // Start/Restart methods which can be called either immediately of after start delay.
    void DoStart();
    void DoRestart();

    enum eDelayedActionType
    {
        actionNone = 0,
        actionStart,
        actionRestart
    };

private:
    FilePath effectPath;
    bool isAutostart;
    float32 startDelay;

    ParticleEffectComponent *effect;
    ParticleEffectSystem *system;
    Matrix4 matrix;
    float32 updateTime;

    eDelayedActionType delayedActionType;
    float32 delayedActionTime;
    bool delayedDeleteAllParticles;
    bool needHandleAutoStart;

    static Camera *defaultCamera;
public:
    INTROSPECTION_EXTEND(UIParticles, UIControl,
        PROPERTY("effectPath", "Effect path", GetEffectPath, SetEffectPath, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("autoStart", "Auto start", IsAutostart, SetAutostart, I_SAVE | I_VIEW | I_EDIT)
        PROPERTY("startDelay", "Start delay", GetStartDelay, SetStartDelay, I_SAVE | I_VIEW | I_EDIT)
    );
};
	
};

#endif