#include "Classes/ShadersModule/ShadersModule.h"
#include "Classes/Project/ProjectManagerData.h"
#include "Classes/SceneManager/SceneData.h"
#include "Classes/Qt/MaterialEditor/MaterialEditor.h"
#include "Classes/Qt/Scene/System/VisibilityCheckSystem/VisibilityCheckSystem.h"
#include "Classes/Qt/Scene/System/EditorMaterialSystem.h"
#include "Classes/Qt/Scene/SceneEditor2.h"
#include "Classes/Settings/SettingsManager.h"
#include "Classes/Settings/Settings.h"
#include "Classes/Application/REGlobal.h"

#include "TArc/WindowSubSystem/ActionUtils.h"
#include "TArc/WindowSubSystem/UI.h"
#include "TArc/WindowSubSystem/QtAction.h"
#include "TArc/Utils/ModuleCollection.h"

#include "Functional/Function.h"
#include "FileSystem/FilePath.h"
#include "Logger/Logger.h"
#include "Reflection/ReflectionRegistrator.h"
#include "Render/2D/Systems/RenderSystem2D.h"
#include "Render/Highlevel/RenderSystem.h"
#include "Render/Material/NMaterial.h"
#include "Render/ShaderCache.h"
#include "Scene3D/Systems/FoliageSystem.h"
#include "Scene3D/Systems/ParticleEffectDebugDrawSystem.h"

namespace ShadersModuleDetail
{
#if defined(LOCAL_FRAMEWORK_SOURCE_PATH)
DAVA::FilePath GetDevMaterialsPath()
{
    DAVA::FilePath devShadersPath(LOCAL_FRAMEWORK_SOURCE_PATH);
    devShadersPath += "/Programs/Data/";
    return devShadersPath;
}
#endif
}

void ShadersModule::PostInit()
{
    using namespace DAVA::TArc;

    QtAction* reloadShadersAction = new QtAction(GetAccessor(), QIcon(":/QtIcons/reload_shaders.png"), QString("Reload Shaders"));

    FieldDescriptor fieldDescriptor(DAVA::ReflectedTypeDB::Get<ProjectManagerData>(), DAVA::FastName(ProjectManagerData::ProjectPathProperty));
    reloadShadersAction->SetStateUpdationFunction(QtAction::Enabled, fieldDescriptor, [](const DAVA::Any& fieldValue) -> DAVA::Any
                                                  {
                                                      return fieldValue.CanCast<DAVA::FilePath>() && !fieldValue.Cast<DAVA::FilePath>().IsEmpty();
                                                  });

    ActionPlacementInfo menuPlacement(CreateMenuPoint("Scene", InsertionParams(InsertionParams::eInsertionMethod::AfterItem, "actionEnableCameraLight")));
    GetUI()->AddAction(DAVA::TArc::mainWindowKey, menuPlacement, reloadShadersAction);

    ActionPlacementInfo toolbarPlacement(CreateToolbarPoint("sceneToolBar", InsertionParams(InsertionParams::eInsertionMethod::AfterItem, "Reload Sprites")));
    GetUI()->AddAction(DAVA::TArc::mainWindowKey, toolbarPlacement, reloadShadersAction);

    connections.AddConnection(reloadShadersAction, &QAction::triggered, DAVA::MakeFunction(this, &ShadersModule::ReloadShaders));

    fieldBinder.reset(new DAVA::TArc::FieldBinder(GetAccessor()));
    fieldBinder->BindField(fieldDescriptor, DAVA::MakeFunction(this, &ShadersModule::OnProjectChanged));
}

void ShadersModule::ReloadShaders()
{
    DAVA::ShaderDescriptorCache::ReloadShaders();

    GetAccessor()->ForEachContext([](DAVA::TArc::DataContext& ctx)
                                  {
                                      SceneData* sceneData = ctx.GetData<SceneData>();
                                      DAVA::RefPtr<SceneEditor2> sceneEditor = sceneData->GetScene();

                                      const DAVA::Set<DAVA::NMaterial*>& topParents = sceneEditor->materialSystem->GetTopParents();

                                      for (auto material : topParents)
                                      {
                                          material->InvalidateRenderVariants();
                                      }
                                      const DAVA::Map<DAVA::uint64, DAVA::NMaterial*>& particleInstances = sceneEditor->particleEffectSystem->GetMaterialInstances();
                                      for (auto material : particleInstances)
                                      {
                                          material.second->InvalidateRenderVariants();
                                      }

                                      DAVA::ParticleEffectDebugDrawSystem* particleEffectDebugDrawSystem = sceneEditor->GetParticleEffectDebugDrawSystem();
                                      if (particleEffectDebugDrawSystem != nullptr)
                                      {
                                          const DAVA::Vector<DAVA::NMaterial*>* const particleDebug = particleEffectDebugDrawSystem->GetMaterials();
                                          for (auto material : *particleDebug)
                                          {
                                              material->InvalidateRenderVariants();
                                          }
                                      }

                                      DAVA::Set<DAVA::NMaterial*> materialList;
                                      sceneEditor->foliageSystem->CollectFoliageMaterials(materialList);
                                      for (auto material : materialList)
                                      {
                                          if (material)
                                              material->InvalidateRenderVariants();
                                      }

                                      sceneEditor->renderSystem->GetDebugDrawer()->InvalidateMaterials();
                                      sceneEditor->renderSystem->SetForceUpdateLights();

                                      sceneEditor->visibilityCheckSystem->InvalidateMaterials();
                                  });
    
#define INVALIDATE_2D_MATERIAL(material) \
    if (DAVA::RenderSystem2D::material) \
    { \
        DAVA::RenderSystem2D::material->InvalidateRenderVariants(); \
        DAVA::RenderSystem2D::material->PreBuildMaterial(DAVA::RenderSystem2D::RENDER_PASS_NAME); \
    }

    INVALIDATE_2D_MATERIAL(DEFAULT_2D_COLOR_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_TEXTURE_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_TEXTURE_NOBLEND_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_TEXTURE_ALPHA8_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_TEXTURE_GRAYSCALE_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_FILL_ALPHA_MATERIAL)
    INVALIDATE_2D_MATERIAL(DEFAULT_2D_TEXTURE_ADDITIVE_MATERIAL)
    
#undef INVALIDATE_2D_MATERIAL

    if (MaterialEditor::Instance() != nullptr)
    {
        MaterialEditor::Instance()->RefreshMaterialProperties();
    }
}

void ShadersModule::OnProjectChanged(const DAVA::Any& projectFieldValue)
{
    DAVA::FilePath newProjectPathname;
    if (projectFieldValue.CanGet<DAVA::FilePath>())
    {
        newProjectPathname = projectFieldValue.Get<DAVA::FilePath>();
    }

    if (newProjectPathname.IsEmpty())
    {
#if defined(LOCAL_FRAMEWORK_SOURCE_PATH)
        DAVA::FilePath::RemoveResourcesFolder(ShadersModuleDetail::GetDevMaterialsPath());
#endif
    }
    else
    {
#if defined(LOCAL_FRAMEWORK_SOURCE_PATH)
        DAVA::FilePath::AddResourcesFolder(ShadersModuleDetail::GetDevMaterialsPath());
#endif
        ReloadShaders();
    }
}

DAVA_VIRTUAL_REFLECTION_IMPL(ShadersModule)
{
    DAVA::ReflectionRegistrator<ShadersModule>::Begin()
    .ConstructorByPointer()
    .End();
}

DECL_GUI_MODULE(ShadersModule);
