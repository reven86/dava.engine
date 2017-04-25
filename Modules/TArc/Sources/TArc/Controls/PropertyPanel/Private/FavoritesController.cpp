#include "TArc/Controls/PropertyPanel/Private/FavoritesController.h"

#include <Debug/DVAssert.h>
#include <Base/BaseTypes.h>

#include <algorithm>

namespace DAVA
{
namespace TArc
{
DAVA_REFLECTION_IMPL(FavoriteItemValue)
{
    ReflectionRegistrator<FavoriteItemValue>::Begin()
    .End();
}

const char* favoritedItemsKey = "favoritedItems";
const char* favoritedCountKey = "favoritesCount";
const char* favoritedElementKey = "favoritesElement_%u";

FavoritesController::FavoritesController()
{
    favoriteRoot.reset(new PropertyNode());
    favoriteRoot->field.ref = Reflection::Create(&favoriteRootItem);
    favoriteRoot->field.key = FastName("Favorites");
    favoriteRoot->propertyType = PropertyNode::FavoritesProperty;
    favoriteRoot->cachedValue = favoriteRoot->field.ref.GetValue();
    favoriteRoot->sortKey = PropertyNode::FavoritesRootSortKey;
}

void FavoritesController::Save(PropertiesItem& propertiesRoot) const
{
    PropertiesItem favoritesListSettings = propertiesRoot.CreateSubHolder(favoritedItemsKey);
    favoritesListSettings.Set(favoritedCountKey, static_cast<int32>(favoritedPathes.size()));
    for (size_t i = 0; i < favoritedPathes.size(); ++i)
    {
        favoritesListSettings.Set(Format(favoritedElementKey, static_cast<uint32>(i)), favoritedPathes[i]);
    }
}

void FavoritesController::Load(const PropertiesItem& propertiesRoot)
{
    PropertiesItem favoritesListSettings = propertiesRoot.CreateSubHolder(favoritedItemsKey);
    uint32 count = static_cast<uint32>(favoritesListSettings.Get(favoritedCountKey, int32(0)));
    favoritedPathes.reserve(count);

    for (uint32 i = 0; i < count; ++i)
    {
        favoritedPathes.push_back(favoritesListSettings.Get<Vector<FastName>>(Format(favoritedElementKey, static_cast<uint32>(i))));
    }
}

void FavoritesController::SetModelRoot(std::shared_ptr<PropertyNode> root)
{
    selfRoot = root;
    favoriteRoot->parent = root;

    if (favoritedPathes.empty() == false)
    {
        favoritedCreated.Emit(selfRoot, favoriteRoot, favoriteRoot->sortKey, true);
    }
}

void FavoritesController::Reset()
{
    selfRoot.reset();
    favoritedRoots.clear();
    childToParent.clear();
    parentToChild.clear();
    idToNodes.clear();
    subFavorited.clear();
}

void FavoritesController::OnChildAdded(const std::shared_ptr<PropertyNode>& parent, const std::shared_ptr<PropertyNode>& child)
{
    DVASSERT(selfRoot != nullptr);

    childToParent.emplace(child, parent);
    parentToChild[parent].emplace(child);
    idToNodes[FastName(child->BuildID())].emplace(child);

    if (subFavorited.count(parent) > 0)
    {
        favoritedCreated.Emit(parent, child, child->sortKey, false);
        subFavorited.emplace(child);
    }

    int32 matchedIndex = MatchPath(child);
    if (matchedIndex != -1)
    {
        favoritedCreated.Emit(favoriteRoot, child, matchedIndex, true);
        subFavorited.emplace(child);
        favoritedRoots.emplace(child, matchedIndex);
    }
}

void FavoritesController::OnChildRemoved(const std::shared_ptr<PropertyNode>& child)
{
    DVASSERT(selfRoot != nullptr);

    auto subFavoriteIter = subFavorited.find(child);
    if (subFavoriteIter != subFavorited.end())
    {
        favoriteDeleted.Emit(child);
        subFavorited.erase(child);
    }

    auto parentIter = childToParent.find(child);
    DVASSERT(parentIter != childToParent.end());
    auto iter = parentToChild.find(parentIter->second);
    DVASSERT(iter != parentToChild.end());
    iter->second.erase(child);
    childToParent.erase(parentIter);
    idToNodes[FastName(child->BuildID())].erase(child);
    favoritedRoots.erase(child);
}

void FavoritesController::AddFavorite(const std::shared_ptr<PropertyNode>& node)
{
    Vector<FastName> path;
    BuildPathToNode(node, path);
    AddFavorite(std::move(path));
}

void FavoritesController::RemoveFavorite(const std::shared_ptr<PropertyNode>& node)
{
    Vector<FastName> path;
    BuildPathToNode(node, path);
    RemoveFavorite(std::move(path));
}

void FavoritesController::AddFavorite(Vector<FastName>&& favoritePath)
{
#if defined(__DAVAENGINE_DEBUG__)
    DVASSERT(std::count(favoritedPathes.begin(), favoritedPathes.end(), favoritePath) == 0);
#endif

    Set<Vector<FastName>> pathesToRemove;
    for (const Vector<FastName>& path : favoritedPathes)
    {
        if (std::search(path.begin(), path.end(), favoritePath.begin(), favoritePath.end()) != path.end())
        {
            pathesToRemove.emplace(path);
        }
    }

    for (const Vector<FastName>& path : pathesToRemove)
    {
        RemoveFavorite(path);
    }

    DVASSERT(favoritePath.size() > 1);
    if (favoritedPathes.empty())
    {
        favoritedCreated.Emit(selfRoot, favoriteRoot, favoriteRoot->sortKey, true);
    }
    favoritedPathes.emplace_back(std::move(favoritePath));

    if (selfRoot == nullptr)
    {
        return;
    }

    const Vector<FastName>& lastPath = favoritedPathes.back();
    auto topLevelItem = idToNodes.find(lastPath.back());
    if (topLevelItem == idToNodes.end())
    {
        return;
    }

    UnorderedSet<std::shared_ptr<PropertyNode>> topLevelNodes = topLevelItem->second;
    for (const auto& node : topLevelNodes)
    {
        int32 pathPartIndex = static_cast<int32>(lastPath.size()) - 2;
        DVASSERT(pathPartIndex >= 0);
        auto currentNodeIter = childToParent.find(node);
        bool matched = true;

        while (true)
        {
            bool indexOutOfBound = pathPartIndex < 0;
            bool iterInvalid = currentNodeIter == childToParent.end();
            if (indexOutOfBound == true || iterInvalid == true)
            {
                if (indexOutOfBound == false || iterInvalid == false)
                {
                    matched = false;
                }
                break;
            }

            if (strcmp(lastPath[pathPartIndex].c_str(), currentNodeIter->second->BuildID().c_str()) != 0)
            {
                matched = false;
                break;
            }

            pathPartIndex--;
            currentNodeIter = childToParent.find(currentNodeIter->second);
        }

        if (matched == true)
        {
            int32 pathIndex = static_cast<int32>(favoritedPathes.size()) - 1;
            favoritedCreated.Emit(favoriteRoot, node, pathIndex, true);
            favoritedRoots.emplace(node, pathIndex);
            subFavorited.emplace(node);
            AddItemRecursive(node);
        }
    }
}

void FavoritesController::RemoveFavorite(const Vector<FastName>& favoritePath)
{
    DVASSERT(favoritePath.size() > 1);
    auto iter = favoritedPathes.begin();
    size_t currentIndex = 0;
    int32 pathIndex = -1;
    while (iter != favoritedPathes.end())
    {
        if (*iter == favoritePath)
        {
            iter = favoritedPathes.erase(iter);
            pathIndex = static_cast<int32>(currentIndex);
            break;
        }

        ++iter;
        ++currentIndex;
    }
    DVASSERT(pathIndex != -1);

    if (selfRoot == nullptr)
    {
        return;
    }

    auto fvRootIter = favoritedRoots.begin();
    while (fvRootIter != favoritedRoots.end())
    {
        if (fvRootIter->second == currentIndex)
        {
            subFavorited.erase(fvRootIter->first);
            RemoveItemRecursive(fvRootIter->first);
            favoriteDeleted.Emit(fvRootIter->first);
            fvRootIter = favoritedRoots.erase(fvRootIter);
            continue;
        }
        else if (fvRootIter->second > pathIndex)
        {
            --fvRootIter->second;
        }

        fvRootIter++;
    }

    if (favoritedPathes.empty() == true)
    {
        favoriteDeleted.Emit(favoriteRoot);
    }
}

void FavoritesController::ClearFavorites()
{
    DVASSERT(favoritedPathes.empty() == false);
    for (auto& iter : favoritedRoots)
    {
        RemoveItemRecursive(iter.first);
        favoriteDeleted.Emit(iter.first);
    }

    favoriteDeleted.Emit(favoriteRoot);

    favoritedRoots.clear();
    favoritedPathes.clear();
}

int32 FavoritesController::MatchPath(const std::shared_ptr<PropertyNode>& node)
{
    for (size_t i = 0; i < favoritedPathes.size(); ++i)
    {
        const Vector<FastName>& path = favoritedPathes[i];
        DVASSERT(path.size() > 1);

        std::shared_ptr<PropertyNode> currentNode = node;
        int32 currentPathIndex = static_cast<int32>(path.size() - 1);

        bool pathMatched = true;
        while (true)
        {
            if (currentPathIndex < 0 || currentNode == nullptr)
            {
                // if we reached end of one of path but not both of them at same time
                // it means that lengths of favorite path and child path in hierarchy are different
                if (currentPathIndex >= 0 || currentNode != nullptr)
                {
                    pathMatched = false;
                }
                break;
            }

            if (strcmp(path[currentPathIndex].c_str(), currentNode->BuildID().c_str()) != 0)
            {
                pathMatched = false;
                break;
            }

            auto iter = childToParent.find(currentNode);
            if (iter != childToParent.end())
            {
                currentNode = iter->second;
            }
            else
            {
                currentNode = nullptr;
            }
            currentPathIndex--;
        }

        if (pathMatched == true)
        {
            return static_cast<int32>(i);
        }
    }

    return -1;
}

void FavoritesController::AddItemRecursive(const std::shared_ptr<PropertyNode>& parent)
{
    auto iter = parentToChild.find(parent);
    if (iter == parentToChild.end())
    {
        return;
    }

    for (const std::shared_ptr<PropertyNode>& node : iter->second)
    {
        subFavorited.emplace(node);
        favoritedCreated.Emit(parent, node, node->sortKey, false);
        AddItemRecursive(node);
    }
}

void FavoritesController::RemoveItemRecursive(const std::shared_ptr<PropertyNode>& parent)
{
    auto iter = parentToChild.find(parent);
    if (iter == parentToChild.end())
    {
        return;
    }

    for (const std::shared_ptr<PropertyNode>& node : iter->second)
    {
        subFavorited.erase(node);
        RemoveItemRecursive(node);
        favoriteDeleted.Emit(node);
    }
}

void FavoritesController::BuildPathToNode(std::shared_ptr<PropertyNode> node, Vector<FastName>& path)
{
    path.reserve(8);

    while (node != nullptr)
    {
        path.push_back(FastName(node->BuildID()));
        auto iter = childToParent.find(node);
        if (iter != childToParent.end())
        {
            node = iter->second;
        }
        else
        {
            node = nullptr;
        }
    }
    std::reverse(path.begin(), path.end());
}

} // namespace TArc
} // namespace DAVA
