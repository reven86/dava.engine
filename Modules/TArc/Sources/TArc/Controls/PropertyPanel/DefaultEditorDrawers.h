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
    void InitStyleOptions(Params& params) const override;
    uint32 GetHeight(Params params) const override;
    void Draw(QPainter* painter, Params params) const override;
};

class TextEditorDrawer : public StaticEditorDrawer
{
public:
    void InitStyleOptions(Params& params) const override;
    uint32 GetHeight(Params params) const override;
    void Draw(QPainter* painter, Params params) const override;
};

class BoolEditorDrawer : public StaticEditorDrawer
{
public:
    void InitStyleOptions(Params& params) const override;
    uint32 GetHeight(Params params) const override;
    void Draw(QPainter* painter, Params params) const override;

    QString GetTextHint(const Any& value, const Vector<std::shared_ptr<PropertyNode>>* nodes) const;
};

class DefaultEnumEditorDrawer : public StaticEditorDrawer
{
public:
    void InitStyleOptions(Params& params) const override;
    uint32 GetHeight(Params params) const override;
    void Draw(QPainter* painter, Params params) const override;

    virtual QString GetTextHint(const Any& value, const Vector<std::shared_ptr<PropertyNode>>* nodes) const = 0;
};

class EnumEditorDrawer : public DefaultEnumEditorDrawer
{
public:
    QString GetTextHint(const Any& value, const Vector<std::shared_ptr<PropertyNode>>* nodes) const override;
};

class FlagsEditorDrawer : public DefaultEnumEditorDrawer
{
public:
    QString GetTextHint(const Any& value, const Vector<std::shared_ptr<PropertyNode>>* nodes) const override;
};

} // namespace TArc
} // namespace DAVA
