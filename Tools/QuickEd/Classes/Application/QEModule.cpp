#include "Application/QEModule.h"
 
#include "EditorCore.h"
#include "Project/Project.h"
#include "Document/DocumentGroup.h"

#include "Debug/DVAssert.h"
#include "Base/FastName.h"
#include "Render/Renderer.h"
#include "Render/DynamicBufferAllocator.h"
#include "Particles/ParticleEmitter.h"
#include "WindowSubSystem/Private/UIManager.h"
#include "DataProcessing/DataNode.h"


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
    GetAccessor().GetGlobalContext().DeleteData<QEModuleDetail::QEGlobalData>();
}

void QEModule::OnRenderSystemInitialized(DAVA::Window* w)
{
    DAVA::Renderer::SetDesiredFPS(60);
    DAVA::DynamicBufferAllocator::SetPageSize(16 * 1024 * 1024); // 16 mb

    using TData = QEModuleDetail::QEGlobalData;
    DVASSERT(GetAccessor().GetGlobalContext().HasData<TData>());
    TData& globalData = GetAccessor().GetGlobalContext().GetData<TData>();
    globalData.editorCore->OnRenderingInitialized();
}

bool QEModule::CanWindowBeClosedSilently(const DAVA::TArc::WindowKey& key)
{
    QEModuleDetail::QEGlobalData& globalData = GetAccessor().GetGlobalContext().GetData<QEModuleDetail::QEGlobalData>();
    DVASSERT(globalData.windowKey == key);
    return globalData.editorCore->GetProject()->GetDocumentGroup()->HasUnsavedDocuments();
}

bool QEModule::ControlWindowClosing(const DAVA::TArc::WindowKey& key, QCloseEvent* event)
{
    QEModuleDetail::QEGlobalData& globalData = GetAccessor().GetGlobalContext().GetData<QEModuleDetail::QEGlobalData>();
    DVASSERT(globalData.windowKey == key);
    Project* project = globalData.editorCore->GetProject();
    bool canAccept = true;
    if (project != nullptr)
    {
        DocumentGroup* documentGroup = project->GetDocumentGroup();
        if (documentGroup->HasUnsavedDocuments())
        {
            canAccept = false;
        }
    }

    if (canAccept)
    {
        event->accept();
    }
    else
    {
        event->ignore();
    }
    return true;
}

void QEModule::SaveOnWindowClose(const DAVA::TArc::WindowKey& key)
{
}

void QEModule::RestoreOnWindowClose(const DAVA::TArc::WindowKey& key)
{
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
    QEModuleDetail::QEGlobalData& globalData = GetAccessor().GetGlobalContext().GetData<QEModuleDetail::QEGlobalData>();
    DVASSERT(globalData.windowKey == key);
    Project* project = globalData.editorCore->GetProject();
    if (project != nullptr)
    {
        project->GetDocumentGroup()->CloseAllDocuments();
    }
}

void QEModule::PostInit()
{
    Themes::InitFromQApplication();

    DAVA::TArc::ContextAccessor& accessor = GetAccessor();
    DAVA::EngineContext& engineContext = accessor.GetEngineContext();

    using TData = QEModuleDetail::QEGlobalData;
    DAVA::TArc::DataContext& globalContext = accessor.GetGlobalContext();
    globalContext.CreateData(std::make_unique<TData>());
    TData& globalData = globalContext.GetData<TData>();

    DAVA::TArc::UIManager& ui = static_cast<DAVA::TArc::UIManager&>(GetUI());
    EditorCore* editorCore = globalData.editorCore.get();
    MainWindow* mainWindow = editorCore->GetMainWindow();
    mainWindow->InjectRenderWidget(GetContextManager().GetRenderWidget());
    ui.InjectWindow(globalData.windowKey, mainWindow);
}
