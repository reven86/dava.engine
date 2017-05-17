#include "Classes/Selection/SelectionSystem.h"
#include "Classes/Selection/Selection.h"

#include "Engine/Engine.h"
#include "Engine/EngineContext.h"

#include "Scene/System/CollisionSystem.h"
#include "Scene/System/ModifSystem.h"
#include "Scene/System/HoodSystem.h"
#include "Scene/SceneSignals.h"
#include "Commands2/Base/RECommandNotificationObject.h"

ENUM_DECLARE(SelectionSystemDrawMode)
{
    ENUM_ADD(SS_DRAW_SHAPE);
    ENUM_ADD(SS_DRAW_CORNERS);
    ENUM_ADD(SS_DRAW_BOX);
    ENUM_ADD(SS_DRAW_NO_DEEP_TEST);
}

SelectionSystem::SelectionSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene)
{
    SceneEditor2* editorScene = static_cast<SceneEditor2*>(scene);
    collisionSystem = editorScene->collisionSystem;
    hoodSystem = editorScene->hoodSystem;
    modificationSystem = editorScene->modifSystem;

    DVASSERT(collisionSystem != nullptr);
    DVASSERT(hoodSystem != nullptr);
    DVASSERT(modificationSystem != nullptr);
    scene->GetEventSystem()->RegisterSystemForEvent(this, DAVA::EventSystem::SWITCH_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, DAVA::EventSystem::LOCAL_TRANSFORM_CHANGED);
    scene->GetEventSystem()->RegisterSystemForEvent(this, DAVA::EventSystem::TRANSFORM_PARENT_CHANGED);

    wasLockedInActiveMode = IsLocked();
}

SelectionSystem::~SelectionSystem()
{
    if (GetScene())
    {
        GetScene()->GetEventSystem()->UnregisterSystemForEvent(this, DAVA::EventSystem::SWITCH_CHANGED);
    }
}

void SelectionSystem::ImmediateEvent(DAVA::Component* component, DAVA::uint32 event)
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

void SelectionSystem::UpdateGroupSelectionMode()
{
    DAVA::Engine* engine = DAVA::Engine::Instance();
    DVASSERT(engine != nullptr);
    const DAVA::EngineContext* engineContext = engine->GetContext();
    DVASSERT(engineContext != nullptr);
    if (engineContext->inputSystem == nullptr)
    {
        return;
    }
    const DAVA::KeyboardDevice& keyboard = engineContext->inputSystem->GetKeyboard();

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

namespace SelectionSystemDetails
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

void SelectionSystem::Process(DAVA::float32 timeElapsed)
{
    if (IsLocked())
    {
        return;
    }

    if (!entitiesForSelection.empty())
    {
        Clear();
        for (auto& entity : entitiesForSelection)
        {
            if (false == SelectionSystemDetails::FindIfParentWasAdded(entity, entitiesForSelection, GetScene()))
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
        SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
        for (Selectable& item : currentSelection.GetMutableContent())
        {
            item.SetBoundingBox(sceneEditor->collisionSystem->GetUntransformedBoundingBox(item.GetContainedObject()));
        }

        currentSelection.RebuildIntegralBoundingBox();
        invalidSelectionBoxes = false;

        selectionBox = currentSelection.GetTransformedBoundingBox();
    }

    UpdateGroupSelectionMode();
    UpdateHoodPos();
}

void SelectionSystem::ProcessSelectedGroup(const SelectableGroup::CollectionType& allObjects)
{
    SelectableGroup::CollectionType collisionObjects;
    collisionObjects.reserve(allObjects.size());

    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
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
            collisionObjects.emplace_back(Selection::GetSelectableEntity(entity));
            wasAdded = true;
        }

        if (wasAdded)
        {
            collisionObjects.back().SetBoundingBox(sceneEditor->collisionSystem->GetUntransformedBoundingBox(collisionObjects.back().GetContainedObject()));
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
        ExcludeEntityFromSelection(firstEntity, false);
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
                objectsToSelect.Add(nextEntity, sceneEditor->collisionSystem->GetUntransformedBoundingBox(nextEntity));
            }
        }
        else
        {
            SelectableGroup newSelection;
            newSelection.Add(nextEntity, sceneEditor->collisionSystem->GetUntransformedBoundingBox(nextEntity));
            SetSelection(newSelection);
        }
    }
}

void SelectionSystem::PerformSelectionAtPoint(const DAVA::Vector2& point)
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

void SelectionSystem::PerformSelectionInCurrentBox()
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

    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
    SceneCameraSystem* cameraSystem = sceneEditor->cameraSystem;
    cameraSystem->GetRayTo2dPoint(DAVA::Vector2(minX, minY), 1.0f, p0, p1);
    cameraSystem->GetRayTo2dPoint(DAVA::Vector2(maxX, minY), 1.0f, p0, p2);
    cameraSystem->GetRayTo2dPoint(DAVA::Vector2(minX, maxY), 1.0f, p0, p4);
    cameraSystem->GetRayTo2dPoint(DAVA::Vector2(maxX, maxY), 1.0f, p0, p3);

    const DAVA::Vector<DAVA::Plane> planes =
    {
      DAVA::Plane(p2, p1, p0),
      DAVA::Plane(p3, p2, p0),
      DAVA::Plane(p4, p3, p0),
      DAVA::Plane(p1, p4, p0)
    };

    const SelectableGroup& allSelectedObjects = collisionSystem->ClipObjectsToPlanes(planes);

    SelectableGroup selectedObjects;
    for (const auto& item : allSelectedObjects.GetContent())
    {
        DAVA::Entity* entity = item.AsEntity();
        if (entity == nullptr)
        {
            Selectable::Object* object = item.GetContainedObject();
            if (!selectedObjects.ContainsObject(object))
            {
                selectedObjects.Add(object, sceneEditor->collisionSystem->GetUntransformedBoundingBox(object));
            }
        }
        else if (IsEntitySelectable(entity))
        {
            DAVA::Entity* selectableEntity = Selection::GetSelectableEntity(entity);
            if (!selectableEntity->GetLocked() && !selectedObjects.ContainsObject(selectableEntity))
            {
                selectedObjects.Add(selectableEntity, sceneEditor->collisionSystem->GetUntransformedBoundingBox(selectableEntity));
            }
        }
    }

    UpdateSelectionGroup(selectedObjects);
    applyOnPhaseEnd = true;
}

void SelectionSystem::AddEntity(DAVA::Entity* entity)
{
    if (systemIsEnabled && entity->GetName().find("editor.") == DAVA::String::npos && (entity != GetScene()))
    { // need ignore editor specific entities in auto selection
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

namespace SelectionSystemDetails
{
void EnumerateSelectableObjects(DAVA::ParticleEmitter* emitter, DAVA::Vector<Selectable::Object*>& enumeratedObjects)
{
    for (DAVA::ParticleLayer* layer : emitter->layers)
    {
        if (layer->innerEmitter != nullptr)
        {
            EnumerateSelectableObjects(layer->innerEmitter, enumeratedObjects);
        }
    }

    enumeratedObjects.push_back(emitter);
}

void EnumerateSelectableObjects(DAVA::Entity* entity, DAVA::Vector<Selectable::Object*>& enumeratedObjects)
{
    DAVA::ParticleEffectComponent* particleEffect = DAVA::GetEffectComponent(entity);
    if (particleEffect != nullptr)
    {
        for (DAVA::int32 i = 0, e = particleEffect->GetEmittersCount(); i < e; ++i)
        {
            EnumerateSelectableObjects(particleEffect->GetEmitterInstance(i)->GetEmitter(), enumeratedObjects);
            enumeratedObjects.push_back(particleEffect->GetEmitterInstance(i));
        }
    }

    enumeratedObjects.push_back(entity);
}
}

void SelectionSystem::RemoveEntity(DAVA::Entity* entity)
{
    if (!entitiesForSelection.empty())
    {
        entitiesForSelection.remove(entity);
    }

    DAVA::Vector<Selectable::Object*> potentiallySelectedObjects;
    SelectionSystemDetails::EnumerateSelectableObjects(entity, potentiallySelectedObjects);
    for (Selectable::Object* object : potentiallySelectedObjects)
    {
        ExcludeEntityFromSelection(object, true);
    }

    invalidSelectionBoxes = true;
}

bool SelectionSystem::Input(DAVA::UIEvent* event)
{
    if (IsLocked() || !selectionAllowed || (0 == componentMaskForSelection) || (event->mouseButton != DAVA::eMouseButtons::LEFT))
    {
        return false;
    }

    if (DAVA::UIEvent::Phase::BEGAN == event->phase)
    {
        for (auto selectionDelegate : selectionDelegates)
        {
            if (selectionDelegate->AllowPerformSelectionHavingCurrent(currentSelection) == false)
            {
                return false;
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
        if ((event->mouseButton == DAVA::eMouseButtons::LEFT) && applyOnPhaseEnd)
        {
            FinishSelection();
        }
        applyOnPhaseEnd = false;
        selecting = false;
    }
    return false;
}

void SelectionSystem::DrawItem(const DAVA::AABBox3& originalBox, const DAVA::Matrix4& transform, DAVA::int32 drawMode,
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

void SelectionSystem::Draw()
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

void SelectionSystem::ProcessCommand(const RECommandNotificationObject& commandNotification)
{
    static const DAVA::Vector<DAVA::uint32> commandIds = { CMDID_ENTITY_REMOVE, CMDID_ENTITY_CHANGE_PARENT, CMDID_TRANSFORM, CMDID_CONVERT_TO_BILLBOARD };
    if (commandNotification.MatchCommandIDs(commandIds))
    {
        invalidSelectionBoxes = true;
    }
}

void SelectionSystem::SetSelection(SelectableGroup& newSelection)
{
    if (IsLocked())
        return;

    newSelection.RemoveIf([this](const Selectable& obj) {
        return (obj.CanBeCastedTo<DAVA::Entity>() && !IsEntitySelectable(obj.AsEntity()));
    });

    /*
	 * Ask delegates if selection could be changed
	 */
    for (auto selectionDelegate : selectionDelegates)
    {
        if (selectionDelegate->AllowChangeSelectionReplacingCurrent(currentSelection, newSelection) == false)
        {
            return;
        }
    }

    /*
	 * Actually change selection
	 */
    for (const auto& i : currentSelection.GetContent())
    {
        objectsToSelect.Remove(i.GetContainedObject());
    }
    currentSelection = newSelection;

    invalidSelectionBoxes = true;
    UpdateHoodPos();
}

void SelectionSystem::AddObjectToSelection(Selectable::Object* object)
{
    if (IsLocked() || currentSelection.ContainsObject(object))
        return;

    Selectable wrapper(object);
    if (!wrapper.CanBeCastedTo<DAVA::Entity>() || IsEntitySelectable(wrapper.AsEntity()))
    {
        auto newSelection = currentSelection;
        SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
        newSelection.Add(object, sceneEditor->collisionSystem->GetUntransformedBoundingBox(object));
        SetSelection(newSelection);
    }
}

void SelectionSystem::AddGroupToSelection(const SelectableGroup& entities)
{
    if (IsLocked())
        return;

    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());

    SelectableGroup newSelection = currentSelection;
    for (const auto& item : entities.GetContent())
    {
        if (currentSelection.ContainsObject(item.GetContainedObject()))
            continue;

        auto entity = item.AsEntity();
        if ((entity == nullptr) || IsEntitySelectable(item.AsEntity()))
        {
            newSelection.Add(item.GetContainedObject(), sceneEditor->collisionSystem->GetUntransformedBoundingBox(item.GetContainedObject()));
        }
    }
    SetSelection(newSelection);
}

bool SelectionSystem::IsEntitySelectable(DAVA::Entity* entity) const
{
    if (!IsLocked() && (entity != nullptr))
    {
        return (componentMaskForSelection & entity->GetAvailableComponentFlags());
    }

    return false;
}

void SelectionSystem::ExcludeSingleItem(Selectable::Object* entity)
{
    auto newSelection = currentSelection;
    if (newSelection.ContainsObject(entity))
    {
        newSelection.Remove(entity);
    }
    if (objectsToSelect.ContainsObject(entity))
    {
        objectsToSelect.Remove(entity);
    }
    SetSelection(newSelection);
}

void SelectionSystem::ExcludeEntityFromSelection(Selectable::Object* entity, bool forceRemove)
{
    if (!IsLocked() || forceRemove == true)
    {
        ExcludeSingleItem(entity);
        invalidSelectionBoxes = true;
        UpdateHoodPos();
    }
}

void SelectionSystem::ExcludeSelection(const SelectableGroup& entities)
{
    if (!IsLocked())
    {
        for (const auto& item : entities.GetContent())
        {
            ExcludeSingleItem(item.GetContainedObject());
        }
        invalidSelectionBoxes = true;
        UpdateHoodPos();
    }
}

void SelectionSystem::Clear()
{
    if (!IsLocked())
    {
        auto allItems = currentSelection.GetContent();
        for (const auto& item : allItems)
        {
            ExcludeSingleItem(item.GetContainedObject());
        }
        invalidSelectionBoxes = true;
        UpdateHoodPos();
    }
}

const SelectableGroup& SelectionSystem::GetSelection() const
{
    static const SelectableGroup emptyGroup = SelectableGroup();
    return IsLocked() ? emptyGroup : currentSelection;
}

void SelectionSystem::CancelSelection()
{
    // don't change selection on phase end
    applyOnPhaseEnd = false;
}

void SelectionSystem::SetLocked(bool lock)
{
    SceneSystem::SetLocked(lock);

    hoodSystem->LockAxis(lock);
    hoodSystem->SetVisible(!lock);

    if (!lock)
    {
        UpdateHoodPos();
    }
}

void SelectionSystem::UpdateHoodPos() const
{
    if (currentSelection.IsEmpty())
    {
        hoodSystem->LockModif(false);
        hoodSystem->SetVisible(false);
    }
    else
    {
        const SelectableGroup& transformableSelection = modificationSystem->GetTransformableSelection();
        bool transformableSelectionEmpty = transformableSelection.IsEmpty();
        hoodSystem->LockModif(transformableSelectionEmpty);

        if (!transformableSelectionEmpty)
        {
            DAVA::Vector3 hoodCenter;
            if (modificationSystem->GetPivotPoint() == Selectable::TransformPivot::ObjectCenter)
            {
                hoodCenter = currentSelection.GetFirst().GetWorldTransform().GetTranslationVector();
            }
            else
            {
                hoodCenter = currentSelection.GetCommonWorldSpaceTranslationVector();
            }
            hoodSystem->SetPosition(hoodCenter);
            hoodSystem->SetVisible(true);
        }
    }

    SceneEditor2* sc = static_cast<SceneEditor2*>(GetScene());
    sc->cameraSystem->UpdateDistanceToCamera();
}

void SelectionSystem::SetSelectionComponentMask(DAVA::uint64 mask)
{
    componentMaskForSelection = mask;

    if (currentSelection.IsEmpty() == false)
    {
        Clear();
    }
}

void SelectionSystem::Activate()
{
    SetLocked(wasLockedInActiveMode);
}

void SelectionSystem::Deactivate()
{
    wasLockedInActiveMode = IsLocked();
    SetLocked(true);
}

void SelectionSystem::UpdateSelectionGroup(const SelectableGroup& newSelection)
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

void SelectionSystem::FinishSelection()
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
        DVASSERT(0, "Invalid selection mode");
    }
    objectsToSelect.Clear();

    SetSelection(newSelection);
}

void SelectionSystem::AddDelegate(SelectionSystemDelegate* delegate_)
{
    DVASSERT(std::find(selectionDelegates.begin(), selectionDelegates.end(), delegate_) == selectionDelegates.end());
    selectionDelegates.push_back(delegate_);
}

void SelectionSystem::RemoveDelegate(SelectionSystemDelegate* delegate_)
{
    auto i = std::remove(selectionDelegates.begin(), selectionDelegates.end(), delegate_);
    selectionDelegates.erase(i, selectionDelegates.end());
}

const DAVA::AABBox3& SelectionSystem::GetSelectionBox() const
{
    return selectionBox;
}
