#pragma once

#include "TArc/Controls/PropertyPanel/StaticEditorDrawer.h"

namespace DAVA
{
namespace TArc
{
struct PropertyNode;
class EmptyEditorDrawer : public StaticEditorDrawer
{
public:
    ~EmptyEditorDrawer() override = default;
    void InitStyleOptions(Params& params) const override;
    uint32 GetHeight(Params params) const override;
    void Draw(QPainter* painter, Params params) const override;
};

class TextEditorDrawer : public StaticEditorDrawer
{
public:
    ~TextEditorDrawer() override = default;
    void InitStyleOptions(Params& params) const override;
    uint32 GetHeight(Params params) const override;
    void Draw(QPainter* painter, Params params) const override;
};

class BoolEditorDrawer : public StaticEditorDrawer
{
public:
    ~BoolEditorDrawer() override = default;
    void InitStyleOptions(Params& params) const override;
    uint32 GetHeight(Params params) const override;
    void Draw(QPainter* painter, Params params) const override;

    QString GetTextHint(const Any& value, const Vector<std::shared_ptr<PropertyNode>>* nodes) const;
};

class EnumEditorDrawer : public StaticEditorDrawer
{
public:
    ~EnumEditorDrawer() override = default;
    void InitStyleOptions(Params& params) const override;
    uint32 GetHeight(Params params) const override;
    void Draw(QPainter* painter, Params params) const override;

    QString GetTextHint(const Any& value, const Vector<std::shared_ptr<PropertyNode>>* nodes) const;
};

} // namespace TArc
} // namespace DAVA
