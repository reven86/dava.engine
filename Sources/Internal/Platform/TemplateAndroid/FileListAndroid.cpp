#include "FileListAndroid.h"
#include "Logger/Logger.h"
#include "Platform/TemplateAndroid/ExternC/AndroidLayer.h"

namespace DAVA
{
JniFileList::JniFileList()
    : jniFileList("com/dava/framework/JNIFileList")
{
    getFileList = jniFileList.GetStaticMethod<jobjectArray, jstring>("GetFileList");
}

Vector<JniFileList::JniFileListEntry> JniFileList::GetFileList(const String& path)
{
    Vector<JniFileList::JniFileListEntry> fileList;
    JNIEnv* env = JNI::GetEnv();
    JNI::LocalRef<jstring> jPath = env->NewStringUTF(path.c_str());

    JNI::LocalRef<jobjectArray> jArray = getFileList(jPath);
    if (jArray)
    {
        jsize size = env->GetArrayLength(jArray);
        for (jsize i = 0; i < size; ++i)
        {
            JNI::LocalRef<jobject> item = env->GetObjectArrayElement(jArray, i);

            JNI::LocalRef<jclass> cls = env->GetObjectClass(item);
#if defined(__DAVAENGINE_COREV2__)
            jfieldID jNameField = env->GetFieldID(cls, "name", JNI::TypeSignature<jstring>::value());
            jfieldID jSizeField = env->GetFieldID(cls, "size", JNI::TypeSignature<jlong>::value());
            jfieldID jIsDirectoryField = env->GetFieldID(cls, "isDirectory", JNI::TypeSignature<jboolean>::value());
#else
            jfieldID jNameField = env->GetFieldID(cls, "name", JNI::TypeMetrics<jstring>());
            jfieldID jSizeField = env->GetFieldID(cls, "size", JNI::TypeMetrics<jlong>());
            jfieldID jIsDirectoryField = env->GetFieldID(cls, "isDirectory", JNI::TypeMetrics<jboolean>());
#endif

            jlong jSize = env->GetLongField(item, jSizeField);
            jboolean jIsDir = env->GetBooleanField(item, jIsDirectoryField);
            JNI::LocalRef<jstring> jName = static_cast<jstring>(env->GetObjectField(item, jNameField));

            JniFileListEntry entry;
            entry.name = JNI::ToString(jName);
            entry.size = jSize;
            entry.isDirectory = jIsDir;
            fileList.push_back(entry);
        }
    }

    return fileList;
}

} //namespace DAVA
