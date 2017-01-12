#pragma once

#include "TArc/Controls/PropertyPanel/Private/ChildCreator.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"

#include "Base/BaseTypes.h"
#include "Base/Any.h"

#include <QAbstractItemModel>

namespace DAVA
{
namespace TArc
{
class ReflectedPropertyItem;

class ReflectedPropertyModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ReflectedPropertyModel();
    ~ReflectedPropertyModel();

    //////////////////////////////////////
    //       QAbstractItemModel         //
    //////////////////////////////////////

    int rowCount(const QModelIndex& parent /* = QModelIndex() */) const override;
    int columnCount(const QModelIndex& parent /* = QModelIndex() */) const override;
    QVariant data(const QModelIndex& index, int role /* = Qt::DisplayRole */) const override;
    bool setData(const QModelIndex& index, const QVariant& value, int role /* = Qt::EditRole */) override;
    QVariant headerData(int section, Qt::Orientation orientation, int role /* = Qt::DisplayRole */) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    //////////////////////////////////////
    //       QAbstractItemModel         //
    //////////////////////////////////////

    void Update();
    void SetObjects(Vector<Reflection> objects);

    void RegisterExtension(const std::shared_ptr<ExtensionChain>& extension);
    void UnregisterExtension(const std::shared_ptr<ExtensionChain>& extension);

    BaseComponentValue* GetComponentValue(const QModelIndex& index) const;

private:
    friend class BaseComponentValue;
    void ChildAdded(std::shared_ptr<const PropertyNode> parent, std::shared_ptr<PropertyNode> node, int32 childPosition);
    void ChildRemoved(std::shared_ptr<PropertyNode> node);

    ReflectedPropertyItem* MapItem(const QModelIndex& item) const;
    QModelIndex MapItem(ReflectedPropertyItem* item) const;

    void Update(ReflectedPropertyItem* item);

    template <typename T>
    std::shared_ptr<T> GetExtensionChain() const;

private:
    std::unique_ptr<ReflectedPropertyItem> rootItem;
    UnorderedMap<std::shared_ptr<const PropertyNode>, ReflectedPropertyItem*> nodeToItem;

    ChildCreator childCreator;
    Map<const Type*, std::shared_ptr<ExtensionChain>> extensions;

    DataWrappersProcessor wrappersProcessor;
};

template <typename Dst, typename Src>
std::shared_ptr<Dst> PolymorphCast(std::shared_ptr<Src> ptr)
{
    DVASSERT(dynamic_cast<Dst*>(ptr.get()) != nullptr);
    return std::static_pointer_cast<Dst>(ptr);
}

template <typename T>
std::shared_ptr<T> ReflectedPropertyModel::GetExtensionChain() const
{
    static_assert(!std::is_same<T, ChildCreatorExtension>::value, "There is no reason to request ChildCreatorExtension");
    static_assert(std::is_base_of<ExtensionChain, T>::value, "ExtensionChain should be base of extension");
    const Type* extType = Type::Instance<T>();
    auto iter = extensions.find(extType);
    if (iter == extensions.end())
    {
        DVASSERT(false);
        return nullptr;
    }

    return PolymorphCast<T>(iter->second);
}

} // namespace TArc
} // namespace DAVA