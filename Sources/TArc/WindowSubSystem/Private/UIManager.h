#pragma once

#include "Base/BaseTypes.h"

#include "WindowSubSystem/UI.h"

class QMainWindow;
namespace tarc
{

class UIManager final: public UI
{
public:
    UIManager();
    ~UIManager();

    void InitializationFinished();

    void AddView(const WindowKey& windowKey, const PanelKey& panelKey, QWidget* widget) override;
    void AddView(const WindowKey& windowKey, const PanelKey& panelKey, const QString& resourceName, DataWrapper data) override;
    void AddAction(const WindowKey& windowKey, const ActionPlacementInfo& placement, QAction* action) override;

    void ShowMessage(const WindowKey& windowKey, const QString& message, DAVA::uint32 duration = 0) override;
    void ClearMessage(const WindowKey& windowKey) override;

    std::unique_ptr<WaitHandle> ShowWaitDialog(const WindowKey& windowKey, const WaitDialogParams& params = WaitDialogParams()) override;

private:
    QWidget* LoadView(const QString& name, const QString& resourceName, DataWrapper data);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

}
