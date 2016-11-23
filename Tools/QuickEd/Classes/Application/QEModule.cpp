#include "Application/QEModule.h"
 
#include "EditorCore.h"
#include "Project/Project.h"
#include "Document/DocumentGroup.h"

#include "Debug/DVAssert.h"
#include "Base/FastName.h"
#include "Render/Renderer.h"
#include "Render/DynamicBufferAllocator.h"
#include "Particles/ParticleEmitter.h"
#include "TArc/WindowSubSystem/Private/UIManager.h"
#include "TArc/DataProcessing/DataNode.h"


#include "QtTools/Utils/Themes/Themes.h"

namespace QEModuleDetail
{
class QEGlobalData : public DAVA::TArc::DataNode
{
public:
    QEGlobalData()
        : editorCore()
        , windowKey(DAVA::FastName("QuickEd"))
    {
        editorCore.reset(new EditorCore());
    }
    ~QEGlobalData()
    {
        DAVA::Logger::Debug("test");
    }
    std::unique_ptr<EditorCore> editorCore;
    DAVA::TArc::WindowKey windowKey;

    DAVA_VIRTUAL_REFLECTION(QEGlobalData)
    {
    };
};
}

QEModule::~QEModule()
{
    GetAccessor().GetGlobalContext()->DeleteData<QEModuleDetail::QEGlobalData>();
}

void QEModule::OnRenderSystemInitialized(DAVA::Window* w)
{
    DAVA::Renderer::SetDesiredFPS(60);
    DAVA::DynamicBufferAllocator::SetPageSize(16 * 1024 * 1024); // 16 mb

    using namespace DAVA::TArc;
    ContextAccessor& accessor = GetAccessor();
    DataContext* globalContext = accessor.GetGlobalContext();

    QEModuleDetail::QEGlobalData* globalData = globalContext->GetData<QEModuleDetail::QEGlobalData>();
    DVASSERT(globalData != nullptr);
    globalData->editorCore->OnRenderingInitialized();
}

bool QEModule::CanWindowBeClosedSilently(const DAVA::TArc::WindowKey& key, DAVA::String& requestWindowText)
{
    QEModuleDetail::QEGlobalData* globalData = GetAccessor().GetGlobalContext()->GetData<QEModuleDetail::QEGlobalData>();
    DVASSERT(globalData->windowKey == key);
    Project* project = globalData->editorCore->GetProject();
    if (project != nullptr)
    {
        DocumentGroup* documentGroup = project->GetDocumentGroup();
        QString windowText = QObject::tr("Save changes to the following items?\n");
        QStringList unsavedDocuments = documentGroup->GetUnsavedDocumentsNames();
        const int maxDisplayableDocumentsCount = 15;
        if (unsavedDocuments.size() > maxDisplayableDocumentsCount)
        {
            unsavedDocuments = unsavedDocuments.mid(0, maxDisplayableDocumentsCount);
            unsavedDocuments << "...";
        }
        windowText.append(unsavedDocuments.join("\n"));
        requestWindowText = windowText.toStdString();
        return documentGroup->HasUnsavedDocuments() == false;
    }
    return true;
}

void QEModule::SaveOnWindowClose(const DAVA::TArc::WindowKey& key)
{
    QEModuleDetail::QEGlobalData* globalData = GetAccessor().GetGlobalContext()->GetData<QEModuleDetail::QEGlobalData>();
    DVASSERT(globalData->windowKey == key);
    Project* project = globalData->editorCore->GetProject();
    if (project != nullptr)
    {
        project->GetDocumentGroup()->SaveAllDocuments();
    }
}

void QEModule::RestoreOnWindowClose(const DAVA::TArc::WindowKey& key)
{
    //do nothing
}

void QEModule::OnContextCreated(DAVA::TArc::DataContext& context)
{
    DVASSERT_MSG(false, "Temporary assert. Now nobody creates context. Nobody.");
}

void QEModule::OnContextDeleted(DAVA::TArc::DataContext& context)
{
    DVASSERT_MSG(false, "Temporary assert. Now nobody creates context and there is no context that can be deleted");
}

void QEModule::OnWindowClosed(const DAVA::TArc::WindowKey& key)
{
    QEModuleDetail::QEGlobalData* globalData = GetAccessor().GetGlobalContext()->GetData<QEModuleDetail::QEGlobalData>();
    DVASSERT(globalData->windowKey == key);
    EditorCore* editorCore = globalData->editorCore.get();
    Project* project = editorCore->GetProject();
    if (project != nullptr)
    {
        project->GetDocumentGroup()->CloseAllDocuments();
        editorCore->CloseProject(true);
    }
}

void QEModule::PostInit()
{
    Themes::InitFromQApplication();

    DAVA::TArc::ContextAccessor& accessor = GetAccessor();
    DAVA::EngineContext* engineContext = accessor.GetEngineContext();

    using TData = QEModuleDetail::QEGlobalData;
    DAVA::TArc::DataContext* globalContext = accessor.GetGlobalContext();
    globalContext->CreateData(std::make_unique<TData>());
    TData* globalData = globalContext->GetData<TData>();

    DAVA::TArc::UIManager& ui = static_cast<DAVA::TArc::UIManager&>(GetUI());
    EditorCore* editorCore = globalData->editorCore.get();
    MainWindow* mainWindow = editorCore->GetMainWindow();
    mainWindow->InjectRenderWidget(GetContextManager().GetRenderWidget());
    ui.InjectWindow(globalData->windowKey, mainWindow);
}
