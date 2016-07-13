#include "Scene/System/SelectionSystem.h"
#include "Scene/System/ModifSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene/SceneSignals.h"

ENUM_DECLARE(SelectionSystemDrawMode)
{
    ENUM_ADD(SS_DRAW_SHAPE);
    ENUM_ADD(SS_DRAW_CORNERS);
    ENUM_ADD(SS_DRAW_BOX);
    ENUM_ADD(SS_DRAW_NO_DEEP_TEST);
}

SceneSelectionSystem::SceneSelectionSystem(SceneEditor2* editor)
    : DAVA::SceneSystem(editor)
    , collisionSystem(editor->collisionSystem)
    , hoodSystem(editor->hoodSystem)
    , modificationSystem(editor->modifSystem)
{
    DVASSERT(collisionSystem != nullptr);
    DVASSERT(hoodSystem != nullptr);
    DVASSERT(modificationSystem != nullptr);
    GetScene()->GetEventSystem()->RegisterSystemForEvent(this, DAVA::EventSystem::SWITCH_CHANGED);
    GetScene()->GetEventSystem()->RegisterSystemForEvent(this, DAVA::EventSystem::LOCAL_TRANSFORM_CHANGED);
    GetScene()->GetEventSystem()->RegisterSystemForEvent(this, DAVA::EventSystem::TRANSFORM_PARENT_CHANGED);
}

SceneSelectionSystem::~SceneSelectionSystem()
{
    if (GetScene())
    {
        GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, DAVA::EventSystem::SWITCH_CHANGED);
    }
}

void SceneSelectionSystem::ImmediateEvent(DAVA::Component* component, DAVA::uint32 event)
{
    switch (event)
    {
    case DAVA::EventSystem::SWITCH_CHANGED:
    case DAVA::EventSystem::LOCAL_TRANSFORM_CHANGED:
    case DAVA::EventSystem::TRANSFORM_PARENT_CHANGED:
    {
        if (currentSelection.ContainsObject(component->GetEntity()))
        {
            invalidSelectionBoxes = true;
        }
        break;
    }
    default:
        break;
    }
}

void SceneSelectionSystem::UpdateGroupSelectionMode()
{
    const auto& keyboard = DAVA::InputSystem::Instance()->GetKeyboard();

    bool addSelection = keyboard.IsKeyPressed(DAVA::Key::LCTRL) || keyboard.IsKeyPressed(DAVA::Key::RCTRL);
    bool excludeSelection = keyboard.IsKeyPressed(DAVA::Key::LALT) || keyboard.IsKeyPressed(DAVA::Key::RALT);

    if (addSelection)
    {
        groupSelectionMode = GroupSelectionMode::Add;
    }
    else if (excludeSelection)
    {
        groupSelectionMode = GroupSelectionMode::Remove;
    }
    else
    {
        groupSelectionMode = GroupSelectionMode::Replace;
    }
}

namespace SceneSelectionSystem_Details
{
bool FindIfParentWasAdded(DAVA::Entity* entity, const DAVA::List<DAVA::Entity*>& container, DAVA::Scene* scene)
{
    DAVA::Entity* parent = entity->GetParent();
    if (parent == scene || parent == nullptr)
    {
        return false;
    }

    auto found = std::find(container.begin(), container.end(), parent);
    if (found != container.end())
    {
        return true;
    }

    return FindIfParentWasAdded(parent, container, scene);
}
}

void SceneSelectionSystem::Process(DAVA::float32 timeElapsed)
{
    ForceEmitSignals();
    if (IsLocked())
    {
        return;
    }

    if (!entitiesForSelection.empty())
    {
        Clear();
        for (auto& entity : entitiesForSelection)
        {
            if (false == SceneSelectionSystem_Details::FindIfParentWasAdded(entity, entitiesForSelection, GetScene()))
            {
                AddObjectToSelection(entity);
            }
        }
        entitiesForSelection.clear();
    }

    // if boxes are invalid we should request them from collision system
    // and store them in the currentSelection objects
    if (invalidSelectionBoxes)
    {
        for (auto& item : currentSelection.GetMutableContent())
        {
            item.SetBoundingBox(GetUntransformedBoundingBox(item.GetContainedObject()));
        }
        invalidSelectionBoxes = false;
    }

    UpdateGroupSelectionMode();
    UpdateHoodPos();
}

void SceneSelectionSystem::ForceEmitSignals()
{
    if (selectionHasChanges)
    {
        SceneSignals::Instance()->EmitSelectionChanged((SceneEditor2*)GetScene(), &currentSelection, &recentlySelectedEntities);
        selectionHasChanges = false;
        recentlySelectedEntities.Clear();
    }
}

void SceneSelectionSystem::ProcessSelectedGroup(const SelectableGroup::CollectionType& allObjects)
{
    SelectableGroup::CollectionType collisionObjects;
    collisionObjects.reserve(allObjects.size());

    for (const auto& item : allObjects)
    {
        bool wasAdded = false;

        auto entity = item.AsEntity();
        if (entity == nullptr)
        {
            collisionObjects.emplace_back(item.GetContainedObject());
            wasAdded = true;
        }
        else if (componentMaskForSelection & entity->GetAvailableComponentFlags())
        {
            collisionObjects.emplace_back(GetSelectableEntity(entity));
            wasAdded = true;
        }

        if (wasAdded)
        {
            collisionObjects.back().SetBoundingBox(GetUntransformedBoundingBox(collisionObjects.back().GetContainedObject()));
        }
    }

    Selectable::Object* firstEntity = collisionObjects.empty() ? nullptr : collisionObjects.front().GetContainedObject();
    Selectable::Object* nextEntity = firstEntity;

    // sequent selection?
    if (SettingsManager::GetValue(Settings::Scene_SelectionSequent).AsBool() && (currentSelection.GetSize() <= 1))
    {
        // find first after currently selected items
        for (size_t i = 0, e = collisionObjects.size(); i < e; i++)
        {
            if (currentSelection.ContainsObject(collisionObjects[i].GetContainedObject()))
            {
                if ((i + 1) < e)
                {
                    nextEntity = collisionObjects[i + 1].GetContainedObject();
                    break;
                }
            }
        }
    }

    const auto& keyboard = DAVA::InputSystem::Instance()->GetKeyboard();

    bool addSelection = keyboard.IsKeyPressed(DAVA::Key::LCTRL) || keyboard.IsKeyPressed(DAVA::Key::RCTRL);
    bool excludeSelection = keyboard.IsKeyPressed(DAVA::Key::LALT) || keyboard.IsKeyPressed(DAVA::Key::RALT);

    if (addSelection && (firstEntity != nullptr))
    {
        AddObjectToSelection(firstEntity);
    }
    else if (excludeSelection && (firstEntity != nullptr))
    {
        ExcludeEntityFromSelection(firstEntity);
    }
    else
    {
        bool selectOnClick = SettingsManager::GetValue(Settings::Scene_SelectionOnClick).AsBool();
        // if new selection is NULL or is one of already selected items
        // we should change current selection only on phase end
        // TODO : review and remove?
        bool containsAlreadySelectedObjects = false;
        for (const auto& obj : collisionObjects)
        {
            if (currentSelection.ContainsObject(obj.GetContainedObject()))
            {
                containsAlreadySelectedObjects = true;
                break;
            }
        }

        if (selectOnClick || (nextEntity == nullptr) || containsAlreadySelectedObjects)
        {
            applyOnPhaseEnd = true;
            objectsToSelect.Clear();
            if (nextEntity != nullptr)
            {
                objectsToSelect.Add(nextEntity, GetUntransformedBoundingBox(nextEntity));
            }
        }
        else
        {
            SelectableGroup newSelection;
            newSelection.Add(nextEntity, GetUntransformedBoundingBox(nextEntity));
            SetSelection(newSelection);
        }
    }
}

void SceneSelectionSystem::PerformSelectionAtPoint(const DAVA::Vector2& point)
{
    DAVA::Vector3 traceFrom;
    DAVA::Vector3 traceTo;
    SceneCameraSystem* cameraSystem = ((SceneEditor2*)GetScene())->cameraSystem;
    if (cameraSystem->GetCurCamera() != nullptr)
    {
        cameraSystem->GetRayTo2dPoint(point, cameraSystem->GetCurCamera()->GetZFar(), traceFrom, traceTo);
    }
    ProcessSelectedGroup(collisionSystem->ObjectsRayTest(traceFrom, traceTo));
}

void SceneSelectionSystem::PerformSelectionInCurrentBox()
{
    UpdateGroupSelectionMode();

    const DAVA::float32 minSelectionSize = 2.0f;

    DAVA::Vector2 selectionSize = selectionEndPoint - selectionStartPoint;
    if ((std::abs(selectionSize.x) < minSelectionSize) || (std::abs(selectionSize.y) < minSelectionSize))
    {
        return;
    };

    float minX = std::min(selectionStartPoint.x, selectionEndPoint.x);
    float minY = std::min(selectionStartPoint.y, selectionEndPoint.y);
    float maxX = std::max(selectionStartPoint.x, selectionEndPoint.x);
    float maxY = std::max(selectionStartPoint.y, selectionEndPoint.y);

    DAVA::Vector3 p0;
    DAVA::Vector3 p1;
    DAVA::Vector3 p2;
    DAVA::Vector3 p3;
    DAVA::Vector3 p4;
    SceneCameraSystem* cameraSystem = ((SceneEditor2*)GetScene())->cameraSystem;
    cameraSystem->GetRayTo2dPoint(DAVA::Vector2(minX, minY), 1.0f, p0, p1);
    cameraSystem->GetRayTo2dPoint(DAVA::Vector2(maxX, minY), 1.0f, p0, p2);
    cameraSystem->GetRayTo2dPoint(DAVA::Vector2(minX, maxY), 1.0f, p0, p4);
    cameraSystem->GetRayTo2dPoint(DAVA::Vector2(maxX, maxY), 1.0f, p0, p3);

    DAVA::Plane planes[4];
    planes[0] = DAVA::Plane(p2, p1, p0);
    planes[1] = DAVA::Plane(p3, p2, p0);
    planes[2] = DAVA::Plane(p4, p3, p0);
    planes[3] = DAVA::Plane(p1, p4, p0);

    const SelectableGroup& allSelectedObjects = collisionSystem->ClipObjectsToPlanes(planes, 4);

    SelectableGroup selectedObjects;
    for (const auto& item : allSelectedObjects.GetContent())
    {
        auto entity = item.AsEntity();
        if (entity == nullptr)
        {
            selectedObjects.Add(item.GetContainedObject(), GetUntransformedBoundingBox(item.GetContainedObject()));
        }
        else if (IsEntitySelectable(entity))
        {
            auto selectableEntity = GetSelectableEntity(entity);
            if (!selectableEntity->GetLocked())
            {
                selectedObjects.Add(selectableEntity, GetUntransformedBoundingBox(selectableEntity));
            }
        }
    }

    UpdateSelectionGroup(selectedObjects);
    applyOnPhaseEnd = true;
}

void SceneSelectionSystem::AddEntity(DAVA::Entity* entity)
{
    if (systemIsEnabled)
    {
        auto autoSelectionEnabled = SettingsManager::GetValue(Settings::Scene_AutoselectNewEntities).AsBool();
        if (autoSelectionEnabled && !IsLocked())
        {
            DAVA::Entity* parent = entity->GetParent();
            auto found = std::find(entitiesForSelection.begin(), entitiesForSelection.end(), parent);
            if (found == entitiesForSelection.end())
            {
                entitiesForSelection.push_back(entity); // need add only parent entities to select one
            }
        }
    }
}

void SceneSelectionSystem::RemoveEntity(DAVA::Entity* entity)
{
    if (!entitiesForSelection.empty())
    {
        entitiesForSelection.remove(entity);
    }

    ExcludeEntityFromSelection(entity);
    invalidSelectionBoxes = true;
}

void SceneSelectionSystem::Input(DAVA::UIEvent* event)
{
    if (IsLocked() || !selectionAllowed || (0 == componentMaskForSelection) || (event->mouseButton != DAVA::UIEvent::MouseButton::LEFT))
    {
        return;
    }

    if (DAVA::UIEvent::Phase::BEGAN == event->phase)
    {
        for (auto selectionDelegate : selectionDelegates)
        {
            if (selectionDelegate->AllowPerformSelectionHavingCurrent(currentSelection) == false)
            {
                return;
            }
        }

        selecting = true;
        selectionStartPoint = event->point;
        selectionEndPoint = selectionStartPoint;
        lastGroupSelection.Clear();
        PerformSelectionAtPoint(selectionStartPoint);
    }
    else if (selecting && (DAVA::UIEvent::Phase::DRAG == event->phase))
    {
        selectionEndPoint = event->point;
        PerformSelectionInCurrentBox();
    }
    else if (DAVA::UIEvent::Phase::ENDED == event->phase)
    {
        if ((event->mouseButton == DAVA::UIEvent::MouseButton::LEFT) && applyOnPhaseEnd)
        {
            FinishSelection();
        }
        applyOnPhaseEnd = false;
        selecting = false;
    }
}

void SceneSelectionSystem::DrawItem(const DAVA::AABBox3& originalBox, const DAVA::Matrix4& transform, DAVA::int32 drawMode,
                                    DAVA::RenderHelper::eDrawType wireDrawType, DAVA::RenderHelper::eDrawType solidDrawType, const DAVA::Color& color)
{
    DAVA::AABBox3 bbox;
    originalBox.GetTransformedBox(transform, bbox);

    // draw selection share
    if (drawMode & SS_DRAW_SHAPE)
    {
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(bbox, color, wireDrawType);
    }
    // draw selection share
    else if (drawMode & SS_DRAW_CORNERS)
    {
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABoxCorners(bbox, color, wireDrawType);
    }
    // fill selection shape
    if (drawMode & SS_DRAW_BOX)
    {
        DAVA::Color alphaScaledColor = color;
        alphaScaledColor.a *= 0.15f;
        GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABox(bbox, alphaScaledColor, solidDrawType);
    }
}

void SceneSelectionSystem::Draw()
{
    if (IsLocked())
    {
        return;
    }

    DAVA::Vector2 selectionSize = selectionEndPoint - selectionStartPoint;
    if (selecting && (selectionSize.Length() >= 1.0f))
    {
        DAVA::Rect targetRect(selectionStartPoint, selectionSize);
        DAVA::RenderSystem2D::Instance()->FillRect(targetRect, DAVA::Color(1.0f, 1.0f, 1.0f, 1.0f / 3.0f));
        DAVA::RenderSystem2D::Instance()->DrawRect(targetRect, DAVA::Color::White);
    }

    DAVA::int32 drawMode = SettingsManager::GetValue(Settings::Scene_SelectionDrawMode).AsInt32();

    DAVA::RenderHelper::eDrawType wireDrawType = (!(drawMode & SS_DRAW_NO_DEEP_TEST)) ?
    DAVA::RenderHelper::DRAW_WIRE_DEPTH :
    DAVA::RenderHelper::DRAW_WIRE_NO_DEPTH;

    DAVA::RenderHelper::eDrawType solidDrawType = (!(drawMode & SS_DRAW_NO_DEEP_TEST)) ?
    DAVA::RenderHelper::DRAW_SOLID_DEPTH :
    DAVA::RenderHelper::DRAW_SOLID_NO_DEPTH;

    bool replacingSelection = selecting && (groupSelectionMode == GroupSelectionMode::Replace);
    if (!replacingSelection)
    {
        for (const auto& item : currentSelection.GetContent())
        {
            if (item.SupportsTransformType(Selectable::TransformType::Disabled))
            {
                DrawItem(item.GetBoundingBox(), item.GetWorldTransform(), drawMode, wireDrawType, solidDrawType, DAVA::Color::White);
            }
        }
    }

    DAVA::Color drawColor = DAVA::Color::White;
    if (groupSelectionMode == GroupSelectionMode::Add)
    {
        drawColor = DAVA::Color(0.5f, 1.0f, 0.5f, 1.0f);
    }
    else if (groupSelectionMode == GroupSelectionMode::Remove)
    {
        drawColor = DAVA::Color(1.0f, 0.5f, 0.5f, 1.0f);
    }

    for (const auto& item : objectsToSelect.GetContent())
    {
        DrawItem(item.GetBoundingBox(), item.GetWorldTransform(), drawMode, wireDrawType, solidDrawType, drawColor);
    }
}

void SceneSelectionSystem::SetSelection(SelectableGroup& newSelection)
{
    if (IsLocked())
        return;

    newSelection.RemoveIf([this](const Selectable& obj) {
        return (obj.CanBeCastedTo<DAVA::Entity>() && !IsEntitySelectable(obj.AsEntity()));
    });

    /*
	 * Ask delegates if selection could be changed
	 */
    selectionHasChanges = true;
    for (auto selectionDelegate : selectionDelegates)
    {
        if (selectionDelegate->AllowChangeSelectionReplacingCurrent(currentSelection, newSelection) == false)
        {
            selectionHasChanges = false;
            return;
        }
    }

    /*
	 * Actually change selection
	 */
    recentlySelectedEntities.Clear();
    for (const auto& i : currentSelection.GetContent())
    {
        objectsToSelect.Remove(i.GetContainedObject());
        recentlySelectedEntities.Add(i.GetContainedObject(), i.GetBoundingBox());
    }
    currentSelection = newSelection;

    if (selectionHasChanges)
    {
        invalidSelectionBoxes = true;
        UpdateHoodPos();
    }
}

void SceneSelectionSystem::AddObjectToSelection(Selectable::Object* object)
{
    if (IsLocked() || currentSelection.ContainsObject(object))
        return;

    Selectable wrapper(object);
    if (!wrapper.CanBeCastedTo<DAVA::Entity>() || IsEntitySelectable(wrapper.AsEntity()))
    {
        auto newSelection = currentSelection;
        newSelection.Add(object, GetUntransformedBoundingBox(object));
        SetSelection(newSelection);
    }
}

void SceneSelectionSystem::AddGroupToSelection(const SelectableGroup& entities)
{
    if (IsLocked())
        return;

    SelectableGroup newSelection = currentSelection;
    for (const auto& item : entities.GetContent())
    {
        if (currentSelection.ContainsObject(item.GetContainedObject()))
            continue;

        auto entity = item.AsEntity();
        if ((entity == nullptr) || IsEntitySelectable(item.AsEntity()))
        {
            newSelection.Add(item.GetContainedObject(), GetUntransformedBoundingBox(item.GetContainedObject()));
        }
    }
    SetSelection(newSelection);
}

bool SceneSelectionSystem::IsEntitySelectable(DAVA::Entity* entity) const
{
    if (!IsLocked() && (entity != nullptr))
    {
        return (componentMaskForSelection & entity->GetAvailableComponentFlags());
    }

    return false;
}

void SceneSelectionSystem::ExcludeSingleItem(Selectable::Object* entity)
{
    auto newSelection = currentSelection;
    if (newSelection.ContainsObject(entity))
    {
        recentlySelectedEntities.Add(entity, DAVA::AABBox3());
        newSelection.Remove(entity);
    }
    if (objectsToSelect.ContainsObject(entity))
    {
        objectsToSelect.Remove(entity);
    }
    SetSelection(newSelection);
}

void SceneSelectionSystem::ExcludeEntityFromSelection(Selectable::Object* entity)
{
    if (!IsLocked())
    {
        ExcludeSingleItem(entity);
        currentSelection.RebuildIntegralBoundingBox();
        UpdateHoodPos();
    }
}

void SceneSelectionSystem::ExcludeSelection(const SelectableGroup& entities)
{
    if (!IsLocked())
    {
        for (const auto& item : entities.GetContent())
        {
            ExcludeSingleItem(item.GetContainedObject());
        }
        currentSelection.RebuildIntegralBoundingBox();
        UpdateHoodPos();
    }
}

void SceneSelectionSystem::Clear()
{
    if (!IsLocked())
    {
        auto allItems = currentSelection.GetContent();
        for (const auto& item : allItems)
        {
            ExcludeSingleItem(item.GetContainedObject());
        }
        currentSelection.RebuildIntegralBoundingBox();
        UpdateHoodPos();
    }
}

const SelectableGroup& SceneSelectionSystem::GetSelection() const
{
    static const SelectableGroup emptyGroup = SelectableGroup();
    return IsLocked() ? emptyGroup : currentSelection;
}

size_t SceneSelectionSystem::GetSelectionCount() const
{
    return IsLocked() == false ? currentSelection.GetSize() : 0;
}

DAVA::Entity* SceneSelectionSystem::GetFirstSelectionEntity() const
{
    if (IsLocked())
        return nullptr;

    DVASSERT(!currentSelection.IsEmpty())
    return currentSelection.GetContent().front().AsEntity();
}

void SceneSelectionSystem::CancelSelection()
{
    // don't change selection on phase end
    applyOnPhaseEnd = false;
}

void SceneSelectionSystem::SetPivotPoint(Selectable::TransformPivot pp)
{
    curPivotPoint = pp;
}

Selectable::TransformPivot SceneSelectionSystem::GetPivotPoint() const
{
    return curPivotPoint;
}

void SceneSelectionSystem::SetLocked(bool lock)
{
    bool lockChanged = IsLocked() != lock;
    SceneSystem::SetLocked(lock);

    hoodSystem->LockAxis(lock);
    hoodSystem->SetVisible(!lock);

    if (!lock)
    {
        UpdateHoodPos();
    }

    if (lockChanged)
    {
        SelectableGroup emptyGroup;
        SelectableGroup* selected = nullptr;
        SelectableGroup* deselected = nullptr;
        if (lock == true)
        {
            selected = &emptyGroup;
            deselected = &currentSelection;
        }
        else
        {
            selected = &currentSelection;
            deselected = &emptyGroup;
        }

        SceneSignals::Instance()->EmitSelectionChanged((SceneEditor2*)GetScene(), selected, deselected);
    }
}

void SceneSelectionSystem::UpdateHoodPos() const
{
    if (currentSelection.IsEmpty())
    {
        hoodSystem->LockModif(false);
        hoodSystem->SetVisible(false);
    }
    else
    {
        bool modificationEnabled = currentSelection.SupportsTransformType(modificationSystem->GetTransformType());
        hoodSystem->LockModif(modificationEnabled == false);

        DAVA::Vector3 hoodCenter;
        if (curPivotPoint == Selectable::TransformPivot::ObjectCenter)
        {
            hoodCenter = currentSelection.GetFirst().GetWorldTransform().GetTranslationVector();
        }
        else
        {
            hoodCenter = currentSelection.GetCommonWorldSpaceTranslationVector();
        }
        hoodSystem->SetPosition(hoodCenter);

        bool hasNonTransformableObjects = false;
        for (const auto& item : currentSelection.GetContent())
        {
            if (item.SupportsTransformType(Selectable::TransformType::Disabled) == false)
            {
                hasNonTransformableObjects = true;
                break;
            }
        }
        hoodSystem->SetVisible(hasNonTransformableObjects == false);
    }

    SceneEditor2* sc = static_cast<SceneEditor2*>(GetScene());
    sc->cameraSystem->UpdateDistanceToCamera();
}

DAVA::Entity* SceneSelectionSystem::GetSelectableEntity(DAVA::Entity* entity)
{
    DAVA::Entity* parent = entity;
    while (nullptr != parent)
    {
        if (parent->GetSolid())
        {
            entity = parent;
        }
        parent = parent->GetParent();
    }
    return entity;
}

DAVA::AABBox3 SceneSelectionSystem::GetUntransformedBoundingBox(Selectable::Object* entity) const
{
    return GetTransformedBoundingBox(Selectable(entity), DAVA::Matrix4::IDENTITY);
}

DAVA::AABBox3 SceneSelectionSystem::GetTransformedBoundingBox(const Selectable& object, const DAVA::Matrix4& transform) const
{
    DAVA::AABBox3 entityBox = collisionSystem->GetBoundingBox(object.GetContainedObject());

    if (object.CanBeCastedTo<DAVA::Entity>())
    {
        // add childs boxes into entity box
        auto entity = object.AsEntity();
        for (DAVA::int32 i = 0; i < entity->GetChildrenCount(); i++)
        {
            Selectable childEntity(entity->GetChild(i));
            DAVA::AABBox3 childBox = GetTransformedBoundingBox(childEntity, childEntity.GetLocalTransform());
            if (childBox.IsEmpty() == false)
            {
                if (entityBox.IsEmpty())
                {
                    entityBox = childBox;
                }
                else
                {
                    entityBox.AddAABBox(childBox);
                }
            }
        }
    }

    DAVA::AABBox3 ret;
    if (entityBox.IsEmpty() == false)
    {
        entityBox.GetTransformedBox(transform, ret);
    }
    return ret;
}

DAVA::AABBox3 SceneSelectionSystem::GetTransformedBoundingBox(const SelectableGroup& group) const
{
    DAVA::AABBox3 result;
    for (const auto& object : group.GetContent())
    {
        DAVA::AABBox3 transformed;
        object.GetBoundingBox().GetTransformedBox(object.GetWorldTransform(), transformed);
        result.AddAABBox(transformed);
    }
    return result;
}

void SceneSelectionSystem::SetSelectionComponentMask(DAVA::uint64 mask)
{
    componentMaskForSelection = mask;

    if (currentSelection.IsEmpty())
    {
        selectionHasChanges = true; // magic to say to selectionModel() of scene tree to reset selection
    }
    else
    {
        Clear();
    }
}

void SceneSelectionSystem::Activate()
{
    SetLocked(false);
}

void SceneSelectionSystem::Deactivate()
{
    SetLocked(true);
}

void SceneSelectionSystem::EnableSystem(bool enabled)
{
    systemIsEnabled = enabled;
}

bool SceneSelectionSystem::IsSystemEnabled() const
{
    return systemIsEnabled;
}

void SceneSelectionSystem::UpdateSelectionGroup(const SelectableGroup& newSelection)
{
    objectsToSelect.Exclude(lastGroupSelection);
    objectsToSelect.RemoveIf([](const Selectable& e) {
        return e.SupportsTransformType(Selectable::TransformType::Disabled);
    });

    if (groupSelectionMode == GroupSelectionMode::Replace)
    {
        objectsToSelect.Join(newSelection);
    }
    else if (groupSelectionMode == GroupSelectionMode::Add)
    {
        for (const auto& item : newSelection.GetContent())
        {
            auto obj = item.GetContainedObject();
            if (!currentSelection.ContainsObject(obj))
            {
                objectsToSelect.Add(obj, item.GetBoundingBox());
            }
        }
    }
    else if (groupSelectionMode == GroupSelectionMode::Remove)
    {
        for (const auto& item : newSelection.GetContent())
        {
            if (currentSelection.ContainsObject(item.GetContainedObject()))
            {
                objectsToSelect.Add(item.GetContainedObject(), item.GetBoundingBox());
            }
        }
    }

    lastGroupSelection = newSelection;
}

void SceneSelectionSystem::FinishSelection()
{
    SelectableGroup newSelection;

    if (groupSelectionMode == GroupSelectionMode::Replace)
    {
        newSelection.Join(objectsToSelect);
    }
    else if (groupSelectionMode == GroupSelectionMode::Add)
    {
        newSelection.Join(objectsToSelect);
        newSelection.Join(currentSelection);
    }
    else if (groupSelectionMode == GroupSelectionMode::Remove)
    {
        newSelection.Join(currentSelection);
        newSelection.Exclude(objectsToSelect);
    }
    else
    {
        DVASSERT_MSG(0, "Invalid selection mode");
    }
    objectsToSelect.Clear();

    SetSelection(newSelection);
}

void SceneSelectionSystem::AddSelectionDelegate(SceneSelectionSystemDelegate* delegate_)
{
    DVASSERT(std::find(selectionDelegates.begin(), selectionDelegates.end(), delegate_) == selectionDelegates.end());
    selectionDelegates.push_back(delegate_);
}

void SceneSelectionSystem::RemoveSelectionDelegate(SceneSelectionSystemDelegate* delegate_)
{
    auto i = std::remove(selectionDelegates.begin(), selectionDelegates.end(), delegate_);
    selectionDelegates.erase(i, selectionDelegates.end());
}
