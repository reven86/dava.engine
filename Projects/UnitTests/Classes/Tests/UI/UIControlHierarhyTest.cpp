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

#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

using namespace DAVA;

DAVA_TESTCLASS (UIControlHierarhyTest)
{
    static Vector<std::pair<FastName, FastName>> callSequence;
    class UITestControl;
    // screen
    // |-x
    // | |-1
    // | | |-1
    // | |
    // | |-2
    // | | |-1
    // | |
    // | |-3
    // | |
    // | |-z
    // | | |-1
    // | | |-2
    // | | |-3

    UIScreen* screen = nullptr;
    UITestControl* root = nullptr;
    UITestControl* x = nullptr;
    UITestControl* x1 = nullptr;
    UITestControl* x2 = nullptr;
    UITestControl* x3 = nullptr;
    UITestControl* x11 = nullptr;
    UITestControl* x21 = nullptr;

    UITestControl* z = nullptr;
    UITestControl* z1 = nullptr;
    UITestControl* z2 = nullptr;
    UITestControl* z3 = nullptr;

    UIScreen* MakeScreen(const char* name)
    {
        UIScreen* c = new UIScreen();
        c->SetName(name);
        return c;
    }

    UITestControl* MakeChild(UIControl * parent, const char* name)
    {
        UITestControl* c = new UITestControl();
        c->SetName(name);
        if (parent)
            parent->AddControl(c);
        return c;
    }

    void SetUp(const String& testName) override
    {
        screen = MakeScreen("screen");

        root = MakeChild(screen, "root");

        x = MakeChild(nullptr, "x");
        x1 = MakeChild(x, "x1");
        x2 = MakeChild(x, "x2");
        x3 = MakeChild(x, "x3");
        x11 = MakeChild(x1, "x11");
        x21 = MakeChild(x2, "x21");

        z = MakeChild(nullptr, "z");
        z1 = MakeChild(z, "z1");
        z2 = MakeChild(z, "z2");
        z3 = MakeChild(z, "z3");

        UIControlSystem::Instance()->SetScreen(screen);
        UIControlSystem::Instance()->Update();

        callSequence.clear();
    }

    void TearDown(const String& testName) override
    {
        UIControlSystem::Instance()->Reset();

        SafeRelease(screen);
        SafeRelease(x);
        SafeRelease(x1);
        SafeRelease(x2);
        SafeRelease(x3);
        SafeRelease(x11);
        SafeRelease(x21);

        SafeRelease(z);
        SafeRelease(z1);
        SafeRelease(z2);
        SafeRelease(z3);
    }

    // UIControl::AddControl
    // simple
    // in OnAppear
    // in OnDisappear
    // in OnBecomeVisible
    // in OnBecomeInvisible

    // UIControl::RemoveControl
    // RemoveControl:
    // simple
    // in OnAppear
    // in OnDisappear
    // in OnBecomeVisible
    // in OnBecomeInvisible

    // UIControl::AddControl
    DAVA_TEST (AddControlToInvisibleActiveHierarhy)
    {
        root->SetVisible(false);
        callSequence.clear();
        root->AddControl(x);

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnAppear"), FastName("x") },
          { FastName("OnAppear"), FastName("x1") },
          { FastName("OnAppear"), FastName("x11") },
          { FastName("OnAppear"), FastName("x2") },
          { FastName("OnAppear"), FastName("x21") },
          { FastName("OnAppear"), FastName("x3") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);
    }

    // UIControl::AddControl
    DAVA_TEST (AddControlToNoneActiveHierarhy)
    {
        root->RemoveFromParent();
        callSequence.clear();
        root->AddControl(x);

        TEST_VERIFY(callSequence.empty());
    }

    // UIControl::AddControl
    DAVA_TEST (AddControlToVisibleActiveHierarhy)
    {
        root->SetVisible(true);
        callSequence.clear();
        root->AddControl(x);

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnAppear"), FastName("x") },
          { FastName("OnAppear"), FastName("x1") },
          { FastName("OnAppear"), FastName("x11") },
          { FastName("OnAppear"), FastName("x2") },
          { FastName("OnAppear"), FastName("x21") },
          { FastName("OnAppear"), FastName("x3") },

          { FastName("OnBecomeVisible"), FastName("x") },
          { FastName("OnBecomeVisible"), FastName("x1") },
          { FastName("OnBecomeVisible"), FastName("x11") },
          { FastName("OnBecomeVisible"), FastName("x2") },
          { FastName("OnBecomeVisible"), FastName("x21") },
          { FastName("OnBecomeVisible"), FastName("x3") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);
    }

    // UIControl::AddControl
    DAVA_TEST (AddControlToInvisibleAciveHierarhyInOnAppearCallback)
    {
        root->SetVisible(false);
        callSequence.clear();
        x1->onAppearCallback = [this]()
        {
            x->AddControl(z);
        };

        root->AddControl(x);

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnAppear"), FastName("x") },
          { FastName("OnAppear"), FastName("x1") },
          { FastName("OnAppear"), FastName("z") },
          { FastName("OnAppear"), FastName("z1") },
          { FastName("OnAppear"), FastName("z2") },
          { FastName("OnAppear"), FastName("z3") },
          { FastName("OnAppear"), FastName("x11") },
          { FastName("OnAppear"), FastName("x2") },
          { FastName("OnAppear"), FastName("x21") },
          { FastName("OnAppear"), FastName("x3") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);

        x1->onAppearCallback = nullptr;
    }

    // UIControl::AddControl
    DAVA_TEST (AddControlToInvisibleActiveHierarhyInOnDisappearCallback)
    {
        root->SetVisible(false);
        root->AddControl(x);
        callSequence.clear();
        x1->onDisappearCallback = [this]()
        {
            x->AddControl(z);
        };

        x->RemoveFromParent();

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnDisappear"), FastName("x3") },
          { FastName("OnDisappear"), FastName("x21") },
          { FastName("OnDisappear"), FastName("x2") },
          { FastName("OnDisappear"), FastName("x11") },
          { FastName("OnDisappear"), FastName("x1") },
          { FastName("OnAppear"), FastName("z") },
          { FastName("OnAppear"), FastName("z1") },
          { FastName("OnAppear"), FastName("z2") },
          { FastName("OnAppear"), FastName("z3") },
          { FastName("OnDisappear"), FastName("z3") },
          { FastName("OnDisappear"), FastName("z2") },
          { FastName("OnDisappear"), FastName("z1") },
          { FastName("OnDisappear"), FastName("z") },
          { FastName("OnDisappear"), FastName("x") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);

        x1->onDisappearCallback = nullptr;
    }

    // UIControl::AddControl
    DAVA_TEST (AddControlToVisibleHierarhyInOnBecomeVisibleCallback)
    {
        callSequence.clear();
        x1->onBecomeVisibleCallback = [this]()
        {
            x->AddControl(z);
        };

        root->AddControl(x);

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnAppear"), FastName("x") },
          { FastName("OnAppear"), FastName("x1") },
          { FastName("OnAppear"), FastName("x11") },
          { FastName("OnAppear"), FastName("x2") },
          { FastName("OnAppear"), FastName("x21") },
          { FastName("OnAppear"), FastName("x3") },

          { FastName("OnBecomeVisible"), FastName("x") },
          { FastName("OnBecomeVisible"), FastName("x1") },
          { FastName("OnAppear"), FastName("z") },
          { FastName("OnAppear"), FastName("z1") },
          { FastName("OnAppear"), FastName("z2") },
          { FastName("OnAppear"), FastName("z3") },
          { FastName("OnBecomeVisible"), FastName("z") },
          { FastName("OnBecomeVisible"), FastName("z1") },
          { FastName("OnBecomeVisible"), FastName("z2") },
          { FastName("OnBecomeVisible"), FastName("z3") },
          { FastName("OnBecomeVisible"), FastName("x11") },
          { FastName("OnBecomeVisible"), FastName("x2") },
          { FastName("OnBecomeVisible"), FastName("x21") },
          { FastName("OnBecomeVisible"), FastName("x3") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);

        x1->onBecomeVisibleCallback = nullptr;
    }

    // UIControl::AddControl
    DAVA_TEST (AddControlToVisibleHierarhyInOnBecomeInvisibleCallback)
    {
        root->AddControl(x);
        callSequence.clear();
        x1->onBecomeInvisibleCallback = [this]()
        {
            x->AddControl(z);
        };

        x->RemoveFromParent();

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnBecomeInvisible"), FastName("x3") },
          { FastName("OnBecomeInvisible"), FastName("x21") },
          { FastName("OnBecomeInvisible"), FastName("x2") },
          { FastName("OnBecomeInvisible"), FastName("x11") },
          { FastName("OnBecomeInvisible"), FastName("x1") },
          { FastName("OnAppear"), FastName("z") },
          { FastName("OnAppear"), FastName("z1") },
          { FastName("OnAppear"), FastName("z2") },
          { FastName("OnAppear"), FastName("z3") },
          { FastName("OnBecomeVisible"), FastName("z") },
          { FastName("OnBecomeVisible"), FastName("z1") },
          { FastName("OnBecomeVisible"), FastName("z2") },
          { FastName("OnBecomeVisible"), FastName("z3") },

          { FastName("OnBecomeInvisible"), FastName("z3") },
          { FastName("OnBecomeInvisible"), FastName("z2") },
          { FastName("OnBecomeInvisible"), FastName("z1") },
          { FastName("OnBecomeInvisible"), FastName("z") },

          { FastName("OnBecomeInvisible"), FastName("x") },
          { FastName("OnDisappear"), FastName("z3") },
          { FastName("OnDisappear"), FastName("z2") },
          { FastName("OnDisappear"), FastName("z1") },
          { FastName("OnDisappear"), FastName("z") },
          { FastName("OnDisappear"), FastName("x3") },
          { FastName("OnDisappear"), FastName("x21") },
          { FastName("OnDisappear"), FastName("x2") },
          { FastName("OnDisappear"), FastName("x11") },
          { FastName("OnDisappear"), FastName("x1") },
          { FastName("OnDisappear"), FastName("x") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);

        x1->onBecomeInvisibleCallback = nullptr;
    }

    // UIControl::RemoveControl
    DAVA_TEST (RemoveControlFromInvisibleActiveHierarhy)
    {
        root->SetVisible(false);
        root->AddControl(x);
        callSequence.clear();

        root->RemoveControl(x);

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnDisappear"), FastName("x3") },
          { FastName("OnDisappear"), FastName("x21") },
          { FastName("OnDisappear"), FastName("x2") },
          { FastName("OnDisappear"), FastName("x11") },
          { FastName("OnDisappear"), FastName("x1") },
          { FastName("OnDisappear"), FastName("x") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);
    }

    // UIControl::RemoveControl
    DAVA_TEST (RemoveControlFromVisibleHierarhy)
    {
        root->SetVisible(true);
        root->AddControl(x);
        callSequence.clear();

        root->RemoveControl(x);

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnBecomeInvisible"), FastName("x3") },
          { FastName("OnBecomeInvisible"), FastName("x21") },
          { FastName("OnBecomeInvisible"), FastName("x2") },
          { FastName("OnBecomeInvisible"), FastName("x11") },
          { FastName("OnBecomeInvisible"), FastName("x1") },
          { FastName("OnBecomeInvisible"), FastName("x") },

          { FastName("OnDisappear"), FastName("x3") },
          { FastName("OnDisappear"), FastName("x21") },
          { FastName("OnDisappear"), FastName("x2") },
          { FastName("OnDisappear"), FastName("x11") },
          { FastName("OnDisappear"), FastName("x1") },
          { FastName("OnDisappear"), FastName("x") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);
    }

    // UIControl::RemoveControl
    DAVA_TEST (RemoveControlFromInvisibleActiveHierarhyInOnAppearCallback)
    {
        root->SetVisible(false);
        x->AddControl(z);
        callSequence.clear();

        x1->onAppearCallback = [this]()
        {
            x->RemoveControl(z);
        };

        root->AddControl(x);

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnAppear"), FastName("x") },
          { FastName("OnAppear"), FastName("x1") },
          { FastName("OnAppear"), FastName("x11") },
          { FastName("OnAppear"), FastName("x2") },
          { FastName("OnAppear"), FastName("x21") },
          { FastName("OnAppear"), FastName("x3") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);

        x1->onAppearCallback = nullptr;
    }

    // UIControl::RemoveControl
    DAVA_TEST (RemoveControlFromInvisibleActiveHierarhyInOnDisappearCallback)
    {
        root->SetVisible(false);
        x->AddControl(z);
        root->AddControl(x);
        callSequence.clear();

        x1->onDisappearCallback = [this]()
        {
            x->RemoveControl(z);
        };

        root->RemoveControl(x);

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnDisappear"), FastName("z3") },
          { FastName("OnDisappear"), FastName("z2") },
          { FastName("OnDisappear"), FastName("z1") },
          { FastName("OnDisappear"), FastName("z") },
          { FastName("OnDisappear"), FastName("x3") },
          { FastName("OnDisappear"), FastName("x21") },
          { FastName("OnDisappear"), FastName("x2") },
          { FastName("OnDisappear"), FastName("x11") },
          { FastName("OnDisappear"), FastName("x1") },
          { FastName("OnDisappear"), FastName("x") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);

        x1->onDisappearCallback = nullptr;
    }

    // UIControl::RemoveControl
    DAVA_TEST (RemoveControlFromVisibleHierarhyInOnBecomeVisibleCallback)
    {
        x->AddControl(z);
        callSequence.clear();
        x1->onBecomeVisibleCallback = [this]()
        {
            x->RemoveControl(z);
        };

        root->AddControl(x);

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnAppear"), FastName("x") },
          { FastName("OnAppear"), FastName("x1") },
          { FastName("OnAppear"), FastName("x11") },
          { FastName("OnAppear"), FastName("x2") },
          { FastName("OnAppear"), FastName("x21") },
          { FastName("OnAppear"), FastName("x3") },
          { FastName("OnAppear"), FastName("z") },
          { FastName("OnAppear"), FastName("z1") },
          { FastName("OnAppear"), FastName("z2") },
          { FastName("OnAppear"), FastName("z3") },

          { FastName("OnBecomeVisible"), FastName("x") },
          { FastName("OnBecomeVisible"), FastName("x1") },
          { FastName("OnDisappear"), FastName("z3") },
          { FastName("OnDisappear"), FastName("z2") },
          { FastName("OnDisappear"), FastName("z1") },
          { FastName("OnDisappear"), FastName("z") },
          { FastName("OnBecomeVisible"), FastName("x11") },
          { FastName("OnBecomeVisible"), FastName("x2") },
          { FastName("OnBecomeVisible"), FastName("x21") },
          { FastName("OnBecomeVisible"), FastName("x3") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);

        x1->onBecomeVisibleCallback = nullptr;
    }

    // UIControl::RemoveControl
    DAVA_TEST (RemoveControlFromVisibleHierarhyInOnBecomeInvisibleCallback)
    {
        x->AddControl(z);
        root->AddControl(x);
        callSequence.clear();
        x1->onBecomeInvisibleCallback = [this]()
        {
            x->RemoveControl(z);
        };

        x->RemoveFromParent();

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnBecomeInvisible"), FastName("z3") },
          { FastName("OnBecomeInvisible"), FastName("z2") },
          { FastName("OnBecomeInvisible"), FastName("z1") },
          { FastName("OnBecomeInvisible"), FastName("z") },
          { FastName("OnBecomeInvisible"), FastName("x3") },
          { FastName("OnBecomeInvisible"), FastName("x21") },
          { FastName("OnBecomeInvisible"), FastName("x2") },
          { FastName("OnBecomeInvisible"), FastName("x11") },
          { FastName("OnBecomeInvisible"), FastName("x1") },
          { FastName("OnDisappear"), FastName("z3") },
          { FastName("OnDisappear"), FastName("z2") },
          { FastName("OnDisappear"), FastName("z1") },
          { FastName("OnDisappear"), FastName("z") },
          { FastName("OnBecomeInvisible"), FastName("x") },

          { FastName("OnDisappear"), FastName("x3") },
          { FastName("OnDisappear"), FastName("x21") },
          { FastName("OnDisappear"), FastName("x2") },
          { FastName("OnDisappear"), FastName("x11") },
          { FastName("OnDisappear"), FastName("x1") },
          { FastName("OnDisappear"), FastName("x") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);

        x1->onBecomeInvisibleCallback = nullptr;
    }

    // UIControl::SetVisible
    DAVA_TEST (SetVisibleForInvisibleControl)
    {
        x->SetVisible(false);
        root->AddControl(x);
        callSequence.clear();

        x->SetVisible(true);

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnBecomeVisible"), FastName("x") },
          { FastName("OnBecomeVisible"), FastName("x1") },
          { FastName("OnBecomeVisible"), FastName("x11") },
          { FastName("OnBecomeVisible"), FastName("x2") },
          { FastName("OnBecomeVisible"), FastName("x21") },
          { FastName("OnBecomeVisible"), FastName("x3") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);
    }

    // UIControl::SetVisible
    DAVA_TEST (SetVisibleForVisibleControl)
    {
        x->SetVisible(true);

        root->AddControl(x);
        callSequence.clear();

        x->SetVisible(true);

        TEST_VERIFY(callSequence.empty());
    }

    // UIControl::SetVisible
    DAVA_TEST (SetInvisibleForVisibleControl)
    {
        x->SetVisible(true);

        root->AddControl(x);
        callSequence.clear();

        x->SetVisible(false);

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnBecomeInvisible"), FastName("x3") },
          { FastName("OnBecomeInvisible"), FastName("x21") },
          { FastName("OnBecomeInvisible"), FastName("x2") },
          { FastName("OnBecomeInvisible"), FastName("x11") },
          { FastName("OnBecomeInvisible"), FastName("x1") },
          { FastName("OnBecomeInvisible"), FastName("x") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);
    }

    // UIControl::SetVisible
    DAVA_TEST (SetInvisibleForInvisibleControl)
    {
        x->SetVisible(false);

        root->AddControl(x);
        callSequence.clear();

        x->SetVisible(false);

        TEST_VERIFY(callSequence.empty());
    }

    // UIControl::SetVisible
    DAVA_TEST (SetVisibleForNoneActiveHierarhy)
    {
        x->SetVisible(false);
        callSequence.clear();

        x->SetVisible(true);

        TEST_VERIFY(callSequence.empty());
    }

    // UIControl::SetVisible
    DAVA_TEST (SetInvisibleForNoneActiveHierarhy)
    {
        x->SetVisible(false);

        TEST_VERIFY(callSequence.empty());
    }

    // UIControl::SetVisible
    DAVA_TEST (SetVisibleToInvisibleActiveHierarhyInOnAppearCallback)
    {
        root->SetVisible(false);
        x->AddControl(z);
        z->SetVisible(false);
        callSequence.clear();
        x1->onAppearCallback = [this]()
        {
            z->SetVisible(true);
        };

        root->AddControl(x);

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnAppear"), FastName("x") },
          { FastName("OnAppear"), FastName("x1") },
          { FastName("OnAppear"), FastName("x11") },
          { FastName("OnAppear"), FastName("x2") },
          { FastName("OnAppear"), FastName("x21") },
          { FastName("OnAppear"), FastName("x3") },
          { FastName("OnAppear"), FastName("z") },
          { FastName("OnAppear"), FastName("z1") },
          { FastName("OnAppear"), FastName("z2") },
          { FastName("OnAppear"), FastName("z3") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);

        x1->onAppearCallback = nullptr;
    }

    // UIControl::SetVisible
    DAVA_TEST (SetVisibleToInvisibleActiveHierarhyInOnDisappearCallback)
    {
        root->SetVisible(false);
        root->AddControl(x);
        x->AddControl(z);
        z->SetVisible(false);
        callSequence.clear();
        x1->onDisappearCallback = [this]()
        {
            z->SetVisible(true);
        };

        x->RemoveFromParent();

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnDisappear"), FastName("z3") },
          { FastName("OnDisappear"), FastName("z2") },
          { FastName("OnDisappear"), FastName("z1") },
          { FastName("OnDisappear"), FastName("z") },
          { FastName("OnDisappear"), FastName("x3") },
          { FastName("OnDisappear"), FastName("x21") },
          { FastName("OnDisappear"), FastName("x2") },
          { FastName("OnDisappear"), FastName("x11") },
          { FastName("OnDisappear"), FastName("x1") },
          { FastName("OnDisappear"), FastName("x") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);

        x1->onDisappearCallback = nullptr;
    }

    // UIControl::SetVisible
    DAVA_TEST (SetVisibleToVisibleHierarhyInOnBecomeVisibleCallback)
    {
        x->AddControl(z);
        z->SetVisible(false);
        callSequence.clear();

        x1->onBecomeVisibleCallback = [this]()
        {
            z->SetVisible(true);
        };

        root->AddControl(x);

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnAppear"), FastName("x") },
          { FastName("OnAppear"), FastName("x1") },
          { FastName("OnAppear"), FastName("x11") },
          { FastName("OnAppear"), FastName("x2") },
          { FastName("OnAppear"), FastName("x21") },
          { FastName("OnAppear"), FastName("x3") },
          { FastName("OnAppear"), FastName("z") },
          { FastName("OnAppear"), FastName("z1") },
          { FastName("OnAppear"), FastName("z2") },
          { FastName("OnAppear"), FastName("z3") },

          { FastName("OnBecomeVisible"), FastName("x") },
          { FastName("OnBecomeVisible"), FastName("x1") },
          { FastName("OnBecomeVisible"), FastName("z") },
          { FastName("OnBecomeVisible"), FastName("z1") },
          { FastName("OnBecomeVisible"), FastName("z2") },
          { FastName("OnBecomeVisible"), FastName("z3") },
          { FastName("OnBecomeVisible"), FastName("x11") },
          { FastName("OnBecomeVisible"), FastName("x2") },
          { FastName("OnBecomeVisible"), FastName("x21") },
          { FastName("OnBecomeVisible"), FastName("x3") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);

        x1->onBecomeVisibleCallback = nullptr;
    }

    // UIControl::SetVisible
    DAVA_TEST (SetVisibleToVisibleHierarhyInOnBecomeInvisibleCallback)
    {
        root->AddControl(x);
        x->AddControl(z);
        z->SetVisible(false);
        callSequence.clear();
        x1->onBecomeInvisibleCallback = [this]()
        {
            z->SetVisible(true);
        };

        x->RemoveFromParent();

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnBecomeInvisible"), FastName("x3") },
          { FastName("OnBecomeInvisible"), FastName("x21") },
          { FastName("OnBecomeInvisible"), FastName("x2") },
          { FastName("OnBecomeInvisible"), FastName("x11") },
          { FastName("OnBecomeInvisible"), FastName("x1") },
          { FastName("OnBecomeVisible"), FastName("z") },
          { FastName("OnBecomeVisible"), FastName("z1") },
          { FastName("OnBecomeVisible"), FastName("z2") },
          { FastName("OnBecomeVisible"), FastName("z3") },
          { FastName("OnBecomeInvisible"), FastName("x") },
          { FastName("OnBecomeInvisible"), FastName("z3") },
          { FastName("OnBecomeInvisible"), FastName("z2") },
          { FastName("OnBecomeInvisible"), FastName("z1") },
          { FastName("OnBecomeInvisible"), FastName("z") },

          { FastName("OnDisappear"), FastName("z3") },
          { FastName("OnDisappear"), FastName("z2") },
          { FastName("OnDisappear"), FastName("z1") },
          { FastName("OnDisappear"), FastName("z") },
          { FastName("OnDisappear"), FastName("x3") },
          { FastName("OnDisappear"), FastName("x21") },
          { FastName("OnDisappear"), FastName("x2") },
          { FastName("OnDisappear"), FastName("x11") },
          { FastName("OnDisappear"), FastName("x1") },
          { FastName("OnDisappear"), FastName("x") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);

        x1->onBecomeInvisibleCallback = nullptr;
    }

    // UIControl::SetVisible
    DAVA_TEST (SetInvisibleToInvisibleActiveHierarhyInOnAppearCallback)
    {
        root->SetVisible(false);
        x->AddControl(z);
        z->SetVisible(false);
        callSequence.clear();
        x1->onAppearCallback = [this]()
        {
            z->SetVisible(false);
        };

        root->AddControl(x);

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnAppear"), FastName("x") },
          { FastName("OnAppear"), FastName("x1") },
          { FastName("OnAppear"), FastName("x11") },
          { FastName("OnAppear"), FastName("x2") },
          { FastName("OnAppear"), FastName("x21") },
          { FastName("OnAppear"), FastName("x3") },
          { FastName("OnAppear"), FastName("z") },
          { FastName("OnAppear"), FastName("z1") },
          { FastName("OnAppear"), FastName("z2") },
          { FastName("OnAppear"), FastName("z3") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);

        x1->onAppearCallback = nullptr;
    }

    // UIControl::SetVisible
    DAVA_TEST (SetInvisibleToInvisibleActiveHierarhyInOnDisappearCallback)
    {
        root->SetVisible(false);
        root->AddControl(x);
        x->AddControl(z);
        z->SetVisible(false);
        callSequence.clear();
        x1->onDisappearCallback = [this]()
        {
            z->SetVisible(false);
        };

        x->RemoveFromParent();

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnDisappear"), FastName("z3") },
          { FastName("OnDisappear"), FastName("z2") },
          { FastName("OnDisappear"), FastName("z1") },
          { FastName("OnDisappear"), FastName("z") },
          { FastName("OnDisappear"), FastName("x3") },
          { FastName("OnDisappear"), FastName("x21") },
          { FastName("OnDisappear"), FastName("x2") },
          { FastName("OnDisappear"), FastName("x11") },
          { FastName("OnDisappear"), FastName("x1") },
          { FastName("OnDisappear"), FastName("x") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);

        x1->onDisappearCallback = nullptr;
    }

    // UIControl::SetVisible
    DAVA_TEST (SetInvisibleToVisibleHierarhyInOnBecomeVisibleCallback)
    {
        x->AddControl(z);
        callSequence.clear();

        x1->onBecomeVisibleCallback = [this]()
        {
            z->SetVisible(false);
        };

        root->AddControl(x);

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnAppear"), FastName("x") },
          { FastName("OnAppear"), FastName("x1") },
          { FastName("OnAppear"), FastName("x11") },
          { FastName("OnAppear"), FastName("x2") },
          { FastName("OnAppear"), FastName("x21") },
          { FastName("OnAppear"), FastName("x3") },
          { FastName("OnAppear"), FastName("z") },
          { FastName("OnAppear"), FastName("z1") },
          { FastName("OnAppear"), FastName("z2") },
          { FastName("OnAppear"), FastName("z3") },

          { FastName("OnBecomeVisible"), FastName("x") },
          { FastName("OnBecomeVisible"), FastName("x1") },
          { FastName("OnBecomeVisible"), FastName("x11") },
          { FastName("OnBecomeVisible"), FastName("x2") },
          { FastName("OnBecomeVisible"), FastName("x21") },
          { FastName("OnBecomeVisible"), FastName("x3") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);

        x1->onBecomeVisibleCallback = nullptr;
    }

    // UIControl::SetVisible
    DAVA_TEST (SetInvisibleToVisibleHierarhyInOnBecomeInvisibleCallback)
    {
        root->AddControl(x);
        x->AddControl(z);
        callSequence.clear();
        x1->onBecomeInvisibleCallback = [this]()
        {
            z->SetVisible(false);
        };

        x->RemoveFromParent();

        Vector<std::pair<FastName, FastName>> expectedCallSequence =
        {
          { FastName("OnBecomeInvisible"), FastName("z3") },
          { FastName("OnBecomeInvisible"), FastName("z2") },
          { FastName("OnBecomeInvisible"), FastName("z1") },
          { FastName("OnBecomeInvisible"), FastName("z") },
          { FastName("OnBecomeInvisible"), FastName("x3") },
          { FastName("OnBecomeInvisible"), FastName("x21") },
          { FastName("OnBecomeInvisible"), FastName("x2") },
          { FastName("OnBecomeInvisible"), FastName("x11") },
          { FastName("OnBecomeInvisible"), FastName("x1") },
          { FastName("OnBecomeInvisible"), FastName("x") },

          { FastName("OnDisappear"), FastName("z3") },
          { FastName("OnDisappear"), FastName("z2") },
          { FastName("OnDisappear"), FastName("z1") },
          { FastName("OnDisappear"), FastName("z") },
          { FastName("OnDisappear"), FastName("x3") },
          { FastName("OnDisappear"), FastName("x21") },
          { FastName("OnDisappear"), FastName("x2") },
          { FastName("OnDisappear"), FastName("x11") },
          { FastName("OnDisappear"), FastName("x1") },
          { FastName("OnDisappear"), FastName("x") },
        };

        TEST_VERIFY(expectedCallSequence == callSequence);

        x1->onBecomeInvisibleCallback = nullptr;
    }

    class UITestControl : public UIControl
    {
    public:
        UITestControl() = default;

        DAVA::Function<void()> onBecomeVisibleCallback;
        DAVA::Function<void()> onBecomeInvisibleCallback;
        DAVA::Function<void()> onAppearCallback;
        DAVA::Function<void()> onDisappearCallback;

    protected:
        ~UITestControl() override = default;

        void OnBecomeVisible() override
        {
            UIControl::OnBecomeVisible();
            callSequence.emplace_back(FastName("OnBecomeVisible"), GetName());
            if (onBecomeVisibleCallback)
                onBecomeVisibleCallback();
        }
        void OnBecomeInvisible() override
        {
            UIControl::OnBecomeInvisible();
            callSequence.emplace_back(FastName("OnBecomeInvisible"), GetName());
            if (onBecomeInvisibleCallback)
                onBecomeInvisibleCallback();
        }

        void OnAppear() override
        {
            UIControl::OnAppear();
            callSequence.emplace_back(FastName("OnAppear"), GetName());
            if (onAppearCallback)
                onAppearCallback();
        }
        void OnDisappear() override
        {
            UIControl::OnDisappear();
            callSequence.emplace_back(FastName("OnDisappear"), GetName());
            if (onDisappearCallback)
                onDisappearCallback();
        }
    };
};
Vector<std::pair<FastName, FastName>> UIControlHierarhyTest::callSequence;