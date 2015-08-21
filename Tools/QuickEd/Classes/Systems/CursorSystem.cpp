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

#include "Systems/CursorSystem.h"
#include <QApplication>
#include "Debug/DVAssert.h"

using namespace DAVA;

void CursorSystem::MouseEnterArea(ControlNode* targetNode, const eArea area)
{
    QCursor cursor = GetCursorByArea(area);
    if (shape == cursor.shape() && shapesCount)
    {
        return;
    }
    shape = cursor.shape();
    shapesCount++;
    qApp->setOverrideCursor(cursor);
}

void CursorSystem::MouseLeaveArea()
{
    while (shapesCount)
    {
        shapesCount--;
        qApp->restoreOverrideCursor();
    }
}

QCursor CursorSystem::GetCursorByArea(const eArea area) const
{
    switch (area)
    {
    case FRAME:
    case PIVOT_POINT:
        return Qt::SizeAllCursor;
    case TOP_LEFT:
    case BOTTOM_RIGHT:
        return Qt::SizeFDiagCursor;
    case TOP_RIGHT:
    case BOTTOM_LEFT:
        return Qt::SizeBDiagCursor;
    case TOP_CENTER:
    case BOTTOM_CENTER:
        return Qt::SizeVerCursor;
    case CENTER_LEFT:
    case CENTER_RIGHT:
        return Qt::SizeHorCursor;
    case ROTATE:
        return Qt::CrossCursor;
    default:
        DVASSERT_MSG(false, "unexpected enum value");
        return QCursor();
    }
}
