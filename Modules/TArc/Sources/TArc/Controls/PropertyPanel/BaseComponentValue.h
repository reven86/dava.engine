#pragma once

#include "TArc/Controls/PropertyPanel/PropertyModelExtensions.h"
#include "TArc/Controls/PropertyPanel/PropertyPanelMeta.h"
#include "TArc/Controls/Widget.h"
#include "TArc/Controls/ControlProxy.h"
#include "TArc/Utils/QtConnections.h"
#include "TArc/WindowSubSystem/UI.h"

#include <Reflection/Reflection.h>
#include <Base/BaseTypes.h>
#include <Base/FastName.h>

#include <QString>
#include <QRect>

class QLayout;
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
    struct Style
    {
        Any bgColor; // Cast<QPalette::ColorRole> should be defined
        Any fontColor; // Cast<QPalette::ColorRole> should be defined
        Any fontBold; // Cast<bool> should be defined
        Any fontItalic; // Cast<bool> should be defined
    };

    BaseComponentValue();
    virtual ~BaseComponentValue();

    void Init(ReflectedPropertyModel* model);

    void Draw(QPainter* painter, const QStyleOptionViewItem& opt);
    void UpdateGeometry(const QStyleOptionViewItem& opt);
    bool HasHeightForWidth() const;
    int GetHeightForWidth(int width) const;
    int GetHeight() const;

    QWidget* AcquireEditorWidget(const QStyleOptionViewItem& option);
    void EnsureEditorCreated(QWidget* parent);

    QString GetPropertyName() const;
    FastName GetID() const;
    int32 GetPropertiesNodeCount() const;
    std::shared_ptr<PropertyNode> GetPropertyNode(int32 index) const;

    void ForceUpdate();

    virtual bool IsReadOnly() const;
    virtual bool IsSpannedControl() const;
    virtual bool RepaintOnUpdateRequire() const;
    bool IsVisible() const;

    const Style& GetStyle() const;
    void SetStyle(const Style& style);

    static QSize toolButtonIconSize;

    static const char* readOnlyFieldName;

protected:
    friend class ComponentStructureWrapper;
    friend class ReflectedPropertyItem;

    virtual Any GetMultipleValue() const = 0;
    virtual bool IsValidValueToSet(const Any& newValue, const Any& currentValue) const = 0;
    virtual ControlProxy* CreateEditorWidget(QWidget* parent, const Reflection& model, DataWrappersProcessor* wrappersProcessor) = 0;

    Any GetValue() const;
    void SetValue(const Any& value);

    std::shared_ptr<ModifyExtension> GetModifyInterface();

    void AddPropertyNode(const std::shared_ptr<PropertyNode>& node, const FastName& id);
    void RemovePropertyNode(const std::shared_ptr<PropertyNode>& node);
    void RemovePropertyNodes();

    ControlProxy* editorWidget = nullptr;
    Vector<std::shared_ptr<PropertyNode>> nodes;
    QWidget* realWidget = nullptr;

    ContextAccessor* GetAccessor() const;
    UI* GetUI() const;
    const WindowKey& GetWindowKey() const;
    DataWrappersProcessor* GetDataProcessor() const;

private:
    void UpdateEditorGeometry(const QRect& geometry) const;

    void CreateButtons(Widget* widget, const M::CommandProducerHolder* holder);
    void CallButtonAction(std::shared_ptr<M::CommandProducer> producer);

    ReflectedPropertyModel* model = nullptr;
    BaseComponentValue* thisValue = nullptr;
    bool isEditorEvent = false;
    Style style;
    FastName itemID;

    QtConnections connections;
    class ButtonModel;
    Widget* buttonsLayout = nullptr;
    DAVA::Vector<std::unique_ptr<ButtonModel>> buttonModels;

    DAVA_VIRTUAL_REFLECTION(BaseComponentValue);
};
}
}
