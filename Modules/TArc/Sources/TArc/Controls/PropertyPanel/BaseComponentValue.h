#pragma once

#include "PropertyModelExtensions.h"
#include "TArc/Controls/ControlProxy.h"

#include "Reflection/Reflection.h"
#include "Base/BaseTypes.h"

#include <QString>
#include <QRect>

class QWidget;
class QStyleOptionViewItem;
class QModelIndex;
class QStyle;
class QPainter;
class QEvent;

namespace DAVA
{
namespace TArc
{
class ReflectedPropertyModel;
class DataContext;
class DataWrappersProcessor;
class StaticEditorDrawer;
class BaseComponentValue;
struct PropertyNode;

class BaseComponentValue : public ReflectionBase
{
public:
    BaseComponentValue();
    virtual ~BaseComponentValue();

    void Init(ReflectedPropertyModel* model);

    virtual bool EditorEvent(QWidget* parent, QEvent* event, const QStyleOptionViewItem& option);

    void Draw(QWidget* parent, QPainter* painter, const QStyleOptionViewItem& opt);
    void UpdateGeometry(QWidget* parent, const QStyleOptionViewItem& opt);
    bool HasHeightForWidth(const QWidget* parent) const;
    int GetHeightForWidth(const QWidget* parent, int width) const;
    int GetHeight(const QWidget* parent) const;

    QWidget* AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option);
    void ReleaseEditorWidget(QWidget* editor);

    QString GetPropertyName() const;
    int32 GetPropertiesNodeCount() const;
    std::shared_ptr<const PropertyNode> GetPropertyNode(int32 index) const;

    virtual bool IsReadOnly() const;
    static const char* readOnlyFieldName;

protected:
    friend class ComponentStructureWrapper;
    friend class ReflectedPropertyItem;

    virtual Any GetMultipleValue() const = 0;
    virtual bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const = 0;
    virtual ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) const = 0;

    Any GetValue() const;
    void SetValue(const Any& value);

    std::shared_ptr<ModifyExtension> GetModifyInterface();

    void AddPropertyNode(const std::shared_ptr<PropertyNode>& node);
    void RemovePropertyNode(const std::shared_ptr<PropertyNode>& node);
    void RemovePropertyNodes();

    mutable ControlProxy* editorWidget = nullptr;
    Vector<std::shared_ptr<PropertyNode>> nodes;

private:
    void EnsureEditorCreated(const QWidget* parent) const;
    void UpdateEditorGeometry(const QWidget* parent, const QRect& geometry) const;

    ReflectedPropertyModel* model = nullptr;
    BaseComponentValue* thisValue = nullptr;
    bool isEditorEvent = false;

    DAVA_VIRTUAL_REFLECTION(BaseComponentValue);
};
}
}
