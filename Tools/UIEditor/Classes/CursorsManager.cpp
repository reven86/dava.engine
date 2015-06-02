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


#include "CursorsManager.h"

#include <QPixmap>

CursorsManager::CursorInfo CursorsManager::cursorsInfo[] =
{
    { Qt::ArrowCursor, ":/Cursors/Cursors/arrow.png", QPoint(0, 0) },
    { Qt::SizeHorCursor, ":/Cursors/Cursors/left_right.png", QPoint(16, 17) },
    { Qt::SizeVerCursor, ":/Cursors/Cursors/up_down.png", QPoint(16, 17) },
    { Qt::SizeAllCursor, ":/Cursors/Cursors/size_all.png", QPoint(16, 17) },
    { Qt::SizeBDiagCursor, ":/Cursors/Cursors/left_corner.png", QPoint(16, 17) },
    { Qt::SizeFDiagCursor, ":/Cursors/Cursors/right_corner.png", QPoint(16, 17) },
    { Qt::OpenHandCursor, ":/Cursors/Cursors/open_hand.png", QPoint(6, 0) }
};
    
CursorsManager::CursorsManager()
{
    LoadCursors();
}

CursorsManager::~CursorsManager()
{
}

void CursorsManager::LoadCursors()
{
    cursorsMap.clear();

    int cursorsCount = COUNT_OF(cursorsInfo);
    for (int i = 0; i < cursorsCount; i ++)
    {
        const CursorsManager::CursorInfo& info = cursorsInfo[i];
        QPixmap pixMap(info.path);
        cursorsMap[info.shape] = QCursor(pixMap, info.hotSpot.x(), info.hotSpot.y());
    }
}

const QCursor& CursorsManager::GetCursor(Qt::CursorShape cursorShape)
{
    DAVA::Map<Qt::CursorShape, QCursor>::iterator iter = cursorsMap.find(cursorShape);
    DVASSERT(iter != cursorsMap.end());

    return iter->second;
}

