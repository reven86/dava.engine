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


#include "FileListAndroid.h"
#include "FileSystem/Logger.h"
#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"
#include "Platform/TemplateAndroid/JniHelpers.h"

namespace DAVA
{

JniFileList::JniFileList()
    : jniFileList("com/dava/framework/JNIFileList")
{
    getFileList = jniFileList.GetStaticMethod<jstringArray, jstring>("GetFileList");
}

Vector<JniFileList::JniFileListEntry> JniFileList::GetFileList(const String& path)
{
	Vector<JniFileList::JniFileListEntry> fileList;
	JNIEnv *env = JNI::GetEnv();
    jstring jPath = env->NewStringUTF(path.c_str());

    jstringArray jArray = getFileList(jPath);
    if (jArray)
    {
        jsize size = env->GetArrayLength(jArray);
        for (jsize i = 0; i < size; ++i)
        {
            jobject item = env->GetObjectArrayElement(jArray, i);

            jclass cls = env->GetObjectClass(item);
            jfieldID jNameField = env->GetFieldID(cls, "name", JNI::TypeMetrics<jstring>());
            jfieldID jSizeField = env->GetFieldID(cls, "size", JNI::TypeMetrics<jlong>());
            jfieldID jIsDirectoryField = env->GetFieldID(cls, "isDirectory", JNI::TypeMetrics<jboolean>());

            jlong jSize = env->GetLongField(item, jSizeField);
            jboolean jIsDir = env->GetBooleanField(item, jIsDirectoryField);
            jstring jName = (jstring) env->GetObjectField(item, jNameField);

            JniFileListEntry entry;
            entry.name = JNI::ToString(jName);
            entry.size = jSize;
            entry.isDirectory = jIsDir;
            fileList.push_back(entry);

            env->DeleteLocalRef(item);
            env->DeleteLocalRef(cls);
            env->DeleteLocalRef(jName);
        }
        env->DeleteLocalRef(jArray);
    }

    env->DeleteLocalRef(jPath);

	return fileList;
}

}//namespace DAVA
