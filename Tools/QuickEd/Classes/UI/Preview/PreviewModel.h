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


#ifndef __QUICKED_PREVIEW_MODEL_H__
#define __QUICKED_PREVIEW_MODEL_H__

#include <QObject>
#include <QPoint>
#include <QSize>
#include "DAVAEngine.h"

#include "Base/Result.h"
#include "ControlSelectionListener.h"

class PackageCanvas;
class ControlNode;
class CheckeredCanvas;

class PreviewModel : public QObject, ControlSelectionListener
{
    Q_OBJECT
public:
    PreviewModel(QObject *parent);
    virtual ~PreviewModel();

    void SetViewControlSize(const QSize &newSize);
    void SetCanvasControlSize(const QSize &newSize);
    void SetCanvasControlScale(int newScale);

    void SetCanvasPosition(const QPoint &newPosition);
    QPoint GetCanvasPosition() const;
    int GetCanvasScale() const;

    QSize GetScaledCanvasSize() const;
    QSize GetViewSize() const;
    DAVA::UIControl *GetViewControl() const;

    void SetRootControls(const QList<ControlNode*> &activatedControls);
    void SetSelectedControls(const QList<ControlNode *> &selectedControls);
    
    void SetEmulationMode(bool emulationMode);

    // ControlSelectionListener
    virtual void OnControlSelected(const DAVA::List<std::pair<DAVA::UIControl *, DAVA::UIControl*> > &selectedPairs) override;
signals:
    void CanvasPositionChanged(const QPoint &canvasPosition);
    void CanvasOrViewChanged(const QSize &viewSize, const QSize &scaledContentSize);
    void CanvasScaleChanged(int canvasScale);

    void ControlNodeSelected(const QList<ControlNode *> &selectedNodes);

    void ErrorOccurred(const DAVA::ResultList &error);

private:
    CheckeredCanvas *FindControlContainer(DAVA::UIControl *control);

private:
    DAVA::Vector2 canvasPosition;

    PackageCanvas *canvas;
    DAVA::UIControl *view;

    DAVA::Map<DAVA::UIControl*, ControlNode*> rootNodes;
    bool emulationMode;
};

inline DAVA::UIControl *PreviewModel::GetViewControl() const 
{
    return view;
}


#endif // __QUICKED_PREVIEW_MODEL_H__
