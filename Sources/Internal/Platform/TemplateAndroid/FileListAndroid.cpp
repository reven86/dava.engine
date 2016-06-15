#include "FileListAndroid.h"
#include "Logger/Logger.h"
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
    JNIEnv* env = JNI::GetEnv();
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
            jstring jName = (jstring)env->GetObjectField(item, jNameField);

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

} //namespace DAVA
