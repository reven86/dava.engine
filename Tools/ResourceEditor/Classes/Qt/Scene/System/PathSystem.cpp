#include "PathSystem.h"

#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Components/Waypoint/WaypointComponent.h"
#include "Scene3D/Components/Waypoint/EdgeComponent.h"
#include "Scene3D/Components/ComponentHelpers.h"

#include "Commands2/ConvertPathCommands.h"

#include "FileSystem/KeyedArchive.h"
#include "Scene3D/Entity.h"

#include "Utils/Utils.h"

#include "Scene/SceneEditor2.h"
#include "Commands2/InspMemberModifyCommand.h"
#include "Commands2/WayEditCommands.h"

namespace PathSystemInternal
{
const DAVA::Array<DAVA::Color, 16> PathColorPallete =
{ { DAVA::Color(0x00ffffff),
    DAVA::Color(0x000000ff),
    DAVA::Color(0x0000ffff),
    DAVA::Color(0xff00ffff),

    DAVA::Color(0x808080ff),
    DAVA::Color(0x008000ff),
    DAVA::Color(0x00ff00ff),
    DAVA::Color(0x80000ff),

    DAVA::Color(0x000080ff),
    DAVA::Color(0x808000ff),
    DAVA::Color(0x800080ff),
    DAVA::Color(0xff0000ff),

    DAVA::Color(0xc0c0c0ff),
    DAVA::Color(0x008080ff),
    DAVA::Color(0xffffffff),
    DAVA::Color(0xffff00ff) } };

const DAVA::String PATH_COLOR_PROP_NAME = "pathColor";
}

PathSystem::PathSystem(DAVA::Scene* scene)
    : DAVA::SceneSystem(scene)
    , currentPath(NULL)
    , isEditingEnabled(false)
{
    sceneEditor = static_cast<SceneEditor2*>(GetScene());
}

PathSystem::~PathSystem()
{
    currentPath = NULL;

    pathes.clear();
    currentSelection.Clear();
}

void PathSystem::AddPath(DAVA::Entity* entity)
{
    sceneEditor->BeginBatch("Add path at scene", 1);
    sceneEditor->Exec(Command2::Create<EntityAddCommand>(entity, sceneEditor));

    if (isEditingEnabled)
        ExpandPathEntity(entity);

    sceneEditor->EndBatch();
}

void PathSystem::AddEntity(DAVA::Entity* entity)
{
    pathes.push_back(entity);

    if (!currentPath)
    {
        currentPath = entity;
    }

    // extract color data from custom properties for old scenes
    DAVA::PathComponent* pc = GetPathComponent(entity);
    if (pc && pc->GetColor() == DAVA::Color())
    {
        DAVA::KeyedArchive* props = GetCustomPropertiesArchieve(entity);
        if (props && props->IsKeyExists(PathSystemInternal::PATH_COLOR_PROP_NAME))
        {
            pc->SetColor(DAVA::Color(props->GetVector4(PathSystemInternal::PATH_COLOR_PROP_NAME)));
            props->DeleteKey(PathSystemInternal::PATH_COLOR_PROP_NAME);
        }
    }
}

void PathSystem::RemoveEntity(DAVA::Entity* entity)
{
    DAVA::FindAndRemoveExchangingWithLast(pathes, entity);

    if (pathes.size())
    {
        if (entity == currentPath)
        {
            currentPath = pathes[0];
        }
    }
    else
    {
        currentPath = nullptr;
    }
}

void PathSystem::WillClone(DAVA::Entity* originalEntity)
{
    if (isEditingEnabled && GetPathComponent(originalEntity) != nullptr)
    {
        CollapsePathEntity(originalEntity);
    }
}

void PathSystem::DidCloned(DAVA::Entity* originalEntity, DAVA::Entity* newEntity)
{
    if (isEditingEnabled)
    {
        if (GetPathComponent(originalEntity) != nullptr)
        {
            ExpandPathEntity(originalEntity);
        }

        if (GetPathComponent(newEntity) != nullptr)
        {
            ExpandPathEntity(newEntity);
        }
    }
}

void PathSystem::Draw()
{
    const DAVA::uint32 count = pathes.size();
    if (!count)
        return;

    if (isEditingEnabled)
    {
        DrawInEditableMode();
    }
    else
    {
        DrawInViewOnlyMode();
    }
}

void PathSystem::DrawInEditableMode()
{
    for (DAVA::Entity* path : pathes)
    {
        DAVA::PathComponent* pc = GetPathComponent(path);
        if (!path->GetVisible() || !pc)
        {
            continue;
        }

        const DAVA::uint32 childrenCount = path->GetChildrenCount();
        for (DAVA::uint32 c = 0; c < childrenCount; ++c)
        {
            DAVA::Entity* waypoint = path->GetChild(c);

            const DAVA::uint32 edgesCount = waypoint->GetComponentCount(DAVA::Component::EDGE_COMPONENT);
            if (edgesCount)
            {
                DAVA::Vector3 startPosition = GetTransformComponent(waypoint)->GetWorldTransform().GetTranslationVector();
                startPosition.z += WAYPOINTS_DRAW_LIFTING;
                for (DAVA::uint32 e = 0; e < edgesCount; ++e)
                {
                    DAVA::EdgeComponent* edge = static_cast<DAVA::EdgeComponent*>(waypoint->GetComponent(DAVA::Component::EDGE_COMPONENT, e));
                    DAVA::Entity* nextEntity = edge->GetNextEntity();
                    if (nextEntity && nextEntity->GetParent())
                    {
                        DAVA::Vector3 finishPosition = GetTransformComponent(nextEntity)->GetWorldTransform().GetTranslationVector();
                        finishPosition.z += WAYPOINTS_DRAW_LIFTING;
                        DrawArrow(startPosition, finishPosition, pc->GetColor());
                    }
                }
            }
        }
    }
}

void PathSystem::DrawInViewOnlyMode()
{
    const DAVA::float32 boxScale = SettingsManager::GetValue(Settings::Scene_DebugBoxWaypointScale).AsFloat();

    const SelectableGroup& selection = sceneEditor->selectionSystem->GetSelection();

    for (auto entity : selection.ObjectsOfType<DAVA::Entity>())
    {
        DAVA::PathComponent* pathComponent = DAVA::GetPathComponent(entity);
        if (entity->GetVisible() == false || !pathComponent)
        {
            continue;
        }

        const DAVA::Vector<DAVA::PathComponent::Waypoint*>& waypoints = pathComponent->GetPoints();
        for (auto waypoint : waypoints)
        {
            DAVA::Vector3 startPosition = waypoint->position;
            const DAVA::AABBox3 wpBoundingBox(startPosition, boxScale);
            const auto& transform = entity->GetWorldTransform();
            bool isStarting = waypoint->IsStarting();

            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABoxTransformed(wpBoundingBox, transform, DAVA::Color(0.3f, 0.3f, isStarting ? 1.0f : 0.0f, 0.3f), DAVA::RenderHelper::DRAW_SOLID_DEPTH);
            GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawAABoxTransformed(wpBoundingBox, transform, DAVA::Color(0.7f, 0.7f, isStarting ? 0.7f : 0.0f, 1.0f), DAVA::RenderHelper::DRAW_WIRE_DEPTH);

            //draw edges
            if (!waypoint->edges.empty())
            {
                startPosition.z += WAYPOINTS_DRAW_LIFTING;
                for (auto edge : waypoint->edges)
                {
                    DAVA::Vector3 finishPosition = edge->destination->position;
                    finishPosition.z += WAYPOINTS_DRAW_LIFTING;
                    DrawArrow(startPosition * transform, finishPosition * transform, pathComponent->GetColor());
                }
            }
        }
    }
}

void PathSystem::DrawArrow(const DAVA::Vector3& start, const DAVA::Vector3& finish, const DAVA::Color& color)
{
    GetScene()->GetRenderSystem()->GetDebugDrawer()->DrawArrow(start, finish, DAVA::Min((finish - start).Length() / 4.f, 4.f), DAVA::ClampToUnityRange(color), DAVA::RenderHelper::DRAW_SOLID_DEPTH);
}

void PathSystem::Process(DAVA::float32 timeElapsed)
{
    const SelectableGroup& selection = sceneEditor->selectionSystem->GetSelection();
    if (currentSelection != selection)
    {
        currentSelection.Clear();
        currentSelection.Join(selection);

        for (auto entity : currentSelection.ObjectsOfType<DAVA::Entity>())
        {
            if (entity->GetComponent(DAVA::Component::PATH_COMPONENT) != nullptr)
            {
                currentPath = entity;
                break;
            }

            if (GetWaypointComponent(entity) && GetPathComponent(entity->GetParent()))
            {
                currentPath = entity->GetParent();
                break;
            }
        }
    }
}

void PathSystem::ProcessCommand(const Command2* command, bool redo)
{
    if (command->MatchCommandID(CMDID_ENABLE_WAYEDIT))
    {
        DVASSERT(command->MatchCommandID(CMDID_DISABLE_WAYEDIT) == false);
        isEditingEnabled = redo;
    }
    else if (command->MatchCommandID(CMDID_DISABLE_WAYEDIT))
    {
        DVASSERT(command->MatchCommandID(CMDID_ENABLE_WAYEDIT) == false);
        isEditingEnabled = !redo;
    }

    if (command->MatchCommandID(CMDID_INSP_MEMBER_MODIFY))
    {
        auto ProcessInspCommand = [this](const InspMemberModifyCommand* inspCommand, bool redo) {
            static const DAVA::FastName NAME("name");
            if (NAME == inspCommand->member->Name())
            {
                const DAVA::uint32 count = pathes.size();
                for (DAVA::uint32 p = 0; p < count; ++p)
                {
                    const DAVA::PathComponent* pc = DAVA::GetPathComponent(pathes[p]);
                    if (inspCommand->object == pc)
                    {
                        DAVA::FastName newPathName = (redo) ? inspCommand->newValue.AsFastName() : inspCommand->oldValue.AsFastName();
                        DAVA::FastName oldPathName = (redo) ? inspCommand->oldValue.AsFastName() : inspCommand->newValue.AsFastName();

                        const DAVA::uint32 childrenCount = pathes[p]->GetChildrenCount();
                        for (DAVA::uint32 c = 0; c < childrenCount; ++c)
                        {
                            DAVA::WaypointComponent* wp = GetWaypointComponent(pathes[p]->GetChild(c));
                            if (wp && wp->GetPathName() == oldPathName)
                            {
                                wp->SetPathName(newPathName);
                            }
                        }

                        break;
                    }
                }
            }
        };

        if (command->GetId() == CMDID_BATCH)
        {
            const CommandBatch* batch = static_cast<const CommandBatch*>(command);
            const DAVA::uint32 count = batch->Size();
            for (DAVA::uint32 i = 0; i < count; ++i)
            {
                const Command2* cmd = batch->GetCommand(i);
                if (cmd->MatchCommandID(CMDID_INSP_MEMBER_MODIFY))
                {
                    ProcessInspCommand(static_cast<const InspMemberModifyCommand*>(cmd), redo);
                }
            }
        }
        else
        {
            ProcessInspCommand(static_cast<const InspMemberModifyCommand*>(command), redo);
        }
    }
}

DAVA::FastName PathSystem::GeneratePathName() const
{
    const DAVA::uint32 count = pathes.size();

    for (DAVA::uint32 i = 0; i <= count; ++i)
    {
        DAVA::FastName generatedName(DAVA::Format("path_%02d", i));

        bool found = false;

        for (DAVA::uint32 p = 0; p < count; ++p)
        {
            const DAVA::PathComponent* pc = DAVA::GetPathComponent(pathes[p]);
            if (generatedName == pc->GetName())
            {
                found = true;
                break;
            }
        }

        if (!found)
            return generatedName;
    }

    return DAVA::FastName();
}

const DAVA::Color& PathSystem::GetNextPathColor() const
{
    const DAVA::uint32 count = pathes.size();
    const DAVA::uint32 index = count % PathSystemInternal::PathColorPallete.size();

    return PathSystemInternal::PathColorPallete[index];
}

void PathSystem::EnablePathEdit(bool enable)
{
    if (enable)
    {
        sceneEditor->BeginBatch("Enable waypoints edit", pathes.size() + 1);
        sceneEditor->Exec(Command2::Create<EnableWayEditCommand>());

        for (auto path : pathes)
        {
            ExpandPathEntity(path);
        }

        sceneEditor->EndBatch();
    }
    else
    {
        sceneEditor->BeginBatch("Disable waypoints edit", pathes.size() + 1);
        sceneEditor->Exec(Command2::Create<DisableWayEditCommand>());

        for (auto path : pathes)
        {
            CollapsePathEntity(path);
        }

        sceneEditor->EndBatch();
    }
}

void PathSystem::ExpandPathEntity(const DAVA::Entity* pathEntity)
{
    DAVA::uint32 pathComponentCount = pathEntity->GetComponentCount(DAVA::Component::PATH_COMPONENT);

    sceneEditor->BeginBatch("Expand path components", pathComponentCount);
    for (DAVA::uint32 i = 0; i < pathComponentCount; ++i)
    {
        DAVA::PathComponent* pathComponent = static_cast<DAVA::PathComponent*>(pathEntity->GetComponent(DAVA::Component::PATH_COMPONENT, i));
        DVASSERT(pathComponent);
        sceneEditor->Exec(Command2::Create<ExpandPathCommand>(pathComponent));
    }

    sceneEditor->EndBatch();
}

void PathSystem::CollapsePathEntity(const DAVA::Entity* pathEntity)
{
    DAVA::uint32 pathComponentCount = pathEntity->GetComponentCount(DAVA::Component::PATH_COMPONENT);
    sceneEditor->BeginBatch("Collapse path components", pathComponentCount);
    for (DAVA::uint32 i = 0; i < pathComponentCount; ++i)
    {
        DAVA::PathComponent* pathComponent = static_cast<DAVA::PathComponent*>(pathEntity->GetComponent(DAVA::Component::PATH_COMPONENT, i));
        DVASSERT(pathComponent);
        sceneEditor->Exec(Command2::Create<CollapsePathCommand>(pathComponent));
    }

    sceneEditor->EndBatch();
}

DAVA::PathComponent* PathSystem::CreatePathComponent()
{
    DAVA::PathComponent* pc = new DAVA::PathComponent();
    pc->SetName(GeneratePathName());
    pc->SetColor(GetNextPathColor());
    return pc;
}
