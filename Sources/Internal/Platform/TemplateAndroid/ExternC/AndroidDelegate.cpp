#include "AndroidDelegate.h"
#include "Logger/Logger.h"

AndroidDelegate::AndroidDelegate(JavaVM* vm)
    :
    AndroidSystemDelegate(vm)
{
}

bool AndroidDelegate::DownloadHttpFile(const DAVA::String& url, const DAVA::String& documentsPathname)
{
    DAVA::Logger::Debug("[AndroidDelegate::DownloadHttpFile] url=%s", url.c_str());
    DAVA::Logger::Debug("[AndroidDelegate::DownloadHttpFile] docpath=%s", documentsPathname.c_str());

    bool retValue = false;
    if (environment)
    {
        jclass cls = environment->FindClass(httpDownloaderName);
        if (cls)
        {
            jmethodID mid = environment->GetStaticMethodID(cls, "DownloadFileFromUrl", "(Ljava/lang/String;Ljava/lang/String;)Z");

            if (mid)
            {
                jstring jstrUrl = environment->NewStringUTF(url.c_str());
                jstring jstrPath = environment->NewStringUTF(documentsPathname.c_str());

                retValue = environment->CallStaticBooleanMethod(cls, mid, jstrUrl, jstrPath);
            }
            else
            {
                DAVA::Logger::Error("[AndroidDelegate::DownloadHttpFile] Can't find method");
            }

            environment->DeleteLocalRef(cls);
        }
    }
    return retValue;
}
