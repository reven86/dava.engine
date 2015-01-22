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

#include "SharedPreferences.h"

namespace DAVA
{

#if defined(__DAVAENGINE_ANDROID__)

SharedPreferences::SharedPreferences()
    : jniSharedPreferences("com/dava/framework/JNISharedPreferences")
{
    getSharedPreferences = jniSharedPreferences.GetStaticMethod<jobject, jstring, jint>("GetSharedPreferences");

    String name = "SharedPreferences";
    JNIEnv *env = JNI::GetEnv();
    jstring jname = env->NewStringUTF(name.c_str());
    jobject tmp = getSharedPreferences(jname, 0);
    env->DeleteLocalRef(jname);
    preferencesObject = env->NewGlobalRef(tmp);
    env->DeleteLocalRef(tmp);


    auto tempPutString = jniSharedPreferences.GetMethod<void, jstring, jstring>("PutString");
    putString = Bind(tempPutString, preferencesObject, _1, _2);

    auto tempGetString = jniSharedPreferences.GetMethod<jstring, jstring, jstring>("GetString");
    getString = Bind(tempGetString, preferencesObject, _1, _2);

    auto tempRemove = jniSharedPreferences.GetMethod<void, jstring>("Remove");
    remove = Bind(tempRemove, preferencesObject, _1);

    auto tempClear = jniSharedPreferences.GetMethod<void>("Clear");
    clear = Bind(tempClear, preferencesObject);

    auto tempPush = jniSharedPreferences.GetMethod<void>("Push");
    push = Bind(tempPush, preferencesObject);

}

SharedPreferences::~SharedPreferences()
{
    JNI::GetEnv()->DeleteGlobalRef(preferencesObject);
}

String SharedPreferences::GetEntryValue(const String &key)
{
    Logger::FrameworkDebug("Trying to Get value for %s key", key.c_str());

    JNIEnv *env = JNI::GetEnv();

    jstring jkey = env->NewStringUTF(key.c_str());
    jstring jdefvalue = env->NewStringUTF("");

    jstring jvalue = getString(jkey, jdefvalue);

    String retValue;
    JNI::CreateStringFromJni(env, jvalue, retValue);
    env->DeleteLocalRef(jkey);
    env->DeleteLocalRef(jdefvalue);

    return retValue;
}

void SharedPreferences::SetEntryValue(const String &key, const String &value)
{
    Logger::FrameworkDebug("Trying to set %s value for %s key", value.c_str(), key.c_str());
    JNIEnv *env = JNI::GetEnv();

    jstring jkey = env->NewStringUTF(key.c_str());
    jstring jvalue = env->NewStringUTF(value.c_str());

    putString(jkey, jvalue);

    env->DeleteLocalRef(jkey);
    env->DeleteLocalRef(jvalue);
}

void SharedPreferences::RemoveEntry(const String &key)
{
    JNIEnv *env = JNI::GetEnv();
    jstring jkey = env->NewStringUTF(key.c_str());
    remove(jkey);
    env->DeleteLocalRef(jkey);
}

void SharedPreferences::Clear()
{
    clear();
}

void SharedPreferences::Push()
{
    push();
}

#endif

}
