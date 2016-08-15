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

    void AddView(const WindowKey& windowKey, const PanelKey& panelKey, QWidget* widget) override;
    void AddView(const WindowKey& windowKey, const PanelKey& panelKey, const DAVA::String& resourceName, DataWrapper data) override;
    void AddAction(const WindowKey& windowKey, const ActionPlacementInfo& placement, QAction* action) override;

private:
    QWidget* LoadView(const DAVA::String& name, const DAVA::String& resourceName, DataWrapper data);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

}
