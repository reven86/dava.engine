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

#include "Base/BaseTypes.h"
#include "Scene3D/Components/LodComponent.h"

#include <QWidget>

class QSplitter;
class QFrame;

class LazyUpdater;

class DistanceSlider : public QWidget
{
    Q_OBJECT

public:
    DistanceSlider(QWidget* parent = 0);

    void SetFramesCount(DAVA::uint32 count);
    DAVA::uint32 GetFramesCount() const;

    void SetLayersCount(DAVA::uint32 count);
    DAVA::uint32 GetLayersCount() const;

    void SetDistances(const DAVA::Vector<DAVA::float32>& distances);
    const DAVA::Vector<DAVA::float32>& GetDistances() const;

signals:
    void DistanceHandleMoved();
    void DistanceHandleReleased();

protected slots:
    void SplitterMoved(int pos, int index);

protected:
    bool eventFilter(QObject* obj, QEvent* e) override;

private:
    DAVA::float32 GetScaleSize() const;

private:
    QSplitter* splitter = nullptr;
    DAVA::Vector<QObject*> splitterHandles;

    DAVA::Vector<QFrame*> frames;
    DAVA::Vector<DAVA::float32> realDistances;

    DAVA::uint32 layersCount = 0;
    DAVA::uint32 framesCount = 0;
};

inline DAVA::uint32 DistanceSlider::GetLayersCount() const
{
    return layersCount;
}

inline DAVA::uint32 DistanceSlider::GetFramesCount() const
{
    return framesCount;
}

#endif // __DISTANCE_SLIDER_H__
