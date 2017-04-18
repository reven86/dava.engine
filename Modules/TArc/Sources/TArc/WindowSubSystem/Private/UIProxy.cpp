#include "UIProxy.h"

namespace DAVA
{
namespace TArc
{
class UIProxy::Guard
{
public:
    Guard(UIProxy* ui_)
        : ui(ui_)
    {
        ui->LockModule();
    }

    ~Guard()
    {
        ui->UnlockModule();
    }

private:
    UIProxy* ui;
};

UIProxy::UIProxy(ClientModule* module_, UI* globalUI_)
    : module(module_)
    , globalUI(globalUI_)
{
    globalUI->lastWaitDialogWasClosed.Connect(&lastWaitDialogWasClosed, &Signal<>::Emit);
}

void UIProxy::DeclareToolbar(const WindowKey& windowKey, const ActionPlacementInfo& toogleToolbarVisibility, const QString& toolbarName)
{
    Guard g(this);
    globalUI->DeclareToolbar(windowKey, toogleToolbarVisibility, toolbarName);
}

void UIProxy::AddView(const WindowKey& windowKey, const PanelKey& panelKey, QWidget* widget)
{
    Guard g(this);
    globalUI->AddView(windowKey, panelKey, widget);
}

void UIProxy::AddAction(const WindowKey& windowKey, const ActionPlacementInfo& placement, QAction* action)
{
    Guard g(this);
    globalUI->AddAction(windowKey, placement, action);
}

void UIProxy::RemoveAction(const WindowKey& windowKey, const ActionPlacementInfo& placement)
{
    Guard g(this);
    globalUI->RemoveAction(windowKey, placement);
}

void UIProxy::ShowMessage(const WindowKey& windowKey, const QString& message, uint32 duration /*= 0*/)
{
    globalUI->ShowMessage(windowKey, message, duration);
}

void UIProxy::ShowNotification(const WindowKey& windowKey, const NotificationParams& params)
{
    globalUI->ShowNotification(windowKey, params);
}

void UIProxy::ClearMessage(const WindowKey& windowKey)
{
    globalUI->ClearMessage(windowKey);
}

ModalMessageParams::Button UIProxy::ShowModalMessage(const WindowKey& windowKey, const ModalMessageParams& params)
{
    return globalUI->ShowModalMessage(windowKey, params);
}

QString UIProxy::GetOpenFileName(const WindowKey& windowKey, const FileDialogParams& params)
{
    return globalUI->GetOpenFileName(windowKey, params);
}

QString UIProxy::GetSaveFileName(const WindowKey& windowKey, const FileDialogParams& params)
{
    return globalUI->GetSaveFileName(windowKey, params);
}

QString UIProxy::GetExistingDirectory(const WindowKey& windowKey, const DirectoryDialogParams& params)
{
    return globalUI->GetExistingDirectory(windowKey, params);
}

std::unique_ptr<WaitHandle> UIProxy::ShowWaitDialog(const WindowKey& windowKey, const WaitDialogParams& params)
{
    return globalUI->ShowWaitDialog(windowKey, params);
}

bool UIProxy::HasActiveWaitDalogues() const
{
    return globalUI->HasActiveWaitDalogues();
}

QWidget* UIProxy::GetWindow(const WindowKey& windowKey)
{
    return globalUI->GetWindow(windowKey);
}

void UIProxy::InjectWindow(const WindowKey& windowKey, QMainWindow* window)
{
    Guard g(this);
    return globalUI->InjectWindow(windowKey, window);
}

void UIProxy::LockModule()
{
    globalUI->SetCurrentModule(module);
}

void UIProxy::UnlockModule()
{
    globalUI->SetCurrentModule(nullptr);
}

void UIProxy::SetCurrentModule(ClientModule* /*module*/)
{
    DVASSERT(false);
}

} // namespace TArc
} // namespace DAVA