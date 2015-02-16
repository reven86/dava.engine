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


#include "DistanceSlider.h"

#include "DAVAEngine.h"

#include <QHBoxLayout>
#include <QApplication>

static const QColor backgroundColors[DAVA::LodComponent::MAX_LOD_LAYERS] =
{
    Qt::green,
    Qt::red,
    Qt::yellow,
    Qt::blue
};

static const int MIN_WIDGET_WIDTH = 1;

DistanceSlider::DistanceSlider(QWidget *parent /*= 0*/)
    : QFrame(parent)
    , layersCount(0)
    , locked(false)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setObjectName(QString::fromUtf8("layout"));
    
    splitter = new DistanceSplitter(this);
    splitter->setObjectName(QString::fromUtf8("splitter"));
    splitter->setGeometry(geometry());
    splitter->setOrientation(Qt::Horizontal);
    splitter->setMinimumHeight(20);
    splitter->setChildrenCollapsible(false);
//    splitter->setOpaqueResize(false); need uncomment if moving will be slow
    
    layout->addWidget(splitter);
    layout->setSpacing(0);
    layout->setMargin(0);
    
    
    for(int i = 0 ; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
    {
        frames[i] = new QFrame(splitter);

        frames[i]->setObjectName(QString::fromUtf8(DAVA::Format("frame_%d",  i).c_str()));
        frames[i]->setFrameShape(QFrame::StyledPanel);
        frames[i]->setFrameShadow(QFrame::Raised);
        
        QPalette pallete;
        pallete.setColor(QPalette::Background, backgroundColors[i]);
        
        
        frames[i]->setPalette(pallete);
        frames[i]->setAutoFillBackground(true);

        frames[i]->setMinimumWidth(MIN_WIDGET_WIDTH);
        
        
        splitter->addWidget(frames[i]);
        
        stretchSize[i] = 0;
    }

    setLayout(layout);
    
    connect(splitter, SIGNAL(splitterMoved(int, int)), SLOT(SplitterMoved(int, int)));
}


DistanceSlider::~DistanceSlider()
{

}


void DistanceSlider::LockDistances(bool lock)
{
    locked = lock;
}

void DistanceSlider::SetLayersCount(int count)
{
    layersCount = count;
    for(int i = 0; i < DAVA::LodComponent::MAX_LOD_LAYERS; ++i)
    {
        frames[i]->setVisible(i < layersCount);
        splitter->setStretchFactor(i, 0);
    }
}

void DistanceSlider::SetDistance(int layer, double value)
{
    if(!locked)
    {
        DVASSERT(0 <= layer && layer < DAVA::LodComponent::MAX_LOD_LAYERS);
        stretchSize[layer] = value;
    
        int scaleSize = GetScaleSize();
    
        QList<int> sizes;
        for(int i = 1; i < layersCount; ++i)
        {
            sizes.push_back((stretchSize[i] - stretchSize[i-1]) * splitter->geometry().width() / scaleSize);
        }
    
        if(layersCount)
            sizes.push_back((scaleSize - stretchSize[layer]) * splitter->geometry().width() / scaleSize);

    
        bool wasBlocked = splitter->blockSignals(true);
        splitter->setSizes(sizes);
        splitter->blockSignals(wasBlocked);
    }
}

double DistanceSlider::GetDistance(int layer) const
{
    DVASSERT(0 <= layer && layer < DAVA::LodComponent::MAX_LOD_LAYERS);

    return stretchSize[layer];
}

void DistanceSlider::SplitterMoved(int pos, int index)
{
    if(layersCount > 0)
    {
        QList<int> sizes = splitter->sizes();
        
        double scaleSize = GetScaleSize();
    
        QVector<int> changedLayers;

        int fullSize = 0;
        int splitterWidth = splitter->geometry().width() - splitter->handleWidth() * (layersCount - 1);

        for (int i = 0; i < layersCount; ++i)
        {
            int sz = sizes.at(i);
            double value = fullSize * scaleSize / splitterWidth;

            if(stretchSize[i] != value)
            {
                stretchSize[i] = value;
                changedLayers.push_front(i);
            }

            fullSize += sz;
        }
    
        if(changedLayers.size() > 0)
        {
            emit DistanceChanged(changedLayers, QApplication::mouseButtons() & Qt::LeftButton);
        }
    }
}

int DistanceSlider::GetScaleSize()
{
    if(layersCount == DAVA::LodComponent::MAX_LOD_LAYERS)
    {
        return DAVA::LodComponent::MAX_LOD_DISTANCE + 100;
    }
    
    return DAVA::LodComponent::MAX_LOD_DISTANCE;
}
