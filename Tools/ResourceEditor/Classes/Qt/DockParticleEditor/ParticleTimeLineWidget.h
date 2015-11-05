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


#ifndef __ResourceEditorQt__ParticleTimeLineWidget__
#define __ResourceEditorQt__ParticleTimeLineWidget__

#include <QWidget>
#include <QDialog>
#include <QLabel>
#include <QTimer>

#include <DAVAEngine.h>
#include "ScrollZoomWidget.h"
#include "Tools/EventFilterDoubleSpinBox/EventFilterDoubleSpinBox.h"
#include "Scene/SceneEditor2.h"

using namespace DAVA;

#define LEFT_INDENT 20
#define TOP_INDENT 14
#define BOTTOM_INDENT 24
#define RIGHT_INDENT 317
#define LINE_STEP 16
#define RECT_SIZE 3
#define LINE_WIDTH 3

#define PARTICLES_INFO_CONTROL_OFFSET 8

#define UPDATE_LAYERS_EXTRA_INFO_PERIOD 250 // in milliseconds

class ParticlesExtraInfoColumn;
class ParticleTimeLineWidget : public ScrollZoomWidget
{
    Q_OBJECT
    friend class ParticlesExtraInfoColumn;

public:
    explicit ParticleTimeLineWidget(QWidget* parent = 0);
    ~ParticleTimeLineWidget();

    void Init(float32 minTime, float32 maxTime);

    struct LINE
    {
        float32 startTime;
        float32 endTime;
        float32 deltaTime;
        float32 loopEndTime;
        bool isLooped;
        bool hasLoopVariation;
        QColor color;
        QString legend;
        ParticleLayer* layer;
    };
    typedef DAVA::Map<uint32, LINE> LINE_MAP;

signals:
    void ChangeVisible(bool visible);

protected slots:

    // Scene Tree signals - Particles are selected.
    void OnEffectSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect);
    void OnEmitterSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter);
    void OnInnerEmitterSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter);
    void OnLayerSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleEffectComponent* effect, DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer, bool forceRefresh);
    void OnForceSelectedFromSceneTree(SceneEditor2* scene, DAVA::ParticleLayer* layer, int forceIndex);

    // Scene Tree signals - values are changed for Particles.
    void OnParticleEmitterValueChanged(SceneEditor2* scene, DAVA::ParticleEmitter* emitter);
    void OnParticleLayerValueChanged(SceneEditor2* scene, DAVA::ParticleLayer* layer);
    void OnParticleEffectStateChanged(SceneEditor2* scene, DAVA::Entity* effect, bool isStarted);

    // Scene Tree signals - Particle Emitter is loaded.
    void OnParticleEmitterLoaded(SceneEditor2* scene, DAVA::ParticleEmitter* emitter);

    // Scene Tree signals - structure changes.
    void OnParticleLayerAdded(SceneEditor2* scene, DAVA::ParticleEmitter* emitter, DAVA::ParticleLayer* layer);
    void OnParticleLayerRemoved(SceneEditor2* scene, DAVA::ParticleEmitter* emitter);

    void OnUpdate();

    void OnUpdateLayersExtraInfoNeeded();

protected:
    void OnParticleEffectSelected(DAVA::ParticleEffectComponent* effect);

    virtual void paintEvent(QPaintEvent*);
    virtual void mouseMoveEvent(QMouseEvent*);
    virtual void mousePressEvent(QMouseEvent*);
    virtual void mouseReleaseEvent(QMouseEvent*);
    virtual void mouseDoubleClickEvent(QMouseEvent*);

private:
    bool GetLineRect(uint32 id, QRect& startPoint, QRect& endPoint) const;
    bool GetLoopedLineRect(uint32 id, QRect& startPoint, QRect& endPoint, float32 startTime, float32 endTime) const;
    QRect GetGraphRect() const;
    QPoint GetPoint(const QPoint&) const;

    void AddLayerLine(uint32 layerLineID, float32 minTime, float32 maxTime,
                      const QColor& layerColor, ParticleLayer* layer);
    void AddLine(uint32 lineId, float32 startTime, float32 endTime, float32 deltaTime, float32 loopEndTime, bool isLooped,
                 bool hasLoopVariation, const QColor& color, const QString& legend, ParticleLayer* layer);

    void OnValueChanged(int lineId);
    void UpdateSizePolicy();

    void ShowLayersExtraInfoValues(bool isVisible);

    void NotifyLayersExtraInfoChanged();
    void UpdateLayersExtraInfoPosition();
    void UpdateLayersExtraInfoValues();
    void ResetLayersExtraInfoValues();

    // Handle situation when the Particle Emitter Node is selected (including
    // case when separate Layer node is selected.
    void HandleEmitterSelected(ParticleEffectComponent* effect, ParticleEmitter* emitter, ParticleLayer* layer);

    virtual QRect GetSliderRect() const;
    virtual QRect GetIncreaseRect() const;
    virtual QRect GetScaleRect() const;
    virtual QRect GetDecreaseRect() const;

private:
    // Get the width/height for particle counter label.
    void GetParticlesCountWidthHeight(const LINE& line, int32& width, int32& height);

    // Cleanup all the timelines and info.
    void CleanupTimelines();

    LINE_MAP lines;
    QFont nameFont;

    QPoint selectedPoint;

    ParticleEffectComponent* selectedEffect;
    ParticleEmitter* selectedEmitter;

    int32 selectedLine;
    int32 selectedLineOrigin;
    Entity* emitterNode;
    Entity* effectNode;
    ParticleLayer* selectedLayer;

    QTimer updateTimer;

    // List of data columns.
    List<ParticlesExtraInfoColumn*> infoColumns;

    // Scene currently selected.
    SceneEditor2* activeScene;

    class SetPointValueDlg : public QDialog
    {
        //Q_OBJECT

    public:
        explicit SetPointValueDlg(float32 value, float32 minValue, float32 maxValue, QWidget* parent = 0);

        float32 GetValue() const;

    private:
        EventFilterDoubleSpinBox* valueSpin;
    };
};

#endif /* defined(__ResourceEditorQt__ParticleTimeLineWidget__) */
