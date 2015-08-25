/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/

#ifndef __TEMPLATEPROJECTIPHONE__STATICWEBVIEWTEST__
#define __TEMPLATEPROJECTIPHONE__STATICWEBVIEWTEST__

#include "DAVAEngine.h"
#include "UI/UIWebView.h"

#include "Infrastructure/BaseScreen.h"

using namespace DAVA;

class MyWebViewDelegate;

class StaticWebViewTest : public BaseScreen
{
protected:
    ~StaticWebViewTest(){};
public:
    StaticWebViewTest();

    void LoadResources() override;
    void UnloadResources() override;

private:
    void OnButtonSetStatic(BaseObject *obj, void *data, void *callerData);
    void OnButtonSetNormal(BaseObject *obj, void *data, void *callerData);
    void OnButtonAdd10ToAlfa(BaseObject *obj, void *data, void *callerData);
    void OnButtonMinus10FromAlfa(BaseObject *obj, void *data, void *callerData);
    void OnButtonCheckTransparancy(BaseObject *obj, void *data, void *callerData);
    void OnButtonUncheckTransparancy(BaseObject *obj, void *data, void *callerData);
    void OnButtonExecJS(BaseObject* obj, void*, void*);
    void OnLoadHTMLString(BaseObject* obj, void*, void*);
    void OnButtonVisible(BaseObject*, void*, void*);
    void OnButtonHide(BaseObject*, void*, void*);

    UIButton* CreateUIButton(Font* font, const Rect& rect, const WideString& str,
                             void (StaticWebViewTest::*targetFunction)(BaseObject*, void*, void*));

    UIButton* setStaticButton = nullptr;
    UIButton* setNormalButton = nullptr;
    UIButton* add10ToAlfaButton = nullptr;
    UIButton* minus10FromAlfaButton = nullptr;
    UIButton* checkTransparancyButton = nullptr;
    UIButton* uncheckTransparancyButton = nullptr;
    UIButton* executeJSButton = nullptr;
    UIButton* loadHTMLString = nullptr;
    UIButton* setVisibleButton = nullptr;
    UIButton* setHideButton = nullptr;

    UIControl* overlapedImage = nullptr;

    UIWebView* webView1 = nullptr;
    UIWebView* webView2 = nullptr;
    UIWebView* webView3 = nullptr;

    MyWebViewDelegate* webviewDelegate = nullptr;
};

#endif /* defined(__TEMPLATEPROJECTIPHONE__STATICWEBVIEWTEST__) */
