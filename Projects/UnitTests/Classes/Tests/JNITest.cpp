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

#include "JNITest.h"
#include "Platform/TemplateAndroid/JniHelpers.h"

JNITest::JNITest()
    : TestTemplate<JNITest>("JNITest")
    , javaNotificationProvider("com/dava/framework/JNINotificationProvider")
{
    RegisterFunction(this, &JNITest::TestFunction, "JNITestTestFunctuion", NULL);

    showNotificationText = javaNotificationProvider.GetStaticMethod<void, jstring, jstring, jstring>("NotifyText");
}

void JNITest::LoadResources()
{

}

void JNITest::UnloadResources()
{

}

void JNITest::ThreadFunc(BaseObject * caller, void * callerData, void * userData)
{

    auto showNotificationProgress = javaNotificationProvider.GetStaticMethod<void, jstring, jstring, jstring, int, int>("NotifyProgress");

    jstring jStrTitle = JNI::CreateJString(L"test");
    jstring jStrText = JNI::CreateJString(L"test2");

    //showNotificationText(jStrTitle, jStrTitle, jStrTitle);

    JNI::GetEnv()->DeleteLocalRef(jStrTitle);
    JNI::GetEnv()->DeleteLocalRef(jStrText);


}
void JNITest::TestFunction(PerfFuncData * data)
{

    // test that we have no local ref table overflow (512 refs allowed).
    for (int i = 0; i < 1024; ++i)
    {
        JNI::JavaClass t("com/dava/framework/JNINotificationProvider");
    }

    Thread *someThread = Thread::Create(Message(this, &JNITest::ThreadFunc));
    someThread->Start();
    someThread->Join();
    someThread->Release();


    JNIEnv *env = JNI::GetEnv();
    JNI::JavaClass jtest("com/dava/unittests/JINTest");
    auto passString = jtest.GetStaticMethod<jboolean, jstring>("PassString");

    jstring str = JNI::CreateJString(L"TestString");
    jboolean isPassed = passString(str);
    env->DeleteLocalRef(str);
    TEST_VERIFY(JNI_TRUE == isPassed);

    auto passStringArray = jtest.GetStaticMethod<jint, jstringArray>("PassStringArray");

    jint stringsToPass = 5;

    JNI::JavaClass stringClass("java.lang.String");
    jobjectArray stringArray = env->NewObjectArray(stringsToPass, stringClass, NULL);

    for (uint32 i = 0; i < stringsToPass; ++i)
    {
        jstring str = JNI::CreateJString(L"TestString");
        env->SetObjectArrayElement(stringArray, i, str);
        env->DeleteLocalRef(str);
    }
    jint stringsPassed = passStringArray(stringArray);

    TEST_VERIFY(stringsToPass == stringsPassed);

    env->DeleteLocalRef(stringArray);

}



