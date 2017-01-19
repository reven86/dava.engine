#pragma once

#include "PropertyModelExtensions.h"

#include "Reflection/Reflection.h"
#include "Base/BaseTypes.h"

#include <QString>
#include <QRect>

class QWidget;
class QStyleOptionViewItem;
class QModelIndex;
class QStyle;
class QPainter;

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

class StaticEditorProxy final
{
public:
    StaticEditorProxy(BaseComponentValue* valueComponent, const StaticEditorDrawer* drawer);

    uint32 GetHeight(QStyle* style, const QStyleOptionViewItem& options) const;
    void Draw(QStyle* style, QPainter* painter, const QStyleOptionViewItem& options) const;

private:
    BaseComponentValue* value = nullptr;
    const StaticEditorDrawer* drawer = nullptr;
};

class InteractiveEditorProxy final
{
public:
    InteractiveEditorProxy(BaseComponentValue* valueComponent);

    QWidget* AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option);
    void ReleaseEditorWidget(QWidget* editor);
    QRect GetEditorRect(QStyle* style, const QStyleOptionViewItem& option);

    void CommitData();

private:
    BaseComponentValue* value = nullptr;
};

class BaseComponentValue : public ReflectionBase
{
public:
    BaseComponentValue();

    void Init(ReflectedPropertyModel* model);

    StaticEditorProxy GetStaticEditor();
    InteractiveEditorProxy GetInteractiveEditor();

    QString GetPropertyName() const;
    int32 GetPropertiesNodeCount() const;
    std::shared_ptr<const PropertyNode> GetPropertyNode(int32 index) const;

protected:
    virtual Any GetValue() const = 0;
    virtual bool IsValidValueToSet(const Any& value) const = 0;
    void SetValue(const Any& value);
    virtual const StaticEditorDrawer* GetStaticEditorDrawer() const = 0;
    virtual QWidget* AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option) = 0;
    virtual void ReleaseEditorWidget(QWidget* editor) = 0;

    void AddPropertyNode(const std::shared_ptr<PropertyNode>& node);
    void RemovePropertyNode(const std::shared_ptr<PropertyNode>& node);
    void RemovePropertyNodes();

    Vector<std::shared_ptr<PropertyNode>> nodes;
    DataWrappersProcessor* GetWrappersProcessor();
    Reflection GetReflection();

private:
    std::shared_ptr<ModifyExtension> GetModifyInterface();

    void UpdateCachedValue();
    void ClearCachedValue();
    void CommitData();

private:
    friend class ReflectedPropertyItem;
    friend class InteractiveEditorProxy;
    friend class StaticEditorProxy;

    ReflectedPropertyModel* model = nullptr;
    BaseComponentValue* thisValue = nullptr;
    Any cachedValue;

    DAVA_VIRTUAL_REFLECTION(BaseComponentValue);
};
}
}