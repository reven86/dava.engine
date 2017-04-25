#pragma once

#include <Reflection/Reflection.h>
#include <Functional/Signal.h>
#include <Base/FastName.h>
#include <Base/UnordererMap.h>
#include <Base/UnordererSet.h>
#include <Base/Vector.h>

#include <memory>

namespace DAVA
{
namespace TArc
{
struct PropertyNode;

struct FavoriteItemValue
{
    DAVA_REFLECTION(FavoriteItemValue);
};

class FavoritesController
{
public:
    FavoritesController();

    void Save(PropertiesItem& propertiesRoot) const;
    void Load(const PropertiesItem& propertiesRoot);

    void SetModelRoot(std::shared_ptr<PropertyNode> root);
    void Reset();

    void OnChildAdded(const std::shared_ptr<PropertyNode>& parent, const std::shared_ptr<PropertyNode>& child);
    void OnChildRemoved(const std::shared_ptr<PropertyNode>& child);

    void AddFavorite(const std::shared_ptr<PropertyNode>& node);
    void RemoveFavorite(const std::shared_ptr<PropertyNode>& node);

    void ClearFavorites();

    Signal<const std::shared_ptr<PropertyNode>& /*parent*/, const std::shared_ptr<PropertyNode>&, int32 /*sortKey*/, bool /*root*/> favoritedCreated;
    Signal<const std::shared_ptr<PropertyNode>&> favoriteDeleted;

private:
    void AddFavorite(Vector<FastName>&& favoritePath);
    void RemoveFavorite(const Vector<FastName>& favoritePath);
    int32 MatchPath(const std::shared_ptr<PropertyNode>& node);

    void AddItemRecursive(const std::shared_ptr<PropertyNode>& parent);
    void RemoveItemRecursive(const std::shared_ptr<PropertyNode>& parent);
    void BuildPathToNode(std::shared_ptr<PropertyNode> node, Vector<FastName>& path);

private:
    std::shared_ptr<PropertyNode> selfRoot;
    std::shared_ptr<PropertyNode> favoriteRoot;
    UnorderedMap<std::shared_ptr<PropertyNode>, std::shared_ptr<PropertyNode>> childToParent;
    UnorderedMap<std::shared_ptr<PropertyNode>, UnorderedSet<std::shared_ptr<PropertyNode>>> parentToChild;
    UnorderedMap<FastName, UnorderedSet<std::shared_ptr<PropertyNode>>> idToNodes;
    UnorderedMap<std::shared_ptr<PropertyNode>, int32> favoritedRoots;
    UnorderedSet<std::shared_ptr<PropertyNode>> subFavorited;

    Vector<Vector<FastName>> favoritedPathes;
    FavoriteItemValue favoriteRootItem;
};

} // namespace TArc
} // namespace DAVA