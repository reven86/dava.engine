#pragma once

#include "TArc/Controls/PropertyPanel/Private/ChildCreator.hpp"

#include "Base/BaseTypes.h"
#include "Base/Any.h"

#include <QAbstractItemModel>

namespace DAVA
{
namespace TArc
{
class ReflectedPropertyModel;

class ReflectedPropertyItem
{
public:
    ReflectedPropertyItem(ReflectedPropertyModel & model);

    Any GetItemName() const;
    Any GetValue() const;

    const Vector<std::shared_ptr<const PropertyNode>>& GetPropertyNodes() const { return nodes; }

private:
    //![don't call it]
    ReflectedPropertyItem(const ReflectedPropertyItem& other) : model(other.model) {}
    ReflectedPropertyItem(ReflectedPropertyItem&& other) : model(other.model) {}
    ReflectedPropertyItem& operator=(const ReflectedPropertyItem& other) { return *this; }
    ReflectedPropertyItem& operator=(ReflectedPropertyItem&& other) { return *this; }
    //![don't call it]

    friend class ReflectedPropertyModel;
    ReflectedPropertyItem * CreateChild();

    size_t GetChildCount() const;
    ReflectedPropertyItem * GetChild(size_t index) const;
    void RemoveChild(size_t index);
    void RemoveChildren();

    void AddObject(const std::shared_ptr<const PropertyNode>& node);
    void RemoveObject(const std::shared_ptr<const PropertyNode>& node);
    void RemoveObjects();
    bool HasObjects() const;

    Any EvalValue() const;
    void SetValue(Any&& newValue);

private:
    ReflectedPropertyModel& model;
    Vector<std::unique_ptr<ReflectedPropertyItem>> children;
    Vector<std::shared_ptr<const PropertyNode>> nodes;
    Any itemValue;
};

class ReflectedPropertyModel: public QAbstractItemModel
{
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

    QModelIndex index(int row, int column, const QModelIndex& parent) const override;
    QModelIndex parent(const QModelIndex& index) const override;

    //////////////////////////////////////
    //       QAbstractItemModel         //
    //////////////////////////////////////

    void Update();
    void SetObjects(const std::vector<Reflection>& objects);

    void RegisterExtension(const std::shared_ptr<ExtensionChain>& extension);
    void UnregisterExtension(const std::shared_ptr<ExtensionChain>& extension);

private:
    void ChildAdded(const std::shared_ptr<const PropertyNode>& parent, const std::shared_ptr<const PropertyNode>& node, size_t childPosition);
    void ChildRemoved(const std::shared_ptr<const PropertyNode>& node);

    const ReflectedPropertyItem* GetEffectiveParent(const AbstractItem* modelParent) const;
    ReflectedPropertyItem* GetEffectiveParent(AbstractItem* modelParent) const;

    AbstractListModel::ItemIndex getModelParent(const ReflectedPropertyItem* effectiveParent) const;

    void Update(ReflectedPropertyItem* item);

    template<typename T>
    std::shared_ptr<T> getExtensionChain() const;

private:
    friend class ReflectedPropertyItem;
    Any GetDataImpl(const ReflectedPropertyItem * item, int column, size_t roleId) const;
    bool SetDataImpl(ReflectedPropertyItem * item, int column, size_t roleId, const Any& data);

    std::unique_ptr<ReflectedPropertyItem> rootItem;
    std::unordered_map<std::shared_ptr<const PropertyNode>, ReflectedPropertyItem*> nodeToItem;

    ChildCreator childCreator;
    std::map<const Type*, std::shared_ptr<ExtensionChain>> extensions;
};

template<typename Dst, typename Src>
std::shared_ptr<Dst> PolymorphCast(std::shared_ptr<Src> ptr)
{
    assert(dynamic_cast<Dst*>(ptr.get()) != nullptr);
    return std::static_pointer_cast<Dst>(ptr);
}

template<typename T>
std::shared_ptr<T> ReflectedPropertyModel::GetExtensionChain() const
{
    static_assert(!std::is_same<T, ChildCreatorExtension>::value, "There is no reason to request ChildCreatorExtension");
    static_assert(std::is_base_of<ExtensionChain, T>::value, "ExtensionChain should be base of extension");
    TypeId typeId = TypeId::getType<T>();
    auto iter = extensions.find(typeId);
    if (iter == extensions.end())
    {
        assert(false);
        return nullptr;
    }

    return PolymorphCast<T>(iter->second);
}

} // namespace TArc

} // namespace DAVA