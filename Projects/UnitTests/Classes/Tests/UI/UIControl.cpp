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

#include "UI/UIControlPackageContext.h"
#include "UnitTests/UnitTests.h"

using namespace DAVA;

extern int32 GetCpuCount();

DAVA_TESTCLASS(UIControlTest)
{
    // root
    // |-a
    // | |-1
    // | | |-1
    // | |
    // | |-2
    // | | |-1
    // | |
    // | |-3
    // |
    // |-b
    // | |-1
    // | | |-1
    // | | |-2
    // | |
    // | |-2
    // | | |-1
    // | |
    // | |-3
    // |
    // |-c
    // | |-1
    // | |-2
    // | |-3

    UIControl* root = nullptr;
    UIControl* a = nullptr;
    UIControl* a1 = nullptr;
    UIControl* a2 = nullptr;
    UIControl* a3 = nullptr;
    UIControl* a11 = nullptr;
    UIControl* a21 = nullptr;

    UIControl* b = nullptr;
    UIControl* b1 = nullptr;
    UIControl* b2 = nullptr;
    UIControl* b3 = nullptr;
    UIControl* b11 = nullptr;
    UIControl* b12 = nullptr;
    UIControl* b21 = nullptr;

    UIControl* c = nullptr;
    UIControl* c1 = nullptr;
    UIControl* c2 = nullptr;
    UIControl* c3 = nullptr;

    UIControl* MakeRoot(const char* name)
    {
        UIControl* c = new UIControl();
        c->SetName(name);
        return c;
    }

    UIControl* Child(UIControl * parent, const char* name)
    {
        UIControl* c = new UIControl();
        c->SetName(name);
        parent->AddControl(c);
        return c;
    }

    void SetUp(const String& testName) override
    {
        root = MakeRoot("root");

        a = Child(root, "a");
        a1 = Child(a, "1");
        a2 = Child(a, "2");
        a3 = Child(a, "3");
        a11 = Child(a1, "1");
        a21 = Child(a2, "1");

        b = Child(root, "b");
        b1 = Child(b, "1");
        b2 = Child(b, "2");
        b3 = Child(b, "3");
        b11 = Child(b1, "1");
        b12 = Child(b1, "2");
        b21 = Child(b2, "1");

        c = Child(root, "c");
        c1 = Child(c, "1");
        c2 = Child(c, "2");
        c3 = Child(c, "3");
    }

    void TearDown(const String& testName) override
    {
        SafeRelease(root);
        SafeRelease(a);
        SafeRelease(a1);
        SafeRelease(a2);
        SafeRelease(a3);
        SafeRelease(a11);
        SafeRelease(a21);

        SafeRelease(b);
        SafeRelease(b1);
        SafeRelease(b2);
        SafeRelease(b3);
        SafeRelease(b11);
        SafeRelease(b12);
        SafeRelease(b21);

        SafeRelease(c);
        SafeRelease(c1);
        SafeRelease(c2);
        SafeRelease(c3);
    }

    // UIControl::FindByName
    DAVA_TEST(FindThemSelves)
    {
        TEST_VERIFY(root->FindByPath(".") == root);
        TEST_VERIFY(b->FindByPath(".") == b);
    }

    // UIControl::FindByName
    DAVA_TEST(FindLocalRoot)
    {
        TEST_VERIFY(root->FindByPath("^") == nullptr);
        TEST_VERIFY(c1->FindByPath("^") == nullptr);

        UIControlPackageContext* context = new UIControlPackageContext();
        root->SetPackageContext(context);
        SafeRelease(context);

        TEST_VERIFY(root->FindByPath("^") == nullptr);
        TEST_VERIFY(c1->FindByPath("^") == root);
        TEST_VERIFY(b21->FindByPath("^") == root);
    }

    // UIControl::FindByName
    DAVA_TEST(FindParent)
    {
        TEST_VERIFY(root->FindByPath("..") == nullptr);
        TEST_VERIFY(c->FindByPath("..") == root);
        TEST_VERIFY(c1->FindByPath("..") == c);
        TEST_VERIFY(b21->FindByPath("..") == b2);
    }

    // UIControl::FindByName
    DAVA_TEST(MatchesOneLevel)
    {
        TEST_VERIFY(root->FindByPath("*/1") == a1);
        TEST_VERIFY(root->FindByPath("*/*/1") == a11);
        TEST_VERIFY(root->FindByPath("*/a") == nullptr);
        TEST_VERIFY(root->FindByPath("*/root") == nullptr);
    }

    // UIControl::FindByName
    DAVA_TEST(MatchesZeroOrMoreLevels)
    {
        TEST_VERIFY(root->FindByPath("**/1") == a1);
        TEST_VERIFY(root->FindByPath("**/a") == a);
        TEST_VERIFY(root->FindByPath("**/root") == nullptr);
        TEST_VERIFY(root->FindByPath("**/1/1") == a11);
    }

    // UIControl::FindByName
    DAVA_TEST(FindSomePatches)
    {
        TEST_VERIFY(root->FindByPath("b/2/1") == b21);
        TEST_VERIFY(b11->FindByPath("../..") == b);
        TEST_VERIFY(b11->FindByPath("../2/../../2/1") == b21);
    }
};