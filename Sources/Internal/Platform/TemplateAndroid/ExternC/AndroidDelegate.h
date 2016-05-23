#ifndef __ANDROID_DELEGATE_H__
#define __ANDROID_DELEGATE_H__

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_ANDROID__)

#include <jni.h>
#include "Platform/TemplateAndroid/CorePlatformAndroid.h"

class AndroidDelegate : public DAVA::AndroidSystemDelegate
{
    enum eConst
    {
        MAX_PATH_SZ = 260
    };

    char httpDownloaderName[MAX_PATH_SZ];

public:
    AndroidDelegate(JavaVM* vm);

    virtual bool DownloadHttpFile(const DAVA::String& url, const DAVA::String& documentsPathname);
};

#endif //#if defined(__DAVAENGINE_ANDROID__)

#endif //#ifndef __ANDROID_LISTENER_H__
