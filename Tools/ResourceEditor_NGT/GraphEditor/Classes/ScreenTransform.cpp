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

#include "ScreenTransform.h"

#include <assert.h>

namespace
{
void UpdateMatrices(QMatrix& translatableMatrix, QMatrix& invertexMatrix, float dx, float dy)
{
    translatableMatrix = translatableMatrix.translate(dx, dy);
    bool invertible = false;
    invertexMatrix = translatableMatrix.inverted(&invertible);
    assert(invertible == true);
}
}

ScreenTransform& ScreenTransform::Instance()
{
    static ScreenTransform transform;
    return transform;
}

ScreenTransform::ScreenTransform()
{
    // move model ZeroPoint into screen center
    UpdateMatrices(pixelToGlobal, globalToPixel, -pixelSize.width() / 2.0f, -pixelSize.height() / 2.0f);
}

void ScreenTransform::Resize(QSizeF const& newPixelSize)
{
    float shiftX = (newPixelSize.width() - pixelSize.width()) / 2.0f;
    float shiftY = (newPixelSize.height() - pixelSize.height()) / 2.0f;
    pixelSize = newPixelSize;
    // screen center was moved in Vector(shiftX, shiftY). Move model ZeroPoint in opposite value
    UpdateMatrices(pixelToGlobal, globalToPixel, -shiftX, -shiftY);

    TransformChanged();
}

void ScreenTransform::Shift(QPointF const& pixelShift)
{
    QPointF scaledShift = pixelShift / scale;
    // move model ZeroPoint on canvas
    UpdateMatrices(globalToPixel, pixelToGlobal, scaledShift.x(), scaledShift.y());
    globalOrg = pixelToGlobal.map(QPointF(pixelSize.width() / 2.0, pixelSize.height() / 2.0));

    TransformChanged();
}

void ScreenTransform::Scale(float scaleFactor, float x, float y)
{
    QPointF scaleShift = QPointF(x, y) * (1 - scaleFactor);
    globalToPixel = globalToPixel * QMatrix(scaleFactor, 0.0, 0.0, scaleFactor, scaleShift.x(), scaleShift.y());
    pixelToGlobal = globalToPixel.inverted();
    scale = globalToPixel.m11();
    globalOrg = pixelToGlobal.map(QPointF(pixelSize.width() / 2.0, pixelSize.height() / 2.0));

    TransformChanged();
}

float ScreenTransform::GetScale() const
{
    return scale;
}

QPointF ScreenTransform::GtoP(QPointF const& pixelPoint) const
{
    return globalToPixel.map(pixelPoint);
}

QPointF ScreenTransform::PtoG(QPointF const& globalPoint) const
{
    return pixelToGlobal.map(globalPoint);
}
