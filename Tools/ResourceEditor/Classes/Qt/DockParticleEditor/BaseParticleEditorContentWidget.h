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


#ifndef __RESOURCEEDITORQT__BASEPARTICLEEDITORCONTENTWIDGET__
#define __RESOURCEEDITORQT__BASEPARTICLEEDITORCONTENTWIDGET__

#include "DAVAEngine.h"
#include <QChar>

#include "Scene/SceneEditor2.h"

using namespace DAVA;

class BaseParticleEditorContentWidget
{
public:
    virtual ~BaseParticleEditorContentWidget() = default;

    virtual void StoreVisualState(KeyedArchive* visualStateProps) = 0;
    virtual void RestoreVisualState(KeyedArchive* visualStateProps) = 0;

    ParticleEmitterInstance* GetEmitterInstance() const;
    ParticleEffectComponent* GetEffect() const;

    void SetActiveScene(SceneEditor2* scene);
    SceneEditor2* GetActiveScene() const;

protected:
    // "Degree mark" character needed for some widgets.
    static const QChar DEGREE_MARK_CHARACTER;

    // Conversion from/to playback speed to/from slider value.
    int ConvertFromPlaybackSpeedToSliderValue(float32 playbackSpeed);
    float ConvertFromSliderValueToPlaybackSpeed(int sliderValue);

protected:
    ParticleEmitterInstance* instance = nullptr;
    ParticleEffectComponent* effect = nullptr;
    SceneEditor2* activeScene = nullptr;
};

inline ParticleEmitterInstance* BaseParticleEditorContentWidget::GetEmitterInstance() const
{
    return instance;
};

inline ParticleEffectComponent* BaseParticleEditorContentWidget::GetEffect() const
{
    return effect;
};

// Get/set the active scene.
inline void BaseParticleEditorContentWidget::SetActiveScene(SceneEditor2* scene)
{
    activeScene = scene;
};

inline SceneEditor2* BaseParticleEditorContentWidget::GetActiveScene() const
{
    return activeScene;
};

#endif /* defined(__RESOURCEEDITORQT__BASEPARTICLEEDITORCONTENTWIDGET__) */
