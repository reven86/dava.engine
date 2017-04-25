#pragma once

#include "EditorSystems/BaseEditorSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include <QMap>
#include <QPixmap>
#include <QString>
#include <QCursor>

namespace DAVA
{
class RenderWidget;
}

class CursorSystem final : public BaseEditorSystem
{
public:
    explicit CursorSystem(DAVA::RenderWidget* renderWidget, EditorSystemsManager* doc, DAVA::TArc::ContextAccessor* accessor);
    ~CursorSystem() override = default;

private:
    void OnDragStateChanged(EditorSystemsManager::eDragState currentState, EditorSystemsManager::eDragState previousState) override;

    void OnActiveAreaChanged(const HUDAreaInfo& areaInfo);

    QPixmap CreatePixmapForArea(float angle, const HUDAreaInfo::eArea area) const;
    QPixmap CreatePixmap(const QString& address) const;

    static QMap<QString, QPixmap> cursorpixes;

    DAVA::RenderWidget* renderWidget = nullptr;
};
