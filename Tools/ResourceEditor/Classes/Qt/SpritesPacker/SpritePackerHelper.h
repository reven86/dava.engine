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


#ifndef __SPRITE_PACKER_HELPER_H__
#define __SPRITE_PACKER_HELPER_H__

#include "DAVAEngine.h"
#include <QFutureWatcher>

class SceneData;
namespace DAVA {

// Sprite Packer Helper for Particles Editor.
class SpritePackerHelper : public QObject, public DAVA::StaticSingleton<SpritePackerHelper>
{
	Q_OBJECT

public:
	SpritePackerHelper();
	void UpdateParticleSprites(DAVA::eGPUFamily gpu);

signals:
	void readyAll();

protected:
    
    void EnumerateSpritesForReloading(Scene * scene, Map<String, Sprite *> &sprites);
    void EnumerateSpritesForParticleEmitter(ParticleEmitter* emitter, Map<String, Sprite *> &sprites);

    void FindAllParticleEffectsRecursive(DAVA::Entity *entity , DAVA::List<DAVA::Entity*> & particleEffects);

	void Pack(DAVA::eGPUFamily gpu);
	void Reload();

    // Stop particle effects and remember their started states.
    void StopParticleEffects(Scene * scene);

    // Restart particle effects which were initially started.
    void RestartParticleEffects();

	QFuture<void> *future;
	QFutureWatcher<void> watcher;
    
    Map<ParticleEffectComponent*, bool> particleEffectsState;

private slots:
    void threadRepackAllFinished();
	
};
};
#endif /* __SPRITE_PACKER_HELPER_H__ */