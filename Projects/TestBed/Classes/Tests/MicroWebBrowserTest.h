#pragma once
#include "DAVAEngine.h"
#include "UI/UIWebView.h"

#include "Infrastructure/BaseScreen.h"

using namespace DAVA;

class GameCore;
class MicroWebBrowserTest : public BaseScreen, public UITextFieldDelegate
{
public:
    MicroWebBrowserTest(GameCore* g);

    void LoadResources() override;
    void UnloadResources() override;

private:
    ~MicroWebBrowserTest() = default;

    void OnLoadPage(BaseObject* obj, void* data, void* callerData);

    // UITextFieldDelegate implementation
    void TextFieldShouldReturn(UITextField* /*textField*/) override;

    RefPtr<UIWebView> webView;
    RefPtr<UITextField> textField;
};
