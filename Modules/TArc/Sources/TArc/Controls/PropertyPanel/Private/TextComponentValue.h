#pragma once

#include "TArc/Controls/PropertyPanel/BaseComponentValue.h"
#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"

#include <QString>

namespace DAVA
{
namespace TArc
{
class TextComponentValue : public BaseComponentValue
{
public:
    QWidget* AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) override;
    void ReleaseEditorWidget(QWidget* editor, const QModelIndex& index) override;
    void StaticEditorPaint(QStyle* style, QPainter* painter, const QStyleOptionViewItem& options) override;

protected:
    virtual Any Convert(const DAVA::String& text) const;

private:
    QString GetObjectName() const;

    DAVA::String GetText() const;
    void SetText(const DAVA::String& text);

    bool IsReadOnly() const;
    bool IsEnabled() const;

private:
    DAVA_VIRTUAL_REFLECTION(TextComponentValue, BaseComponentValue);
};

class FastNameComponentValue : public TextComponentValue
{
private:
    Any Convert(const DAVA::String& text) const;

    DAVA_VIRTUAL_REFLECTION(FastNameComponentValue, TextComponentValue);
};
}
}