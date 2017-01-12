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
    virtual void Draw(QStyle* style, QPainter* painter, const QStyleOptionViewItem& options, const Any& value) = 0;
};

} // namespace TARC
} // namespace DAVA