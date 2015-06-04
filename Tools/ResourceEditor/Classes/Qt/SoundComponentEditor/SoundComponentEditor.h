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


#ifndef __SOUND_EDITOR_H__
#define __SOUND_EDITOR_H__

#include <QDialog>
#include <QListWidgetItem>
#include "DAVAEngine.h"
#include "Qt/Scene/SceneEditor2.h"

namespace Ui {
class SoundComponentEditor;
}

class SoundComponentEditor : public QDialog
{
    Q_OBJECT
    
public:
    explicit SoundComponentEditor(SceneEditor2* scene, QWidget *parent = 0);
    virtual ~SoundComponentEditor();
    
    void SetEditableEntity(DAVA::Entity * entity);

private slots:
    void OnEventSelected(QListWidgetItem * item);

    void OnSliderMoved(int value);
    void OnSliderPressed();

    void OnPlay();
    void OnStop();
    void OnAddEvent();
    void OnRemoveEvent();
    void OnAutoTrigger(bool checked);

private:
    struct ParamSliderData : public QObjectUserData
    {
        DAVA::String paramName;
        DAVA::float32 minValue;
        DAVA::float32 maxValue;
    };

    void FillEventsList();

    void AddSliderWidget(const DAVA::SoundEvent::SoundEventParameterInfo & param, float32 currentParamValue);
    void ClearParamsFrame();
    void FillEventParamsFrame();

    DAVA::Entity * entity;
    DAVA::SoundComponent * component;
    int32 selectedEventIndex;

    SceneEditor2* scene;

    Ui::SoundComponentEditor *ui;
};

#endif // __SOUND_EDITOR_H__
