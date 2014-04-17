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

namespace DAVA
{

jclass JniFileList::gJavaClass = NULL;
const char* JniFileList::gJavaClassName = NULL;

jclass JniFileList::GetJavaClass() const
{
	return gJavaClass;
}

const char* JniFileList::GetJavaClassName() const
{
	return gJavaClassName;
}

Vector<JniFileList::JniFileListEntry> JniFileList::GetFileList(const String& path)
{
	Vector<JniFileList::JniFileListEntry> fileList;
	jmethodID mid = GetMethodID("GetFileList", "(Ljava/lang/String;)[Ljava/lang/Object;");

	if (mid)
	{
		jstring jPath = GetEnvironment()->NewStringUTF(path.c_str());

		jobjectArray jArray = (jobjectArray) GetEnvironment()->CallStaticObjectMethod(GetJavaClass(), mid, jPath);
		if (jArray)
		{
			jsize size = GetEnvironment()->GetArrayLength(jArray);
			for (jsize i = 0; i < size; ++i)
			{
				jobject item = GetEnvironment()->GetObjectArrayElement(jArray, i);

				jclass cls = GetEnvironment()->GetObjectClass(item);
				jfieldID jNameField = GetEnvironment()->GetFieldID(cls, "name", "Ljava/lang/String;");
				jfieldID jSizeField = GetEnvironment()->GetFieldID(cls, "size", "J");
				jfieldID jIsDirectoryField = GetEnvironment()->GetFieldID(cls, "isDirectory", "Z");

				jlong jSize = GetEnvironment()->GetLongField(item, jSizeField);
				jboolean jIsDir = GetEnvironment()->GetBooleanField(item, jIsDirectoryField);
				jstring jName = (jstring) GetEnvironment()->GetObjectField(item, jNameField);

				JniFileListEntry entry;
				CreateStringFromJni(GetEnvironment(), jName, entry.name);
				entry.size = jSize;
				entry.isDirectory = jIsDir;
				fileList.push_back(entry);

				GetEnvironment()->DeleteLocalRef(item);
			}
			GetEnvironment()->DeleteLocalRef(jArray);
		}

		GetEnvironment()->DeleteLocalRef(jPath);
	}

	return fileList;
}

}//namespace DAVA
