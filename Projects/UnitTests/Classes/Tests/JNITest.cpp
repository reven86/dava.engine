#include "DAVAEngine.h"
#include "UnitTests/UnitTests.h"

#if defined(__DAVAENGINE_ANDROID__)

#include "Platform/TemplateAndroid/JniHelpers.h"

extern "C" {
JNIEXPORT void JNICALL Java_com_dava_unittests_UnitTests_nativeCall(JNIEnv* env, jobject classthis, jint callsCount, jboolean releaseRef);
}

using namespace DAVA;

namespace
{
Function<jboolean(jobject)> out;
Function<jobject(void)> getObjectFromJava;
}

void JNICALL Java_com_dava_unittests_UnitTests_nativeCall(JNIEnv* env, jobject classthis, jint callsCount, jboolean releaseRef)
{
    static uint32 i = 0;

    Logger::Error("Call From Pure Java To Native %d", ++i);

    for (uint32 i = 0; i < static_cast<uint32>(callsCount); i++)
    {
        // call method and retrieve object from Java
        jobject jtestobj = getObjectFromJava();
        out(jtestobj);

        if (JNI_TRUE == releaseRef)
        {
            JNI::GetEnv()->DeleteLocalRef(jtestobj);
        }
    }
}

DAVA_TESTCLASS (JNITest)
{
    JNI::JavaClass jtest;
    Function<void(jint, jint, jboolean)> askJavaToCallToC;

    JNI::JavaClass javaNotificationProvider;
    Function<void(jstring, jstring, jstring, jboolean)> showNotificationText;

    JNITest()
        : javaNotificationProvider("com/dava/framework/JNINotificationProvider")
        , jtest("com/dava/unittests/JNITest")
    {
        showNotificationText = javaNotificationProvider.GetStaticMethod<void, jstring, jstring, jstring, jboolean>("NotifyText");

        // we suppose that retrieved object is
        JNI::JavaClass jniTestObject("com/dava/unittests/JNITestObject");

        out = jniTestObject.GetMethod<jboolean>("Out");
        askJavaToCallToC = jtest.GetStaticMethod<void, jint, jint, jboolean>("AskForCallsFromJava");

        // Take method to retrive some jobject
        getObjectFromJava = jtest.GetStaticMethod<jobject>("GetObject");
    }

    DAVA_TEST (TestFunction)
    {
        // try to use Java Class from !Main thread.
        Thread* someThread = Thread::Create(Message(this, &JNITest::ThreadFunc));
        someThread->Start();
        while (someThread->GetState() != Thread::STATE_ENDED)
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
        JNIEnv* env = JNI::GetEnv();

        // get Function as Static Method for PassString
        auto passString = jtest.GetStaticMethod<jboolean, jstring>("PassString");

        // prepare data
        jstring str = JNI::ToJNIString(L"TestString");

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
            jstring str = JNI::ToJNIString(L"TestString");
            env->SetObjectArrayElement(stringArray, i, str);
            env->DeleteLocalRef(str);
        }
        // Call Static Method
        jint stringsPassed = passStringArray(stringArray);

        TEST_VERIFY(stringsToPass == stringsPassed);

        env->DeleteLocalRef(stringArray);

        // Try to call dinamic method for object
        str = JNI::ToJNIString(L"TestString");

        // call method and retrieve object from Java
        jobject jtestobj = getObjectFromJava();

        //and call dynamic method for object.
        jboolean outres = out(jtestobj);

        env->DeleteLocalRef(jtestobj);

        TEST_VERIFY(JNI_TRUE == outres);

        env->DeleteLocalRef(str);
    }

    DAVA_TEST (Native_Calls)
    {
        // Call Java_com_dava_unittests_UnitTests_nativeCall from pure Java Activity.

        // 1024 times from java and each time 1 call from native to java - should work
        askJavaToCallToC(1024, 1, false);

        // 1 call from java and 256 calls from native to java - should work - 512 calls allowed.
        askJavaToCallToC(1, 256, false);

        // 1 call from java and 1024 calls from native to java - should work - true - release local ref
        askJavaToCallToC(1, 1024, true);
    }

    void ThreadFunc(BaseObject * caller, void* callerData, void* userData)
    {
        JNI::JavaClass inThreadInitedClass("com/dava/framework/JNINotificationProvider");

        auto showNotificationProgress = javaNotificationProvider.GetStaticMethod<void, jstring, jstring, jstring, int, int, jboolean>("NotifyProgress");
        auto showNotificationProgressThread = inThreadInitedClass.GetStaticMethod<void, jstring, jstring, jstring, int, int, jboolean>("NotifyProgress");

        jstring jStrTitle = JNI::ToJNIString(L"test");
        jstring jStrText = JNI::ToJNIString(L"test2");

        showNotificationText(jStrTitle, jStrTitle, jStrTitle, false);
        showNotificationProgressThread(jStrTitle, jStrTitle, jStrTitle, 100, 100, false);

        JNI::GetEnv()->DeleteLocalRef(jStrTitle);
        JNI::GetEnv()->DeleteLocalRef(jStrText);

        JNI::JavaClass jniText("com/dava/framework/JNITextField");

        // call method and retrieve object from Java
        jobject jtestobj = getObjectFromJava();

        //and call dynamic method for object.
        jboolean outres = out(jtestobj);

        TEST_VERIFY(JNI_TRUE == outres);
    }
};

#endif
