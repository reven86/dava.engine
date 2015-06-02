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


#ifndef __DISTANCE_SLIDER_H__
#define __DISTANCE_SLIDER_H__

#include "DAVAEngine.h"

#include <QFrame>
#include <QSplitter>
#include <QSplitterHandle>
#include <QFrame>
#include <QSet>

class DistanceSplitterHandle : public QSplitterHandle
{
public:
    DistanceSplitterHandle(Qt::Orientation o, QSplitter *parent) 
        : QSplitterHandle(o, parent)
    { }

protected:
    virtual void mouseReleaseEvent(QMouseEvent * e)
    {
        bool opq = splitter()->opaqueResize();

        splitter()->setOpaqueResize(false);
        QSplitterHandle::mouseReleaseEvent(e);
        splitter()->setOpaqueResize(opq);
    }
};


class DistanceSplitter : public QSplitter
{
public:
    DistanceSplitter(QWidget *parent = 0)
        : QSplitter(parent)
    { }

protected:
    virtual QSplitterHandle* createHandle()
    {
        return new DistanceSplitterHandle(orientation(), this);
    }
};

class DistanceSlider: public QFrame
{
    Q_OBJECT

public:
    DistanceSlider(QWidget *parent = 0);
    ~DistanceSlider();

    void SetLayersCount(int count);
    inline int GetLayersCount() const;

    void SetDistance(int layer, double value);
    double GetDistance(int layer) const;

    void LockDistances(bool lock);

signals:
    void DistanceChanged(const QVector<int> &changedLayers, bool continious);
    
protected slots:
    void SplitterMoved(int pos, int index);

protected:
    int GetScaleSize();
    
private:
    QSplitter *splitter;
    QFrame *frames[DAVA::LodComponent::MAX_LOD_LAYERS];
    bool locked;
    
    int layersCount;
    
    int stretchSize[DAVA::LodComponent::MAX_LOD_LAYERS];
};

inline int DistanceSlider::GetLayersCount() const
{
    return layersCount;
}

#endif // __DISTANCE_SLIDER_H__
