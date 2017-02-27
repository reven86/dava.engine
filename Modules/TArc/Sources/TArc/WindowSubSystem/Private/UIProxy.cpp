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
    Guard g(this);
    globalUI->ShowMessage(windowKey, message, duration);
}

void UIProxy::ClearMessage(const WindowKey& windowKey)
{
    Guard g(this);
    globalUI->ClearMessage(windowKey);
}

ModalMessageParams::Button UIProxy::ShowModalMessage(const WindowKey& windowKey, const ModalMessageParams& params)
{
    Guard g(this);
    return globalUI->ShowModalMessage(windowKey, params);
}

QString UIProxy::GetOpenFileName(const WindowKey& windowKey, const FileDialogParams& params)
{
    Guard g(this);
    return globalUI->GetOpenFileName(windowKey, params);
}

QString UIProxy::GetSaveFileName(const WindowKey& windowKey, const FileDialogParams& params)
{
    Guard g(this);
    return globalUI->GetSaveFileName(windowKey, params);
}

QString UIProxy::GetExistingDirectory(const WindowKey& windowKey, const DirectoryDialogParams& params)
{
    Guard g(this);
    return globalUI->GetExistingDirectory(windowKey, params);
}

std::unique_ptr<WaitHandle> UIProxy::ShowWaitDialog(const WindowKey& windowKey, const WaitDialogParams& params)
{
    Guard g(this);
    return globalUI->ShowWaitDialog(windowKey, params);
}

bool UIProxy::HasActiveWaitDalogues() const
{
    Guard g(const_cast<UIProxy*>(this));
    return globalUI->HasActiveWaitDalogues();
}

QWidget* UIProxy::GetWindow(const WindowKey& windowKey)
{
    Guard g(this);
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