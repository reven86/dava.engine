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

    void AddView(const WindowKey& key, QWidget* widget) override;
    void AddView(const WindowKey& key, const DAVA::String& resourceName, DataWrapper data) override;
    void AddAction(const DAVA::FastName& appID, const QUrl& placement, QAction* action) override;

private:
    QMainWindow* FindOrCreateWindow(const DAVA::FastName& appID);
    QWidget* LoadView(const DAVA::String& name, const DAVA::String& resourceName, DataWrapper data);

private:
    struct Impl;
    std::unique_ptr<Impl> impl;
};

}
