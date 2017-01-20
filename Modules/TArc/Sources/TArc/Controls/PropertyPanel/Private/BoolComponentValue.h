#pragma once

#include "TArc/Controls/PropertyPanel/ProxyComponentValue.h"
#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/DefaultEditorDrawers.h"
#include "TArc/Controls/PropertyPanel/DefaultValueCompositors.h"

namespace DAVA
{
namespace TArc
{
class BoolComponentValue : public ProxyComponentValue<BoolEditorDrawer, BoolValueCompositor>
{
public:
    BoolComponentValue() = default;

protected:
    QWidget* AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option) override;
    void ReleaseEditorWidget(QWidget* editor) override;

private:
    Qt::CheckState GetCheckState() const;
    void SetCheckState(Qt::CheckState checkState);

    bool IsReadOnly() const;
    bool IsEnabled() const;

private:
    DAVA_VIRTUAL_REFLECTION(BoolComponentValue, ProxyComponentValue<BoolEditorDrawer, BoolValueCompositor>);
};
}
}
