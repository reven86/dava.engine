#pragma once

#include "PropertyModelExtensions.h"

#include "Reflection/Reflection.h"
#include "Base/BaseTypes.h"

#include <QString>

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
class QtReflected;
class DataContext;
class DataWrappersProcessor;
struct PropertyNode;

class BaseComponentValue : public ReflectionBase
{
public:
    BaseComponentValue();

    void Init(ReflectedPropertyModel* model);

    virtual QWidget* AcquireEditorWidget(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) = 0;
    virtual void ReleaseEditorWidget(QWidget* editor, const QModelIndex& index) = 0;
    virtual void StaticEditorPaint(QStyle* style, QPainter* painter, const QStyleOptionViewItem& options) = 0;

    QString GetPropertyName() const;

    int32 GetPropertiesNodeCount() const;
    std::shared_ptr<const PropertyNode> GetPropertyNode(int32 index) const;

protected:
    friend class ReflectedPropertyItem;

    void AddPropertyNode(const std::shared_ptr<PropertyNode>& node);
    void RemovePropertyNode(const std::shared_ptr<PropertyNode>& node);
    void RemovePropertyNodes();

    Vector<std::shared_ptr<PropertyNode>> nodes;
    std::shared_ptr<ModifyExtension> GetModifyInterface();
    DataWrappersProcessor* GetWrappersProcessor();
    Reflection GetReflection();

private:
    ReflectedPropertyModel* model = nullptr;
    BaseComponentValue* thisValue = nullptr;

    DAVA_VIRTUAL_REFLECTION(BaseComponentValue);
};
}
}