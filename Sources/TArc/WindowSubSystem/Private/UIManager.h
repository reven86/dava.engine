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
    void AddView(const WindowKey& key, QWidget* widget) override;

private:
    QMainWindow* FindOrCreateWindow(const DAVA::FastName& appID);

private:
    DAVA::Array<DAVA::Function<void(const WindowKey&, QWidget*, QMainWindow*)>, WindowKey::TypesCount> addFunctions;
    DAVA::UnorderedMap<DAVA::FastName, QMainWindow*> windows;
};

}
