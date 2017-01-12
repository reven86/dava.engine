#pragma once

#include "TArc/Controls/PropertyPanel/StaticEditorDrawer.h"

namespace DAVA
{
namespace TArc
{
class TextEditorDrawer : public StaticEditorDrawer
{
public:
    void Draw(QStyle* style, QPainter* painter, const QStyleOptionViewItem& options, const Any& value) override;
};

} // namespace TArc
} // namespace DAVA