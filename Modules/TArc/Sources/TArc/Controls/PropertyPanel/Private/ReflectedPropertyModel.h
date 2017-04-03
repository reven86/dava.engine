#pragma once

#include "TArc/Controls/PropertyPanel/Private/ChildCreator.h"
#include "TArc/Controls/PropertyPanel/Private/ReflectionPathTree.h"
#include "TArc/DataProcessing/DataWrappersProcessor.h"
#include "TArc/DataProcessing/PropertiesHolder.h"
#include "TArc/WindowSubSystem/UI.h"

#include "Base/BaseTypes.h"
#include "Base/Any.h"

#include <QAbstractItemModel>

namespace DAVA
{
namespace TArc
{
class ReflectedPropertyItem;
class ContextAccessor;
class OperationInvoker;
class UI;

class ReflectedPropertyModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    ReflectedPropertyModel(WindowKey wndKey, ContextAccessor* accessor, OperationInvoker* invoker, UI* ui);
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
    void UpdateFast();
    void SetObjects(Vector<Reflection> objects);

    void RegisterExtension(const std::shared_ptr<ExtensionChain>& extension);
    void UnregisterExtension(const std::shared_ptr<ExtensionChain>& extension);

    BaseComponentValue* GetComponentValue(const QModelIndex& index) const;
    void SyncWrapper()
    {
        wrappersProcessor.Sync();
    }

    void SetExpanded(bool expanded, const QModelIndex& index);
    QModelIndexList GetExpandedList() const;
    QModelIndexList GetExpandedChildren(const QModelIndex& index) const;

    void SaveState(PropertiesItem& propertyRoot) const;
    void LoadState(const PropertiesItem& propertyRoot);

    void HideEditors();

    bool IsFavorite(const QModelIndex& index) const;
    bool IsInFavoriteHierarchy(const QModelIndex& index) const;
    void AddFavorite(const QModelIndex& index);
    void RemoveFavorite(const QModelIndex& index);

    bool IsFavoriteOnly() const;
    void SetFavoriteOnly(bool isFavoriteOnly);

private:
    friend class BaseComponentValue;
    void ChildAdded(std::shared_ptr<const PropertyNode> parent, std::shared_ptr<PropertyNode> node, int32 childPosition);
    void ChildRemoved(std::shared_ptr<PropertyNode> node);

    ReflectedPropertyItem* MapItem(const QModelIndex& item) const;
    QModelIndex MapItem(ReflectedPropertyItem* item) const;

    ReflectedPropertyItem* GetSmartRoot() const;

    void Update(ReflectedPropertyItem* item);
    void UpdateFastImpl(ReflectedPropertyItem* item);
    void HideEditor(ReflectedPropertyItem* item);

    template <typename T>
    std::shared_ptr<T> GetExtensionChain() const;
    ReflectedPropertyItem* LookUpItem(const std::shared_ptr<PropertyNode>& node, const Vector<std::unique_ptr<ReflectedPropertyItem>>& children);

    DataWrappersProcessor* GetWrappersProcessor(const std::shared_ptr<PropertyNode>& node);
    void GetExpandedListImpl(QModelIndexList& list, ReflectedPropertyItem* item) const;

    void RefreshFavoritesRoot();
    void RefreshFavorites(ReflectedPropertyItem* item, uint32 level, bool insertSessionIsOpen, const Set<size_t>& candidates);
    ReflectedPropertyItem* CreateDeepCopy(ReflectedPropertyItem* itemToCopy, ReflectedPropertyItem* copyParent, size_t positionInParent);

private:
    std::unique_ptr<ReflectedPropertyItem> rootItem;
    ReflectedPropertyItem* favoritesRoot = nullptr;
    UnorderedMap<std::shared_ptr<const PropertyNode>, ReflectedPropertyItem*> nodeToItem;
    UnorderedMap<ReflectedPropertyItem*, ReflectedPropertyItem*> favoriteToItem;
    UnorderedMap<ReflectedPropertyItem*, ReflectedPropertyItem*> itemToFavorite;

    ChildCreator childCreator;
    Map<const Type*, std::shared_ptr<ExtensionChain>> extensions;

    DataWrappersProcessor wrappersProcessor;
    DataWrappersProcessor fastWrappersProcessor;
    ReflectionPathTree expandedItems;
    Vector<Vector<FastName>> favoritedPathes;

    WindowKey wndKey;
    ContextAccessor* accessor = nullptr;
    OperationInvoker* invoker = nullptr;
    UI* ui = nullptr;

    bool showFavoriteOnly = false;
    class InsertGuard;
    class RemoveGuard;
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