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

#if defined(__DAVAENGINE_ANDROID__)

#include "Platform/TemplateAndroid/JniHelpers.h"

using namespace DAVA;

DAVA_TESTCLASS(JNITest)
{
    JNI::JavaClass javaNotificationProvider;
    Function<void (jstring, jstring, jstring, jboolean)> showNotificationText;

    JNITest()
        : javaNotificationProvider("com/dava/framework/JNINotificationProvider")
    {
        showNotificationText = javaNotificationProvider.GetStaticMethod<void, jstring, jstring, jstring, jboolean>("NotifyText");
    }

    DAVA_TEST(TestFunction)
    {
        // try to use Java Class from !Main thread.
        Thread *someThread = Thread::Create(Message(this, &JNITest::ThreadFunc));
        someThread->Start();
        while(someThread->GetState() != Thread::STATE_ENDED)
        {
            JobManager::Instance()->Update();
        }
        someThread->Join();
        someThread->Release();

        // test that we have no local ref table overflow (512 refs allowed).
        for (int i = 0; i < 1024; ++i)
        {
            JNI::JavaClass t("com/dava/framework/JNINotificationProvider");
        }

        // test calls to Java using JNITest java class
        JNIEnv *env = JNI::GetEnv();

        // get class reference
        JNI::JavaClass jtest("com/dava/unittests/JNITest");
        // get Function as Static Method for PassString
        auto passString = jtest.GetStaticMethod<jboolean, jstring>("PassString");

        // prepare data
        jstring str = JNI::CreateJString(L"TestString");

        // call Java Method
        jboolean isPassed = passString(str);

        // release data
        env->DeleteLocalRef(str);
        TEST_VERIFY(JNI_TRUE == isPassed);

        // Try to pass String Array.
        // Get Static Method
        auto passStringArray = jtest.GetStaticMethod<jint, jstringArray>("PassStringArray");

        jint stringsToPass = 5;

        // Create ObjectsArray for strings
        JNI::JavaClass stringClass("java/lang/String");
        jobjectArray stringArray = env->NewObjectArray(stringsToPass, stringClass, NULL);

        // fill array
        for (uint32 i = 0; i < stringsToPass; ++i)
        {
            jstring str = JNI::CreateJString(L"TestString");
            env->SetObjectArrayElement(stringArray, i, str);
            env->DeleteLocalRef(str);
        }
        // Call Static Method
        jint stringsPassed = passStringArray(stringArray);

        TEST_VERIFY(stringsToPass == stringsPassed);

        env->DeleteLocalRef(stringArray);

        // Try to call dinamic method for object
        str = JNI::CreateJString(L"TestString");
        // Take method to retrive some jobject
        auto notGet = jtest.GetStaticMethod<jobject> ("GetN");

        // call method and retrieve object from Java
        jobject jtestobj = notGet();

        // we suppose that retrieved object is
        JNI::JavaClass jniTestObject("com/dava/unittests/JNITestObject");
        // take dynamic method for this class
        auto out = jniTestObject.GetMethod<jboolean>("Out");

        //and call dynamic method for object.
        jboolean outres = out(jtestobj);

        TEST_VERIFY(JNI_TRUE == outres);

        env->DeleteLocalRef(str);
    }

    void ThreadFunc(BaseObject * caller, void * callerData, void * userData)
    {
        JNI::JavaClass inThreadInitedClass("com/dava/framework/JNINotificationProvider");

        auto showNotificationProgress = javaNotificationProvider.GetStaticMethod<void, jstring, jstring, jstring, int, int, jboolean>("NotifyProgress");
        auto showNotificationProgressThread = inThreadInitedClass.GetStaticMethod<void, jstring, jstring, jstring, int, int, jboolean>("NotifyProgress");

        jstring jStrTitle = JNI::CreateJString(L"test");
        jstring jStrText = JNI::CreateJString(L"test2");

        showNotificationText(jStrTitle, jStrTitle, jStrTitle, false);
        showNotificationProgressThread(jStrTitle, jStrTitle, jStrTitle, 100, 100, false);

        JNI::GetEnv()->DeleteLocalRef(jStrTitle);
        JNI::GetEnv()->DeleteLocalRef(jStrText);

        JNI::JavaClass jniText("com/dava/framework/JNITextField");
    }
};

#endif
