#include "ReflectionDeclaration/ReflectionDeclaration.h"
#include "ReflectionDeclaration/Private/AnyCasts.h"

#include "Reflection/Reflection.h"
#include "Reflection/ReflectionRegistrator.h"

#include "Engine/Engine.h"
#include "Scene3D/Entity.h"
#include "Scene3D/Components/Controller/RotationControllerComponent.h"
#include "Scene3D/Components/VisibilityCheckComponent.h"
#include "Scene3D/Components/Controller/SnapToLandscapeControllerComponent.h"
#include "Scene3D/Components/Controller/WASDControllerComponent.h"
#include "Scene3D/Components/Waypoint/EdgeComponent.h"
#include "Scene3D/Components/Waypoint/PathComponent.h"
#include "Scene3D/Components/Waypoint/WaypointComponent.h"
#include "Scene3D/Components/ActionComponent.h"
#include "Scene3D/Components/AnimationComponent.h"
#include "Scene3D/Components/BulletComponent.h"
#include "Scene3D/Components/CameraComponent.h"
#include "Scene3D/Components/CustomPropertiesComponent.h"
#include "Scene3D/Components/DebugRenderComponent.h"
#include "Scene3D/Components/LightComponent.h"
#include "Scene3D/Components/ParticleEffectComponent.h"
#include "Scene3D/Components/QualitySettingsComponent.h"
#include "Scene3D/Components/RenderComponent.h"
#include "Scene3D/Components/SkeletonComponent.h"
#include "Scene3D/Components/SoundComponent.h"
#include "Scene3D/Components/SpeedTreeComponent.h"
#include "Scene3D/Components/StaticOcclusionComponent.h"
#include "Scene3D/Components/SwitchComponent.h"
#include "Scene3D/Components/TransformComponent.h"
#include "Scene3D/Components/UpdatableComponent.h"
#include "Scene3D/Components/UserComponent.h"
#include "Scene3D/Components/WaveComponent.h"
#include "Scene3D/Components/WindComponent.h"
#include "Scene3D/Lod/LodComponent.h"
#include "Entity/Component.h"
#include "Render/3D/PolygonGroup.h"
#include "Render/Highlevel/LandscapeSubdivision.h"
#include "Render/Highlevel/RenderObject.h"
#include "Render/Highlevel/RenderBatch.h"
#include "Render/Highlevel/Vegetation/VegetationRenderObject.h"
#include "Render/Highlevel/BillboardRenderObject.h"
#include "Render/Highlevel/Heightmap.h"
#include "Render/Highlevel/Landscape.h"
#include "Render/Highlevel/Light.h"
#include "Render/Highlevel/SpeedTreeObject.h"
#include "Math/Vector.h"
#include "Math/Rect.h"
#include "Math/AABBox3.h"
#include "Math/Color.h"

#include "Debug/Private/ImGui.h"
#include "Utils/StringFormat.h"

namespace DAVA
{
namespace ReflectionDeclarationDetail
{
float32 GetMinX(AABBox3* box)
{
    return box->min.x;
}

void SetMinX(AABBox3* box, float32 v)
{
    box->min.x = v;
}

float32 GetMinY(AABBox3* box)
{
    return box->min.y;
}

void SetMinY(AABBox3* box, float32 v)
{
    box->min.y = v;
}

float32 GetMinZ(AABBox3* box)
{
    return box->min.z;
}

void SetMinZ(AABBox3* box, float32 v)
{
    box->min.z = v;
}

float32 GetMaxX(AABBox3* box)
{
    return box->max.x;
}

void SetMaxX(AABBox3* box, float32 v)
{
    box->max.x = v;
}

float32 GetMaxY(AABBox3* box)
{
    return box->max.y;
}

void SetMaxY(AABBox3* box, float32 v)
{
    box->max.y = v;
}

float32 GetMaxZ(AABBox3* box)
{
    return box->max.z;
}

void SetMaxZ(AABBox3* box, float32 v)
{
    box->max.z = v;
}
}

void RegisterVector2()
{
    ReflectionRegistrator<Vector2>::Begin()
    .Field("X", &Vector2::x)[M::SubProperty()]
    .Field("Y", &Vector2::y)[M::SubProperty()]
    .End();
}

void RegisterVector3()
{
    ReflectionRegistrator<Vector3>::Begin()
    .Field("X", &Vector3::x)[M::SubProperty()]
    .Field("Y", &Vector3::y)[M::SubProperty()]
    .Field("Z", &Vector3::z)[M::SubProperty()]
    .End();
}

void RegisterVector4()
{
    ReflectionRegistrator<Vector4>::Begin()
    .Field("X", &Vector4::x)[M::SubProperty()]
    .Field("Y", &Vector4::y)[M::SubProperty()]
    .Field("Z", &Vector4::z)[M::SubProperty()]
    .Field("W", &Vector4::w)[M::SubProperty()]
    .End();
}

void RegisterRect()
{
    ReflectionRegistrator<Rect>::Begin()
    .Field("X", &Rect::x)[M::SubProperty()]
    .Field("Y", &Rect::y)[M::SubProperty()]
    .Field("Width", &Rect::dx)[M::SubProperty()]
    .Field("Height", &Rect::dy)[M::SubProperty()]
    .End();
}

void RegisterAABBox3()
{
    ReflectionRegistrator<AABBox3>::Begin()
    .Field("MinX", &ReflectionDeclarationDetail::GetMinX, &ReflectionDeclarationDetail::SetMinX)[M::SubProperty()]
    .Field("MinY", &ReflectionDeclarationDetail::GetMinY, &ReflectionDeclarationDetail::SetMinY)[M::SubProperty()]
    .Field("MinZ", &ReflectionDeclarationDetail::GetMinZ, &ReflectionDeclarationDetail::SetMinZ)[M::SubProperty()]
    .Field("MaxX", &ReflectionDeclarationDetail::GetMaxX, &ReflectionDeclarationDetail::SetMaxX)[M::SubProperty()]
    .Field("MaxY", &ReflectionDeclarationDetail::GetMaxY, &ReflectionDeclarationDetail::SetMaxY)[M::SubProperty()]
    .Field("MaxZ", &ReflectionDeclarationDetail::GetMaxZ, &ReflectionDeclarationDetail::SetMaxZ)[M::SubProperty()]
    .End();
}

void RegisterColor()
{
    ReflectionRegistrator<Color>::Begin()
    .Field("R", &Color::r)
    .Field("G", &Color::g)
    .Field("B", &Color::b)
    .Field("A", &Color::a)
    .End();
}

void RegisterPermanentNames()
{
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Component);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RotationControllerComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SnapToLandscapeControllerComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WASDControllerComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(EdgeComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PathComponent);
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(PathComponent::Waypoint, "Waypoint");
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(PathComponent::Edge, "Edge");
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WaypointComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ActionComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(AnimationComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(BulletComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(CameraComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(CustomPropertiesComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(DebugRenderComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(LightComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(ParticleEffectComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(QualitySettingsComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RenderComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SkeletonComponent);
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(SkeletonComponent::JointConfig, "JointConfig");
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SoundComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SoundComponentElement);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SpeedTreeComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(StaticOcclusionDataComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(StaticOcclusionComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(StaticOcclusionDebugDrawComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SwitchComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(TransformComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UpdatableComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(UserComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VisibilityCheckComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WaveComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(WindComponent);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(LodComponent);
    DAVA_REFLECTION_REGISTER_CUSTOM_PERMANENT_NAME(ActionComponent::Action, "Action");
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(PolygonGroup);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RenderObject);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RenderObject::IndexedRenderBatch);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(LandscapeSubdivision);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(LandscapeSubdivision::SubdivisionMetrics);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(RenderBatch);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(VegetationRenderObject);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(BillboardRenderObject);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Heightmap);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Landscape);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Light);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(SpeedTreeObject);
    DAVA_REFLECTION_REGISTER_PERMANENT_NAME(Entity);
}

void ProvideReflectionDebugInfo()
{
    GetPrimaryWindow()->update.Connect([](Window*, float32) {
        if (ImGui::IsInitialized())
        {
            static int updateFrame = 0;
            static TypeDetail::TypeDB::Stats typeStats;
            static ReflectedTypeDB::Stats reflectionStats;

            if (updateFrame > 0)
            {
                updateFrame--;
            }
            else
            {
                updateFrame = 120; // once per 120 frames
                typeStats = TypeDetail::TypeDB::GetStats();
                reflectionStats = ReflectedTypeDB::GetStats();
            }

            auto addRow = [](const char* name, const char* format, size_t value)
            {
                ImGui::Text(name);
                ImGui::NextColumn();
                ImGui::Text(Format(format, value).c_str());
                ImGui::NextColumn();
            };

            ImGui::Begin("Reflection stats");
            ImGui::Columns(2, nullptr, true);

            addRow("typesCount", "%u", typeStats.typesCount);
            addRow("typesMemory", "%u bytes", typeStats.typesMemory);
            addRow("typeInheritanceCount", "%u", typeStats.typeInheritanceCount);
            addRow("typeInheritanceInfoCount", "%u", typeStats.typeInheritanceInfoCount);
            addRow("typeInheritanceMemory", "%u bytes", typeStats.typeInheritanceMemory);
            addRow("typeDBMemory", "%u bytes", typeStats.typeDBMemory);

            addRow("reflectedTypeCount", "%u", reflectionStats.reflectedTypeCount);
            addRow("reflectedTypeMemory", "%u", reflectionStats.reflectedTypeMemory);
            addRow("reflectedTypeDBMemory", "%u bytes", reflectionStats.reflectedTypeDBMemory);
            addRow("reflectedStructCount", "%u", reflectionStats.reflectedStructCount);
            addRow("reflectedStructWrapperCount", "%u", reflectionStats.reflectedStructWrapperCount);
            addRow("reflectedStructWrapperClassCount", "%u", reflectionStats.reflectedStructWrapperClassCount);
            addRow("reflectedStructWrapperClassMemory", "%u bytes", reflectionStats.reflectedStructWrapperClassMemory);
            addRow("reflectedStructFieldsCount", "%u", reflectionStats.reflectedStructFieldsCount);
            addRow("reflectedStructMethodsCount", "%u", reflectionStats.reflectedStructMethodsCount);
            addRow("reflectedStructEnumsCount", "%u", reflectionStats.reflectedStructEnumsCount);
            addRow("reflectedStructCtorsCount", "%u", reflectionStats.reflectedStructCtorsCount);
            addRow("reflectedStructDtorsCount", "%u", reflectionStats.reflectedStructDtorsCount);
            addRow("reflectedStructMetasCount", "%u", reflectionStats.reflectedStructMetasCount);
            addRow("reflectedStructMetaMCount", "%u", reflectionStats.reflectedStructMetaMCount);
            addRow("reflectedStructMemory", "%u bytes", reflectionStats.reflectedStructMemory);

            addRow("total", "%u bytes", typeStats.totalMemory + reflectionStats.totalMemory);

            ImGui::Columns();
            ImGui::End();
        }
    });
}

void RegisterReflectionForBaseTypes()
{
    RegisterAnyCasts();

    RegisterVector2();
    RegisterVector3();
    RegisterVector4();
    RegisterRect();
    RegisterAABBox3();
    RegisterColor();

    RegisterPermanentNames();

#ifdef __DAVAENGINE_DEBUG__
    ProvideReflectionDebugInfo();
#endif
}
} // namespace DAVA