#pragma once

#include "TArc/Controls/PropertyPanel/ProxyComponentValue.h"
#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/DefaultEditorDrawers.h"
#include "TArc/Controls/PropertyPanel/DefaultValueCompositors.h"

#include <QObject>
#include <QPointer>

namespace DAVA
{
namespace TArc
{
class BoolComponentValue : public ProxyComponentValue<BoolEditorDrawer, BoolValueCompositor>, private QObject
{
public:
    BoolComponentValue() = default;

protected:
    QWidget* AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option) override;
    void ReleaseEditorWidget(QWidget* editor) override;
    bool EditorEvent(QEvent* event, const QStyleOptionViewItem& option) override;

private:
    Qt::CheckState GetCheckState() const;
    void SetCheckState(Qt::CheckState checkState);

    String GetTextHint() const;
    bool IsReadOnly() const;
    bool IsEnabled() const;

    bool HitEventAndResend(QEvent* event, const QStyleOptionViewItem& option);

private:
    DAVA_VIRTUAL_REFLECTION(BoolComponentValue, ProxyComponentValue<BoolEditorDrawer, BoolValueCompositor>);
    bool implCall = false;
};
}
}
