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



#ifndef __PARTICLE_EFFECT_PROPERTIES_WIDGET__H__
#define __PARTICLE_EFFECT_PROPERTIES_WIDGET__H__

#include <QWidget>
#include "BaseParticleEditorContentWidget.h"

#include <QVBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QCheckBox>

class ParticleEffectPropertiesWidget: public QWidget, public BaseParticleEditorContentWidget
{
	Q_OBJECT
	
public:
	explicit ParticleEffectPropertiesWidget(QWidget* parent = 0);
	~ParticleEffectPropertiesWidget();

	void Init(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect);
	ParticleEffectComponent* GetEffect() {return particleEffect;};

	virtual void StoreVisualState(KeyedArchive* visualStateProps);
	virtual void RestoreVisualState(KeyedArchive* visualStateProps);

public slots:
	void OnValueChanged();
	
protected:
	void InitWidget(QWidget* widget, bool connectWidget = true);
	void UpdatePlaybackSpeedLabel();

private:
	ParticleEffectComponent* particleEffect;

	QVBoxLayout* mainLayout;

	QLabel* effectPlaybackSpeedLabel;
	QSlider* effectPlaybackSpeed;
	
	QCheckBox* checkboxStopOnLoad;
	
	bool blockSignals;
};


#endif /* defined(__PARTICLE_EFFECT_PROPERTIES_WIDGET__H__) */
