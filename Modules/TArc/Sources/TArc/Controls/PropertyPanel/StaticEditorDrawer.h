#pragma once

#include <Base/Any.h>

#include <QStyleOption>

class QStyle;
class QPainter;
class QModelIndex;

namespace DAVA
{
namespace TArc
{
struct PropertyNode;
class StaticEditorDrawer
{
public:
    virtual ~StaticEditorDrawer() = default;

    struct Params
    {
        const QStyle* style = nullptr;
        QStyleOptionViewItem options;
        Any value;
        const Vector<std::shared_ptr<PropertyNode>>* nodes;
    };

    virtual void InitStyleOptions(Params& params) const = 0;
    virtual uint32 GetHeight(Params params) const = 0;
    virtual void Draw(QPainter* painter, Params params) const = 0;
};

} // namespace TArc
} // namespace DAVA