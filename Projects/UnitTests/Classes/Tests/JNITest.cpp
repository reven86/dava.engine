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

JNITest::JNITest()
    : TestTemplate<JNITest>("JNITest")
{
    RegisterFunction(this, &JNITest::TestFunction, "JNITestTestFunctuion", NULL);
}

void JNITest::LoadResources()
{

}

void JNITest::UnloadResources()
{

}

void JNITest::TestFunction(PerfFuncData * data)
{
	JNI::JavaClass javaNotificationProvider("com/dava/framework/JNINotificationProvider");
	auto showNotificationNext = javaNotificationProvider.GetStaticMethod<void, jstring, jstring, jstring>("NotifyText");
	auto showNotificationProgress = javaNotificationProvider.GetStaticMethod<void, jstring, jstring, jstring, int, int>("NotifyProgress");

	jstring jStrTitle = CreateJString(JNI::GetEnv(), L"test");
	jstring jStrText = CreateJString(JNI::GetEnv(), L"test2");


	JNI::GetEnv()->DeleteLocalRef(jStrTitle);
	JNI::GetEnv()->DeleteLocalRef(jStrText);

    TEST_VERIFY(true == Thread::IsMainThread());
}



