#include "Classes/Application/InitModule.h"

#include "Classes/Qt/Scene/Selectable.h"
#include "Classes/Qt/Scene/BaseTransformProxies.h"
#include "Classes/Qt/Settings/SettingsManager.h"
#include "Classes/Qt/Settings/Settings.h"
#include "Classes/StringConstants.h"
#include "version.h"

#include "Engine/EngineContext.h"
#include "Scene3D/Entity.h"
#include "Particles/ParticleEmitterInstance.h"
#include "FileSystem/ResourceArchive.h"
#include "FileSystem/FileSystem.h"

InitModule::~InitModule()
{
    Selectable::RemoveAllTransformProxies();
}

void InitModule::PostInit()
{
    Selectable::AddTransformProxyForClass<DAVA::Entity, EntityTransformProxy>();
    Selectable::AddTransformProxyForClass<DAVA::ParticleEmitterInstance, EmitterTransformProxy>();

    UnpackHelpDoc();
}

void InitModule::UnpackHelpDoc()
{
    DAVA::EngineContext* engineContext = GetAccessor()->GetEngineContext();
    DAVA::String editorVer = SettingsManager::GetValue(Settings::Internal_EditorVersion).AsString();
    DAVA::FilePath docsPath = DAVA::FilePath(ResourceEditor::DOCUMENTATION_PATH);
    if (editorVer != APPLICATION_BUILD_VERSION || !engineContext->fileSystem->Exists(docsPath))
    {
        DAVA::Logger::FrameworkDebug("Unpacking Help...");
        try
        {
            DAVA::ResourceArchive helpRA("~res:/Help.docs");
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
