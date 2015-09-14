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
#include "Model/PackageHierarchy/ControlNode.h"
#include "UI/UIControl.h"
#include <QPixmap>
#include <QTransform>

using namespace DAVA;

QMap<QString, QPixmap> CursorSystem::cursorpixes;

CursorSystem::CursorSystem(Document* doc)
    : BaseSystem(doc)
{
    document->ActiveAreaChanged.Connect(this, &CursorSystem::SetActiveArea);
}

void CursorSystem::OnDeactivated()
{
    SetActiveArea(HUDAreaInfo());
}

void CursorSystem::SetActiveArea(const HUDAreaInfo &areaInfo)
{
    if (areaInfo.area == HUDAreaInfo::NO_AREA)
    {
        while (qApp->overrideCursor() != nullptr)
        {
            qApp->restoreOverrideCursor();
        }
    }
    else
    {
        auto control = areaInfo.owner->GetControl();
        float angle = control->GetGeometricData().angle;
        QPixmap pixmap = CreatePixmapForArea(angle, areaInfo.area);
        qApp->setOverrideCursor(QCursor(pixmap));
    }
}

QPixmap CursorSystem::CreatePixmapForArea(float angle, const HUDAreaInfo::eArea area) const
{
    QTransform transform;
    transform.rotateRadians(angle);
    QPixmap pixmap;
    switch (area)
    {
    case HUDAreaInfo::FRAME_AREA:
        return CreatePixmap(":/Cursors/moveCursor.png");
    case HUDAreaInfo::PIVOT_POINT_AREA:
        return CreatePixmap(":/Cursors/cursorCross.png");
    case HUDAreaInfo::TOP_LEFT_AREA:
    case HUDAreaInfo::BOTTOM_RIGHT_AREA:
        pixmap = CreatePixmap(":/Cursors/northWestSouthEastResizeCursor.png");
        return pixmap.transformed(transform);
    case HUDAreaInfo::TOP_RIGHT_AREA:
    case HUDAreaInfo::BOTTOM_LEFT_AREA:
        pixmap = CreatePixmap(":/Cursors/northEastSouthWestResizeCursor.png");
        return pixmap.transformed(transform);
    case HUDAreaInfo::TOP_CENTER_AREA:
    case HUDAreaInfo::BOTTOM_CENTER_AREA:
        pixmap = CreatePixmap(":/Cursors/northSouthResizeCursor.png");
        return pixmap.transformed(transform);
    case HUDAreaInfo::CENTER_LEFT_AREA:
    case HUDAreaInfo::CENTER_RIGHT_AREA:
        pixmap = CreatePixmap(":/Cursors/eastWestResizeCursor.png");
        return pixmap.transformed(transform);
    case HUDAreaInfo::ROTATE_AREA:
        return CreatePixmap(":/Cursors/cursorRotate.png");
    default:
        DVASSERT_MSG(false, "unexpected enum value");
        return QPixmap();
    }
}

QPixmap CursorSystem::CreatePixmap(const QString &address) const
{
    if(cursorpixes.contains(address))
    {
        return cursorpixes[address];
    }
    else
    {
        QPixmap pixmap(address);
        DVASSERT(!pixmap.isNull());
        cursorpixes.insert(address, pixmap);
        return pixmap;
    }
}