#ifndef __QUICKED_CURSOR_SYSTEM_H__
#define __QUICKED_CURSOR_SYSTEM_H__

#include "EditorSystems/BaseEditorSystem.h"
#include "EditorSystems/EditorSystemsManager.h"
#include <QMap>
#include <QPixmap>
#include <QString>

class CursorSystem final : public BaseEditorSystem
{
public:
    explicit CursorSystem(EditorSystemsManager* doc);
    ~CursorSystem() override = default;

private:
    void OnActiveAreaChanged(const HUDAreaInfo& areaInfo);

    QPixmap CreatePixmapForArea(float angle, const HUDAreaInfo::eArea area) const;
    QPixmap CreatePixmap(const QString& address) const;

    static QMap<QString, QPixmap> cursorpixes;
};

#endif // __QUICKED_TREE_SYSTEM_H__
