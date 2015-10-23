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


#include "GridVisualizer.h"
#include "Render/2D/Systems/RenderSystem2D.h"
using namespace DAVA;
// Construction/destruction.
GridVisualizer::GridVisualizer() :
    curScale(0.0f)
{
}

GridVisualizer::~GridVisualizer()
{

}

void GridVisualizer::SetScale(float32 scale)
{
    curScale = scale;
}

void GridVisualizer::DrawGridIfNeeded(const Rect& rect, UniqueHandle renderState)
{
    // Grid constants.
    static const Color gridColor = Color(0.5f, 0.5f, 0.5f, 0.5f); // TODO: customize with designers.
    static const Vector2 gridSize = Vector2(1.0f, 1.0f);
    static const float32 scaleTresholdToDrawGrid = 8.0f; // 800%+ as requested.

    if (curScale < scaleTresholdToDrawGrid)
    {
        return;
    }

    RenderSystem2D::Instance()->DrawGrid(rect, gridSize, gridColor);
}