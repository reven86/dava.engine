#pragma once

#include <Base/Any.h>

class QStyle;
class QStyleOptionViewItem;
class QPainter;
class QModelIndex;

namespace DAVA
{
namespace TArc
{
class StaticEditorDrawer
{
public:
    virtual ~StaticEditorDrawer() = default;

    virtual uint32 GetHeight(QStyle* style, const QStyleOptionViewItem& options, const Any& value) const = 0;
    virtual void Draw(QStyle* style, QPainter* painter, const QStyleOptionViewItem& options, const Any& value) const = 0;
};

} // namespace TARC
} // namespace DAVA