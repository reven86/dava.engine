#include "Qt/Scene/System/ModifSystem.h"
#include "Qt/Scene/System/HoodSystem.h"
#include "Qt/Scene/System/CameraSystem.h"
#include "Qt/Scene/System/CollisionSystem.h"
#include "Qt/Scene/System/SelectionSystem.h"
#include "Qt/Scene/System/TextDrawSystem.h"
#include "Qt/Scene/SceneSignals.h"

#include "Scene/SceneEditor2.h"

#include "Scene3D/Systems/StaticOcclusionSystem.h"

#include "Commands2/TransformCommand.h"
#include "Commands2/BakeTransformCommand.h"
#include "Commands2/EntityAddCommand.h"
#include "Commands2/EntityLockCommand.h"
#include <QApplication>

EntityModificationSystem::EntityModificationSystem(DAVA::Scene* scene, SceneCollisionSystem* colSys, SceneCameraSystem* camSys, HoodSystem* hoodSys)
    : DAVA::SceneSystem(scene)
    , collisionSystem(colSys)
    , cameraSystem(camSys)
    , hoodSystem(hoodSys)
{
    SetTransformType(Selectable::TransformType::Disabled);
    SetModifAxis(ST_AXIS_Z);
}

void EntityModificationSystem::SetModifAxis(ST_Axis axis)
{
    if (axis != ST_AXIS_NONE)
    {
        curAxis = axis;
        hoodSystem->SetModifAxis(axis);
    }
}

ST_Axis EntityModificationSystem::GetModifAxis() const
{
    return curAxis;
}

void EntityModificationSystem::SetTransformType(Selectable::TransformType mode)
{
    transformType = mode;
    hoodSystem->SetTransformType(mode);
}

Selectable::TransformType EntityModificationSystem::GetTransformType() const
{
    return transformType;
}

bool EntityModificationSystem::GetLandscapeSnap() const
{
    return snapToLandscape;
}

void EntityModificationSystem::SetLandscapeSnap(bool snap)
{
    snapToLandscape = snap;
}

void EntityModificationSystem::PlaceOnLandscape(const SelectableGroup& entities)
{
    if (ModifCanStart(entities))
    {
        bool prevSnapToLandscape = snapToLandscape;

        snapToLandscape = true;
        BeginModification(entities);

        // move by z axis, so we will snap to landscape and keep x,y coords unmodified
        DAVA::Vector3 newPos3d = modifStartPos3d;
        newPos3d.z += 1.0f;
        Move(newPos3d);

        ApplyModification();
        EndModification();

        snapToLandscape = prevSnapToLandscape;
    }
}

void EntityModificationSystem::ResetTransform(const SelectableGroup& entities)
{
    SceneEditor2* sceneEditor = ((SceneEditor2*)GetScene());
    if (nullptr != sceneEditor && ModifCanStart(entities))
    {
        DAVA::Matrix4 zeroTransform;
        zeroTransform.Identity();

        sceneEditor->BeginBatch("Multiple transform", entities.GetSize());
        for (const Selectable& item : entities.GetContent())
        {
            sceneEditor->Exec(Command2::Create<TransformCommand>(item, item.GetLocalTransform(), zeroTransform));
        }
        sceneEditor->EndBatch();
    }
}

bool EntityModificationSystem::InModifState() const
{
    return inModifState;
}

bool EntityModificationSystem::InCloneState() const
{
    return (cloneState == CLONE_NEED);
}

bool EntityModificationSystem::InCloneDoneState() const
{
    return (cloneState == CLONE_DONE);
}

void EntityModificationSystem::Process(DAVA::float32 timeElapsed)
{
}

void EntityModificationSystem::Input(DAVA::UIEvent* event)
{
    if (IsLocked() || (collisionSystem == nullptr))
    {
        return;
    }

    // current selected entities
    SceneSelectionSystem* selectionSystem = static_cast<SceneEditor2*>(GetScene())->selectionSystem;
    const SelectableGroup& selectedEntities = selectionSystem->GetSelection();

    DAVA::Camera* camera = cameraSystem->GetCurCamera();

    // if we are not in modification state, try to find some selected item
    // that have mouse cursor at the top of it
    if (!inModifState)
    {
        // can we start modification???
        if (ModifCanStartByMouse(selectedEntities))
        {
            SceneSignals::Instance()->EmitMouseOverSelection((SceneEditor2*)GetScene(), &selectedEntities);

            if (DAVA::UIEvent::Phase::BEGAN == event->phase)
            {
                if (event->mouseButton == DAVA::UIEvent::MouseButton::LEFT)
                {
                    // go to modification state
                    inModifState = true;

                    // select current hood axis as active
                    if ((transformType == Selectable::TransformType::Translation) || (transformType == Selectable::TransformType::Rotation))
                    {
                        SetModifAxis(hoodSystem->GetPassingAxis());
                    }

                    // set entities to be modified
                    BeginModification(selectedEntities);

                    // init some values, needed for modifications
                    modifStartPos3d = CamCursorPosToModifPos(camera, event->point);
                    modifStartPos2d = event->point;

                    // check if this is move with copy action
                    int curKeyModifiers = QApplication::keyboardModifiers();
                    if (curKeyModifiers & Qt::ShiftModifier && (transformType == Selectable::TransformType::Translation))
                    {
                        cloneState = CLONE_NEED;
                    }
                }
            }
        }
        else
        {
            SceneSignals::Instance()->EmitMouseOverSelection((SceneEditor2*)GetScene(), nullptr);
        }
    }
    // or we are already in modification state
    else
    {
        // phase still continue
        if (event->phase == DAVA::UIEvent::Phase::DRAG)
        {
            DAVA::Vector3 moveOffset;
            DAVA::float32 rotateAngle;
            DAVA::float32 scaleForce;

            switch (transformType)
            {
            case Selectable::TransformType::Translation:
            {
                DAVA::Vector3 newPos3d = CamCursorPosToModifPos(camera, event->point);
                moveOffset = Move(newPos3d);
                modified = true;
            }
            break;
            case Selectable::TransformType::Rotation:
            {
                rotateAngle = Rotate(event->point);
                modified = true;
            }
            break;
            case Selectable::TransformType::Scale:
            {
                scaleForce = Scale(event->point);
                modified = true;
            }
            break;
            default:
                break;
            }

            if (modified)
            {
                if (cloneState == CLONE_NEED)
                {
                    CloneBegin();
                    cloneState = CLONE_DONE;
                }

                // say to selection system, that selected items were modified
                selectionSystem->CancelSelection();

                // lock hood, so it wont process ui events, wont calc. scale depending on it current position
                hoodSystem->LockScale(true);
                hoodSystem->SetModifOffset(moveOffset);
                hoodSystem->SetModifRotate(rotateAngle);
                hoodSystem->SetModifScale(scaleForce);
            }
        }
        // phase ended
        else if (event->phase == DAVA::UIEvent::Phase::ENDED)
        {
            if (event->mouseButton == DAVA::UIEvent::MouseButton::LEFT)
            {
                if (modified)
                {
                    if (cloneState == CLONE_DONE)
                    {
                        CloneEnd();
                    }
                    else
                    {
                        ApplyModification();
                    }
                }

                hoodSystem->SetModifOffset(DAVA::Vector3(0, 0, 0));
                hoodSystem->SetModifRotate(0);
                hoodSystem->SetModifScale(0);
                hoodSystem->LockScale(false);

                EndModification();
                inModifState = false;
                modified = false;
                cloneState = CLONE_DONT;
            }
        }
    }
}

void EntityModificationSystem::AddDelegate(EntityModificationSystemDelegate* delegate)
{
    delegates.push_back(delegate);
}

void EntityModificationSystem::RemoveDelegate(EntityModificationSystemDelegate* delegate)
{
    delegates.remove(delegate);
}

SelectableGroup EntityModificationSystem::BeginModification(const SelectableGroup& inputEntities)
{
    EndModification();
    if (inputEntities.IsEmpty())
        return inputEntities;

    SelectableGroup result = inputEntities;
    result.RemoveObjectsWithDependantTransform();

    DAVA::AABBox3 localBox;
    for (const Selectable& item : result.GetContent())
    {
        localBox.AddPoint(item.GetLocalTransform().GetTranslationVector());
    }
    DAVA::Vector3 averageLocalTranslation = localBox.GetCenter();

    modifEntities.reserve(result.GetSize());
    for (const Selectable& item : result.GetContent())
    {
        modifEntities.emplace_back();
        EntityToModify& etm = modifEntities.back();

        etm.object = item;
        etm.originalTransform = item.GetLocalTransform();

        etm.toLocalZero.CreateTranslation(-etm.originalTransform.GetTranslationVector());
        etm.fromLocalZero.CreateTranslation(etm.originalTransform.GetTranslationVector());

        etm.toWorldZero.CreateTranslation(-averageLocalTranslation);
        etm.fromWorldZero.CreateTranslation(averageLocalTranslation);

        etm.originalParentWorldTransform.Identity();

        // inverse parent world transform, and remember it
        DAVA::Entity* entity = item.AsEntity();
        if ((entity != nullptr) && (entity->GetParent() != nullptr))
        {
            etm.originalParentWorldTransform = entity->GetParent()->GetWorldTransform();
        }
        else if (item.CanBeCastedTo<DAVA::ParticleEmitterInstance>()) // special case for emitter
        {
            DAVA::ParticleEmitterInstance* emitter = item.Cast<DAVA::ParticleEmitterInstance>();
            DAVA::ParticleEffectComponent* ownerComponent = emitter->GetOwner();
            if (ownerComponent != nullptr)
            {
                DAVA::Entity* ownerEntity = ownerComponent->GetEntity();
                if (ownerEntity != nullptr)
                {
                    etm.originalParentWorldTransform = ownerEntity->GetWorldTransform();
                }
            }
        }

        etm.inversedParentWorldTransform = etm.originalParentWorldTransform;
        etm.inversedParentWorldTransform.SetTranslationVector(DAVA::Vector3(0.0f, 0.0f, 0.0f));
        if (!etm.inversedParentWorldTransform.Inverse())
        {
            etm.inversedParentWorldTransform.Identity();
        }
    }

    // remember axis vector we are rotating around
    switch (curAxis)
    {
    case ST_AXIS_X:
    case ST_AXIS_YZ:
        rotateAround = DAVA::Vector3(1, 0, 0);
        break;
    case ST_AXIS_Y:
    case ST_AXIS_XZ:
        rotateAround = DAVA::Vector3(0, 1, 0);
        break;
    case ST_AXIS_XY:
    case ST_AXIS_Z:
        rotateAround = DAVA::Vector3(0, 0, 1);
        break;

    default:
        break;
    }

    // 2d axis projection we are rotating around
    modifEntitiesCenter = inputEntities.GetCommonWorldSpaceTranslationVector();
    DAVA::Vector2 rotateAxis = Cam2dProjection(modifEntitiesCenter, modifEntitiesCenter + rotateAround);

    // axis dot products
    DAVA::Vector2 zeroPos = cameraSystem->GetScreenPos(modifEntitiesCenter);
    DAVA::Vector2 xPos = cameraSystem->GetScreenPos(modifEntitiesCenter + DAVA::Vector3(1, 0, 0));
    DAVA::Vector2 yPos = cameraSystem->GetScreenPos(modifEntitiesCenter + DAVA::Vector3(0, 1, 0));
    DAVA::Vector2 zPos = cameraSystem->GetScreenPos(modifEntitiesCenter + DAVA::Vector3(0, 0, 1));

    DAVA::Vector2 vx = xPos - zeroPos;
    DAVA::Vector2 vy = yPos - zeroPos;
    DAVA::Vector2 vz = zPos - zeroPos;

    crossXY = DAVA::Abs(vx.CrossProduct(vy));
    crossXZ = DAVA::Abs(vx.CrossProduct(vz));
    crossYZ = DAVA::Abs(vy.CrossProduct(vz));

    // real rotate should be done in direction of 2dAxis normal,
    // so calculate this normal
    rotateNormal = DAVA::Vector2(-rotateAxis.y, rotateAxis.x);
    if (!rotateNormal.IsZero())
    {
        rotateNormal.Normalize();
    }

    DAVA::Camera* camera = cameraSystem->GetCurCamera();
    if (camera != nullptr)
    {
        isOrthoModif = camera->GetIsOrtho();
    }

    return result;
}

void EntityModificationSystem::EndModification()
{
    modifEntitiesCenter.Set(0, 0, 0);
    modifEntities.clear();
    isOrthoModif = false;
}

bool EntityModificationSystem::ModifCanStart(const SelectableGroup& objects) const
{
    return !objects.IsEmpty() && objects.SupportsTransformType(transformType);
}

bool EntityModificationSystem::ModifCanStartByMouse(const SelectableGroup& objects) const
{
    if (ModifCanStart(objects) == false)
        return false;

    // we can start modification if mouse is over hood
    // on mouse is over one of currently selected items
    if (hoodSystem->GetPassingAxis() != ST_AXIS_NONE)
        return true;

    if (SettingsManager::GetValue(Settings::Scene_ModificationByGizmoOnly).AsBool())
        return false;

    // send this ray to collision system and get collision objects
    // check if one of got collision objects is intersected with selected items
    // if so - we can start modification
    const SelectableGroup::CollectionType& collisionEntities = collisionSystem->ObjectsRayTestFromCamera();
    if (collisionEntities.empty())
        return false;

    for (const Selectable& collisionItem : collisionEntities)
    {
        for (const Selectable& selectedItem : objects.GetContent())
        {
            if (selectedItem == collisionItem)
                return true;

            DAVA::Entity* selectedEntity = selectedItem.AsEntity();
            DAVA::Entity* collisionEntity = collisionItem.AsEntity();
            if ((selectedEntity != nullptr) && (collisionEntity != nullptr) && selectedEntity->GetSolid())
            {
                if (IsEntityContainRecursive(selectedEntity, collisionEntity))
                    return true;
            }
        }
    }

    return false;
}

void EntityModificationSystem::ApplyModification()
{
    SceneEditor2* sceneEditor = ((SceneEditor2*)GetScene());

    if (sceneEditor == nullptr)
        return;

    bool transformChanged = false;
    DAVA::uint32 count = static_cast<DAVA::uint32>(modifEntities.size());
    for (DAVA::uint32 i = 0; i < count; ++i)
    {
        if (modifEntities[i].originalTransform != modifEntities[i].object.GetLocalTransform())
        {
            transformChanged = true;
            break;
        }
    }

    if (transformChanged)
    {
        sceneEditor->BeginBatch("Multiple transform", count);
        for (size_t i = 0; i < count; ++i)
        {
            sceneEditor->Exec(Command2::Create<TransformCommand>(modifEntities[i].object, modifEntities[i].originalTransform, modifEntities[i].object.GetLocalTransform()));
        }
        sceneEditor->EndBatch();
    }
}

DAVA::Vector3 EntityModificationSystem::CamCursorPosToModifPos(DAVA::Camera* camera, DAVA::Vector2 pos)
{
    DAVA::Vector3 ret;

    if (NULL != camera)
    {
        if (camera->GetIsOrtho())
        {
            DAVA::Vector3 dir = cameraSystem->GetPointDirection(pos);
            ret = DAVA::Vector3(dir.x, dir.y, 0);
        }
        else
        {
            DAVA::Vector3 planeNormal;
            DAVA::Vector3 camPosition = cameraSystem->GetCameraPosition();
            DAVA::Vector3 camToPointDirection = cameraSystem->GetPointDirection(pos);

            switch (curAxis)
            {
            case ST_AXIS_X:
            {
                if (crossXY > crossXZ)
                    planeNormal = DAVA::Vector3(0, 0, 1);
                else
                    planeNormal = DAVA::Vector3(0, 1, 0);
            }
            break;
            case ST_AXIS_Y:
            {
                if (crossXY > crossYZ)
                    planeNormal = DAVA::Vector3(0, 0, 1);
                else
                    planeNormal = DAVA::Vector3(1, 0, 0);
            }
            break;
            case ST_AXIS_Z:
            {
                if (crossXZ > crossYZ)
                    planeNormal = DAVA::Vector3(0, 1, 0);
                else
                    planeNormal = DAVA::Vector3(1, 0, 0);
            }
            break;
            case ST_AXIS_XZ:
                planeNormal = DAVA::Vector3(0, 1, 0);
                break;
            case ST_AXIS_YZ:
                planeNormal = DAVA::Vector3(1, 0, 0);
                break;
            case ST_AXIS_XY:
            default:
                planeNormal = DAVA::Vector3(0, 0, 1);
                break;
            }

            DAVA::Plane plane(planeNormal, modifEntitiesCenter);
            DAVA::float32 distance = FLT_MAX;

            plane.IntersectByRay(camPosition, camToPointDirection, distance);
            ret = camPosition + (camToPointDirection * distance);
        }
    }

    return ret;
}

DAVA::Vector2 EntityModificationSystem::Cam2dProjection(const DAVA::Vector3& from, const DAVA::Vector3& to)
{
    DAVA::Vector2 axisBegin = cameraSystem->GetScreenPos(from);
    DAVA::Vector2 axisEnd = cameraSystem->GetScreenPos(to);
    DAVA::Vector2 ret = axisEnd - axisBegin;

    if (ret.IsZero())
    {
        ret = DAVA::Vector2(1.0f, 1.0f);
    }

    ret.Normalize();
    return ret;
}

DAVA::Vector3 EntityModificationSystem::Move(const DAVA::Vector3& newPos3d)
{
    DAVA::Vector3 moveOffset;
    DAVA::Vector3 modifPosWithLocedAxis = modifStartPos3d;
    DAVA::Vector3 deltaPos3d = newPos3d - modifStartPos3d;

    switch (curAxis)
    {
    case ST_AXIS_X:
        modifPosWithLocedAxis.x += DAVA::Vector3(1, 0, 0).DotProduct(deltaPos3d);
        break;
    case ST_AXIS_Y:
        modifPosWithLocedAxis.y += DAVA::Vector3(0, 1, 0).DotProduct(deltaPos3d);
        break;
    case ST_AXIS_Z:
        if (!isOrthoModif)
        {
            modifPosWithLocedAxis.z += DAVA::Vector3(0, 0, 1).DotProduct(deltaPos3d);
        }
        break;
    case ST_AXIS_XY:
        modifPosWithLocedAxis.x = newPos3d.x;
        modifPosWithLocedAxis.y = newPos3d.y;
        break;
    case ST_AXIS_XZ:
        if (!isOrthoModif)
        {
            modifPosWithLocedAxis.x = newPos3d.x;
            modifPosWithLocedAxis.z = newPos3d.z;
        }
        break;
    case ST_AXIS_YZ:
        if (!isOrthoModif)
        {
            modifPosWithLocedAxis.z = newPos3d.z;
            modifPosWithLocedAxis.y = newPos3d.y;
        }
        break;
    default:
        break;
    }

    moveOffset = modifPosWithLocedAxis - modifStartPos3d;

    for (EntityToModify& etm : modifEntities)
    {
        DAVA::Matrix4 moveModification = DAVA::Matrix4::MakeTranslation(moveOffset * etm.inversedParentWorldTransform);
        DAVA::Matrix4 newLocalTransform = etm.originalTransform * moveModification;

        if (snapToLandscape)
        {
            newLocalTransform = newLocalTransform * SnapToLandscape(newLocalTransform.GetTranslationVector(), etm.originalParentWorldTransform);
        }

        etm.object.SetLocalTransform(newLocalTransform);
    }

    return moveOffset;
}

DAVA::float32 EntityModificationSystem::Rotate(const DAVA::Vector2& newPos2d)
{
    SceneSelectionSystem* selectionSystem = static_cast<SceneEditor2*>(GetScene())->selectionSystem;
    Selectable::TransformPivot pivotPoint = selectionSystem->GetPivotPoint();

    DAVA::Vector2 rotateLength = newPos2d - modifStartPos2d;
    DAVA::float32 rotateForce = -(rotateNormal.DotProduct(rotateLength)) / 70.0f;

    for (EntityToModify& etm : modifEntities)
    {
        DAVA::Matrix4 rotateModification = DAVA::Matrix4::MakeRotation(rotateAround, rotateForce);
        DAVA::Matrix4& toZero = (pivotPoint == Selectable::TransformPivot::ObjectCenter) ? etm.toLocalZero : etm.toWorldZero;
        DAVA::Matrix4& fromZero = (pivotPoint == Selectable::TransformPivot::ObjectCenter) ? etm.fromLocalZero : etm.fromWorldZero;
        etm.object.SetLocalTransform(etm.originalTransform * toZero * rotateModification * fromZero);
    }

    return rotateForce;
}

DAVA::float32 EntityModificationSystem::Scale(const DAVA::Vector2& newPos2d)
{
    SceneSelectionSystem* selectionSystem = static_cast<SceneEditor2*>(GetScene())->selectionSystem;
    Selectable::TransformPivot pivotPoint = selectionSystem->GetPivotPoint();

    DAVA::Vector2 scaleDir = (newPos2d - modifStartPos2d);
    DAVA::float32 scaleForce = 1.0f - (scaleDir.y / 70.0f);

    if (scaleForce >= 0.0f)
    {
        for (EntityToModify& etm : modifEntities)
        {
            DAVA::Vector3 scaleVector(scaleForce, scaleForce, scaleForce);
            DAVA::Matrix4 scaleModification = DAVA::Matrix4::MakeScale(scaleVector);
            DAVA::Matrix4& toZero = (pivotPoint == Selectable::TransformPivot::ObjectCenter) ? etm.toLocalZero : etm.toWorldZero;
            DAVA::Matrix4& fromZero = (pivotPoint == Selectable::TransformPivot::ObjectCenter) ? etm.fromLocalZero : etm.fromWorldZero;
            etm.object.SetLocalTransform(etm.originalTransform * toZero * scaleModification * fromZero);
        }
    }

    return scaleForce;
}

DAVA::Matrix4 EntityModificationSystem::SnapToLandscape(const DAVA::Vector3& point, const DAVA::Matrix4& originalParentTransform) const
{
    DAVA::Matrix4 ret;
    ret.Identity();

    DAVA::Landscape* landscape = collisionSystem->GetLandscape();
    if (NULL != landscape)
    {
        DAVA::Vector3 resPoint;
        DAVA::Vector3 realPoint = point * originalParentTransform;

        if (landscape->PlacePoint(realPoint, resPoint))
        {
            resPoint = resPoint - realPoint;
            ret.SetTranslationVector(resPoint);
        }
    }

    return ret;
}

bool EntityModificationSystem::IsEntityContainRecursive(const DAVA::Entity* entity, const DAVA::Entity* child) const
{
    bool ret = false;

    if (NULL != entity && NULL != child)
    {
        for (int i = 0; !ret && i < entity->GetChildrenCount(); ++i)
        {
            if (child == entity->GetChild(i))
            {
                ret = true;
            }
            else
            {
                ret = IsEntityContainRecursive(entity->GetChild(i), child);
            }
        }
    }

    return ret;
}

void EntityModificationSystem::CloneBegin()
{
    // remove modif entities that are children for other modif entities
    for (DAVA::uint32 i = 0; i < modifEntities.size(); ++i)
    {
        DAVA::Entity* iEntity = modifEntities[i].object.AsEntity();
        for (DAVA::uint32 j = 0; (iEntity != nullptr) && (j < modifEntities.size()); ++j)
        {
            if (i == j)
                continue;

            DAVA::Entity* jEntity = modifEntities[j].object.AsEntity();
            if ((jEntity != nullptr) && jEntity->IsMyChildRecursive(iEntity))
            {
                DAVA::RemoveExchangingWithLast(modifEntities, i);
                --i;
                break;
            }
        }
    }

    if (modifEntities.empty())
        return;

    clonedEntities.reserve(modifEntities.size());
    for (const EntityToModify& item : modifEntities)
    {
        DAVA::Entity* origEntity = item.object.AsEntity();
        if (origEntity == nullptr)
            continue;

        for (auto delegate : delegates)
        {
            delegate->WillClone(origEntity);
        }
        DAVA::Entity* newEntity = origEntity->Clone();
        for (auto delegate : delegates)
        {
            delegate->DidCloned(origEntity, newEntity);
        }

        newEntity->SetLocalTransform(item.originalTransform);

        DAVA::Scene* scene = origEntity->GetScene();
        if (scene != nullptr)
        {
            DAVA::StaticOcclusionSystem* occlusionSystem = scene->staticOcclusionSystem;
            DVASSERT(occlusionSystem);
            occlusionSystem->InvalidateOcclusionIndicesRecursively(newEntity);
        }

        origEntity->GetParent()->AddNode(newEntity);
        clonedEntities.push_back(newEntity);
    }
}

void EntityModificationSystem::CloneEnd()
{
    if (modifEntities.size() > 0 && clonedEntities.size() == modifEntities.size())
    {
        SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());

        DAVA::uint32 count = static_cast<DAVA::uint32>(modifEntities.size());
        sceneEditor->BeginBatch("Clone", count);

        // we just moved original objects. Now we should return them back
        // to there original positions and move cloned object to the new positions
        // and only after that perform "add cloned entities to scene" commands
        for (DAVA::uint32 i = 0; i < count; ++i)
        {
            // remember new transform
            DAVA::Matrix4 newLocalTransform = modifEntities[i].object.GetLocalTransform();

            // return original entity to original pos
            modifEntities[i].object.SetLocalTransform(modifEntities[i].originalTransform);

            // move cloned entity to new pos
            clonedEntities[i]->SetLocalTransform(newLocalTransform);

            // remove entity from scene
            DAVA::Entity* cloneParent = clonedEntities[i]->GetParent();
            if (cloneParent)
            {
                cloneParent->RemoveNode(clonedEntities[i]);

                // and add it once again with command
                sceneEditor->Exec(Command2::Create<EntityAddCommand>(clonedEntities[i], cloneParent));
            }

            // make cloned entity selected
            SafeRelease(clonedEntities[i]);
        }

        sceneEditor->EndBatch();
    }

    clonedEntities.clear();
}

void EntityModificationSystem::RemoveEntity(DAVA::Entity* entity)
{
    if (GetLandscape(entity) != NULL)
    {
        SetLandscapeSnap(false);

        SceneEditor2* sceneEditor = ((SceneEditor2*)GetScene());
        SceneSignals::Instance()->EmitSnapToLandscapeChanged(sceneEditor, false);
    }
}

void EntityModificationSystem::MovePivotZero(const SelectableGroup& entities)
{
    if (ModifCanStart(entities))
    {
        BakeGeometry(entities, BAKE_ZERO_PIVOT);
    }
}

void EntityModificationSystem::MovePivotCenter(const SelectableGroup& entities)
{
    if (ModifCanStart(entities))
    {
        BakeGeometry(entities, BAKE_CENTER_PIVOT);
    }
}

void EntityModificationSystem::LockTransform(const SelectableGroup& entities, bool lock)
{
    SceneEditor2* sceneEditor = ((SceneEditor2*)GetScene());
    if (sceneEditor == nullptr)
    {
        return;
    }

    DAVA::uint32 count = static_cast<DAVA::uint32>(entities.GetSize());
    sceneEditor->BeginBatch("Lock entities", count);
    for (DAVA::Entity* entity : entities.ObjectsOfType<DAVA::Entity>())
    {
        sceneEditor->Exec(Command2::Create<EntityLockCommand>(entity, lock));
    }
    sceneEditor->EndBatch();
}

void EntityModificationSystem::BakeGeometry(const SelectableGroup& entities, BakeMode mode)
{
    SceneEditor2* sceneEditor = ((SceneEditor2*)GetScene());
    if ((sceneEditor == nullptr) && (entities.GetSize() != 1))
        return;

    DAVA::Entity* entity = entities.GetFirst().AsEntity();
    if (entity == nullptr)
        return;

    const char* commandMessage = nullptr;
    switch (mode)
    {
    case BAKE_ZERO_PIVOT:
        commandMessage = "Move pivot point to zero";
        break;
    case BAKE_CENTER_PIVOT:
        commandMessage = "Move pivot point to center";
        break;
    default:
        DVASSERT_MSG(0, "Unknown bake mode");
        return;
    }

    DAVA::RenderObject* ro = GetRenderObject(entity);
    if (ro != nullptr)
    {
        DAVA::Set<DAVA::Entity*> entityList;
        SearchEntitiesWithRenderObject(ro, sceneEditor, entityList);

        if (entityList.size() > 0)
        {
            DAVA::Matrix4 bakeTransform;

            switch (mode)
            {
            case BAKE_ZERO_PIVOT:
                bakeTransform = entity->GetLocalTransform();
                break;
            case BAKE_CENTER_PIVOT:
                bakeTransform.SetTranslationVector(-ro->GetBoundingBox().GetCenter());
                break;
            }

            sceneEditor->BeginBatch(commandMessage, entityList.size());

            // bake render object
            sceneEditor->Exec(Command2::Create<BakeGeometryCommand>(ro, bakeTransform));

            // inverse bake to be able to move object on same place
            // after it geometry was baked
            DAVA::Matrix4 afterBakeTransform = bakeTransform;
            afterBakeTransform.Inverse();

            // for entities with same render object set new transform
            // to make them match their previous position
            DAVA::Set<DAVA::Entity*>::iterator it;
            for (it = entityList.begin(); it != entityList.end(); ++it)
            {
                DAVA::Entity* en = *it;
                DAVA::Matrix4 origTransform = en->GetLocalTransform();
                DAVA::Matrix4 newTransform = afterBakeTransform * origTransform;
                sceneEditor->Exec(Command2::Create<TransformCommand>(Selectable(en), origTransform, newTransform));

                // also modify childs transform to make them be at
                // right position after parent entity changed
                for (DAVA::int32 i = 0; i < en->GetChildrenCount(); ++i)
                {
                    DAVA::Entity* childEntity = en->GetChild(i);

                    DAVA::Matrix4 childOrigTransform = childEntity->GetLocalTransform();
                    DAVA::Matrix4 childNewTransform = childOrigTransform * bakeTransform;

                    sceneEditor->Exec(Command2::Create<TransformCommand>(Selectable(childEntity), childOrigTransform, childNewTransform));
                }
            }

            sceneEditor->EndBatch();
        }
    }
    else if (entity->GetChildrenCount() > 0) // just modify child entities
    {
        DAVA::Vector3 newPivotPos = DAVA::Vector3(0, 0, 0);
        SceneSelectionSystem* selectionSystem = ((SceneEditor2*)GetScene())->selectionSystem;

        if (mode == BAKE_CENTER_PIVOT)
        {
            newPivotPos = selectionSystem->GetUntransformedBoundingBox(entity).GetCenter();
        }

        DAVA::uint32 count = static_cast<DAVA::uint32>(entity->GetChildrenCount());
        sceneEditor->BeginBatch(commandMessage, count + 1);

        // transform parent entity
        DAVA::Matrix4 transform;
        transform.SetTranslationVector(newPivotPos - entity->GetLocalTransform().GetTranslationVector());
        sceneEditor->Exec(Command2::Create<TransformCommand>(Selectable(entity), entity->GetLocalTransform(), entity->GetLocalTransform() * transform));

        // transform child entities with inversed parent transformation
        transform.Inverse();
        for (DAVA::uint32 i = 0; i < count; ++i)
        {
            DAVA::Entity* childEntity = entity->GetChild(i);
            sceneEditor->Exec(Command2::Create<TransformCommand>(Selectable(childEntity), childEntity->GetLocalTransform(), childEntity->GetLocalTransform() * transform));
        }

        sceneEditor->EndBatch();
    }
}

void EntityModificationSystem::SearchEntitiesWithRenderObject(DAVA::RenderObject* ro, DAVA::Entity* root, DAVA::Set<DAVA::Entity*>& result)
{
    if (NULL != root)
    {
        DAVA::int32 count = root->GetChildrenCount();
        for (DAVA::int32 i = 0; i < count; ++i)
        {
            DAVA::Entity* en = root->GetChild(i);
            DAVA::RenderObject* enRenderObject = GetRenderObject(en);

            bool isSame = false;
            if (NULL != enRenderObject && ro->GetRenderBatchCount() == enRenderObject->GetRenderBatchCount())
            {
                // if renderObjects has same number of render batches we also should
                // check if polygon groups used inside that render batches are completely identical
                // but we should deal with the fact, that polygon groups order can differ
                for (DAVA::uint32 j = 0; j < enRenderObject->GetRenderBatchCount(); ++j)
                {
                    bool found = false;
                    DAVA::PolygonGroup* pg = enRenderObject->GetRenderBatch(j)->GetPolygonGroup();

                    for (size_t k = 0; k < ro->GetRenderBatchCount(); ++k)
                    {
                        if (ro->GetRenderBatch(k)->GetPolygonGroup() == pg)
                        {
                            found = true;
                            break;
                        }
                    }

                    isSame = found;
                    if (!found)
                    {
                        break;
                    }
                }
            }

            if (isSame)
            {
                result.insert(en);
            }
            else if (en->GetChildrenCount() > 0)
            {
                SearchEntitiesWithRenderObject(ro, en, result);
            }
        }
    }
}

bool EntityModificationSystem::AllowPerformSelectionHavingCurrent(const SelectableGroup& currentSelection)
{
    return (transformType == Selectable::TransformType::Disabled) || !ModifCanStartByMouse(currentSelection);
}

bool EntityModificationSystem::AllowChangeSelectionReplacingCurrent(const SelectableGroup& currentSelection, const SelectableGroup& newSelection)
{
    return true;
}

void EntityModificationSystem::ApplyMoveValues(ST_Axis axis, const SelectableGroup& selection, const DAVA::Vector3& values, bool absoluteTransform)
{
    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
    sceneEditor->BeginBatch("Multiple move", selection.GetSize());

    for (const Selectable& item : selection.GetContent())
    {
        DAVA::Matrix4 origMatrix = item.GetLocalTransform();
        DAVA::Vector3 newPos = origMatrix.GetTranslationVector();

        if (axis & ST_AXIS_X)
        {
            newPos.x = absoluteTransform ? values.x : newPos.x + values.x;
        }

        if (axis & ST_AXIS_Y)
        {
            newPos.y = absoluteTransform ? values.y : newPos.y + values.y;
        }

        if ((axis & ST_AXIS_Z) && !snapToLandscape)
        {
            newPos.z = absoluteTransform ? values.z : newPos.z + values.z;
        }

        DAVA::Matrix4 newMatrix = origMatrix;
        newMatrix.SetTranslationVector(newPos);
        sceneEditor->Exec(Command2::Create<TransformCommand>(item, origMatrix, newMatrix));
    }
    sceneEditor->EndBatch();
}

void EntityModificationSystem::ApplyRotateValues(ST_Axis axis, const SelectableGroup& selection, const DAVA::Vector3& values, bool absoluteTransform)
{
    DAVA::float32 x = DAVA::DegToRad(values.x);
    DAVA::float32 y = DAVA::DegToRad(values.y);
    DAVA::float32 z = DAVA::DegToRad(values.z);

    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
    sceneEditor->BeginBatch("Multiple rotate", selection.GetSize());

    for (const Selectable& item : selection.GetContent())
    {
        DAVA::Matrix4 origMatrix = item.GetLocalTransform();

        DAVA::Vector3 pos, scale, rotate;
        if (origMatrix.Decomposition(pos, scale, rotate))
        {
            if (absoluteTransform == false)
            {
                rotate *= 0.0f;
            }

            DAVA::Matrix4 rotationMatrix;
            DAVA::Matrix4 moveToZeroPos;
            DAVA::Matrix4 moveFromZeroPos;
            moveToZeroPos.CreateTranslation(-origMatrix.GetTranslationVector());
            moveFromZeroPos.CreateTranslation(origMatrix.GetTranslationVector());

            switch (axis)
            {
            case ST_AXIS_X:
                rotationMatrix.CreateRotation(DAVA::Vector3(1, 0, 0), x - rotate.x);
                break;
            case ST_AXIS_Y:
                rotationMatrix.CreateRotation(DAVA::Vector3(0, 1, 0), y - rotate.y);
                break;
            case ST_AXIS_Z:
                rotationMatrix.CreateRotation(DAVA::Vector3(0, 0, 1), z - rotate.z);
                break;
            default:
                DVASSERT_MSG(0, "Unable to rotate around several axis at once");
                break;
            }

            DAVA::Matrix4 newMatrix = origMatrix * moveToZeroPos * rotationMatrix * moveFromZeroPos;
            newMatrix.SetTranslationVector(origMatrix.GetTranslationVector());

            sceneEditor->Exec(Command2::Create<TransformCommand>(item, origMatrix, newMatrix));
        }
    }

    sceneEditor->EndBatch();
}

void EntityModificationSystem::ApplyScaleValues(ST_Axis axis, const SelectableGroup& selection, const DAVA::Vector3& values, bool absoluteTransform)
{
    DAVA::float32 scaleValue = 1.0f;

    switch (axis)
    {
    case ST_AXIS_X:
        scaleValue = values.x;
        break;
    case ST_AXIS_Y:
        scaleValue = values.y;
        break;
    case ST_AXIS_Z:
        scaleValue = values.z;
        break;
    default:
        DVASSERT_MSG(0, "Scaling must be uniform, unable to scale via several axis");
        break;
    }

    SceneEditor2* sceneEditor = static_cast<SceneEditor2*>(GetScene());
    sceneEditor->BeginBatch("Multiple scale", selection.GetSize());

    for (const Selectable& item : selection.GetContent())
    {
        DAVA::Matrix4 origMatrix = item.GetLocalTransform();

        DAVA::Vector3 pos, scale, rotate;
        if (origMatrix.Decomposition(pos, scale, rotate))
        {
            if (absoluteTransform)
            {
                scaleValue = (scale.x < std::numeric_limits<float>::epsilon()) ? 0.0f : (scaleValue / scale.x);
            }

            DAVA::Matrix4 moveToZeroPos;
            moveToZeroPos.CreateTranslation(-origMatrix.GetTranslationVector());

            DAVA::Matrix4 moveFromZeroPos;
            moveFromZeroPos.CreateTranslation(origMatrix.GetTranslationVector());

            DAVA::Matrix4 scaleMatrix;
            scaleMatrix.CreateScale(DAVA::Vector3(scaleValue, scaleValue, scaleValue));

            DAVA::Matrix4 newMatrix = origMatrix * moveToZeroPos * scaleMatrix * moveFromZeroPos;
            newMatrix.SetTranslationVector(origMatrix.GetTranslationVector());

            sceneEditor->Exec(Command2::Create<TransformCommand>(item, origMatrix, newMatrix));
        }
    }

    sceneEditor->EndBatch();
}
