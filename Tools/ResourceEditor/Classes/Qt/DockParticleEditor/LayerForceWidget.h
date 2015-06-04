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


#ifndef __ResourceEditorQt__LayerForceWidget__
#define __ResourceEditorQt__LayerForceWidget__

#include <DAVAEngine.h>

#include <QWidget>
#include "BaseParticleEditorContentWidget.h"

using namespace DAVA;

class TimeLineWidget;
class QVBoxLayout;

class LayerForceWidget: public QWidget, public BaseParticleEditorContentWidget
{
    Q_OBJECT
    
public:
    explicit LayerForceWidget(QWidget *parent = 0);
    ~LayerForceWidget();
	
	void Init(SceneEditor2* scene, ParticleLayer* layer, uint32 forceIndex, bool updateMinimized);
	ParticleLayer* GetLayer() const {return layer;};
	int32 GetForceIndex() const {return forceIndex;};

	void Update();

	virtual void StoreVisualState(KeyedArchive* visualStateProps);
	virtual void RestoreVisualState(KeyedArchive* visualStateProps);

signals:
	void ValueChanged();
	
protected slots:
	void OnValueChanged();
	
protected:
	void InitWidget(QWidget* widget);
	
private:
	QVBoxLayout* mainBox;
	ParticleLayer* layer;
	int32 forceIndex;
	
	TimeLineWidget* forceTimeLine;	
	TimeLineWidget* forceOverLifeTimeLine;
	
	bool blockSignals;
};

#endif /* defined(__ResourceEditorQt__LayerForceWidget__) */
