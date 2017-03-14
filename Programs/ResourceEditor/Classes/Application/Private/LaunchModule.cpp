#include "Classes/Application/LaunchModule.h"
#include "Classes/Application/REGlobal.h"
#include "Classes/Qt/Scene/BaseTransformProxies.h"
#include "Classes/Qt/Scene/BaseTransformProxies.h"
#include "Classes/Selection/Selectable.h"

#include "Classes/Project/ProjectManagerData.h"
#include "Classes/Settings/SettingsManager.h"
#include "Classes/StringConstants.h"

#include <Tools/Version.h>

#include "TArc/DataProcessing/DataListener.h"
#include "TArc/DataProcessing/DataWrapper.h"
#include "TArc/Utils/ModuleCollection.h"

#include "Particles/ParticleEmitterInstance.h"
#include "Engine/EngineContext.h"
#include "FileSystem/ResourceArchive.h"
#include "FileSystem/FileSystem.h"

class LaunchModule::FirstSceneCreator : public QObject, private DAVA::TArc::DataListener
{
public:
    FirstSceneCreator(LaunchModule* module_)
        : module(module_)
    {
        DAVA::TArc::ContextAccessor* accessor = module->GetAccessor();
        wrapper = accessor->CreateWrapper(DAVA::ReflectedTypeDB::Get<ProjectManagerData>());
        wrapper.SetListener(this);
    }

    void OnDataChanged(const DAVA::TArc::DataWrapper& w, const DAVA::Vector<DAVA::Any>& fields) override
    {
        if (!wrapper.HasData())
        {
            return;
        }

        DAVA::TArc::ContextAccessor* accessor = module->GetAccessor();
        ProjectManagerData* data = accessor->GetGlobalContext()->GetData<ProjectManagerData>();
        if (data->IsOpened())
        {
            module->InvokeOperation(REGlobal::CreateNewSceneOperation.ID);
            wrapper.SetListener(nullptr);
            deleteLater();
        }
    }

private:
    LaunchModule* module = nullptr;
    DAVA::TArc::DataWrapper wrapper;
};

LaunchModule::~LaunchModule()
{
    Selectable::RemoveAllTransformProxies();
}

void LaunchModule::PostInit()
{
    Selectable::AddTransformProxyForClass<DAVA::Entity, EntityTransformProxy>();
    Selectable::AddTransformProxyForClass<DAVA::ParticleEmitterInstance, EmitterTransformProxy>();

    delayedExecutor.DelayedExecute([this]()
                                   {
                                       InvokeOperation(REGlobal::OpenLastProjectOperation.ID);
                                   });
    UnpackHelpDoc();
    new FirstSceneCreator(this);
}

void LaunchModule::UnpackHelpDoc()
{
    const DAVA::EngineContext* engineContext = GetAccessor()->GetEngineContext();
    DAVA::String editorVer = SettingsManager::GetValue(Settings::Internal_EditorVersion).AsString();
    DAVA::FilePath docsPath = DAVA::FilePath(ResourceEditor::DOCUMENTATION_PATH);
    if (editorVer != APPLICATION_BUILD_VERSION || !engineContext->fileSystem->Exists(docsPath))
    {
        DAVA::Logger::FrameworkDebug("Unpacking Help...");
        try
        {
            DAVA::ResourceArchive helpRA("~res:/ResourceEditor/Help.docs");
            engineContext->fileSystem->DeleteDirectory(docsPath);
            engineContext->fileSystem->CreateDirectory(docsPath, true);
            helpRA.UnpackToFolder(docsPath);
        }
        catch (std::exception& ex)
        {
            DAVA::Logger::Error("can't unpack Help.docs: %s", ex.what());
            DVASSERT(false && "can't upack Help.docs");
        }
    }
    SettingsManager::SetValue(Settings::Internal_EditorVersion, DAVA::VariantType(DAVA::String(APPLICATION_BUILD_VERSION)));
}