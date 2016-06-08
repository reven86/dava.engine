#include "EditorSystems/CursorSystem.h"
#include <QApplication>
#include "Debug/DVAssert.h"
#include "Model/PackageHierarchy/ControlNode.h"
#include "UI/UIControl.h"
#include <QPixmap>
#include <QTransform>
#include "EditorCore.h"
#include "QtTools/DavaGLWidget/Davaglwidget.h"

using namespace DAVA;

QMap<QString, QPixmap> CursorSystem::cursorpixes;

CursorSystem::CursorSystem(EditorSystemsManager* parent)
    : BaseEditorSystem(parent)
{
    systemsManager->ActiveAreaChanged.Connect(this, &CursorSystem::OnActiveAreaChanged);
}

void CursorSystem::OnActiveAreaChanged(const HUDAreaInfo& areaInfo)
{
    DavaGLWidget* davaGLWidget = EditorCore::Instance()->GetMainWindow()->previewWidget->GetGLWidget();
    if (areaInfo.area == HUDAreaInfo::NO_AREA)
    {
        davaGLWidget->UnsetCursor();
    }
    else
    {
        auto control = areaInfo.owner->GetControl();
        float angle = control->GetGeometricData().angle;
        QPixmap pixmap = CreatePixmapForArea(angle, areaInfo.area);
        davaGLWidget->SetCursor(QCursor(pixmap));
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

QPixmap CursorSystem::CreatePixmap(const QString& address) const
{
    if (cursorpixes.contains(address))
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