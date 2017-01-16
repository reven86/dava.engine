#pragma once

#include "TArc/Controls/PropertyPanel/StaticEditorDrawer.h"

namespace DAVA
{
namespace TArc
{
class EmptyEditorDrawer : public StaticEditorDrawer
{
public:
    uint32 GetHeight(QStyle* style, const QStyleOptionViewItem& options, const Any& value) const override;
    void Draw(QStyle* style, QPainter* painter, const QStyleOptionViewItem& options, const Any& value) const override;
};

class TextEditorDrawer : public StaticEditorDrawer
{
public:
    uint32 GetHeight(QStyle* style, const QStyleOptionViewItem& options, const Any& value) const override;
    void Draw(QStyle* style, QPainter* painter, const QStyleOptionViewItem& options, const Any& value) const override;
};

} // namespace TArc
} // namespace DAVA