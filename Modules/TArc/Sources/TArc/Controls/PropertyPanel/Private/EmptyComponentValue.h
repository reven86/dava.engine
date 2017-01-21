#pragma once

#include "TArc/Controls/PropertyPanel/ProxyComponentValue.h"
#include "TArc/Controls/PropertyPanel/DefaultValueCompositors.h"
#include "TArc/Controls/PropertyPanel/DefaultEditorDrawers.h"
#include "Reflection/Reflection.h"

namespace DAVA
{
namespace TArc
{
class EmptyComponentValue : public ProxyComponentValue<EmptyEditorDrawer, EmptyValueCompositor>
{
public:
    QWidget* AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option) override;
    void ReleaseEditorWidget(QWidget* editor) override;

private:
    DAVA_VIRTUAL_REFLECTION(EmptyComponentValue, ProxyComponentValue<EmptyEditorDrawer, EmptyValueCompositor>);
};
}
} // namespace DAVA