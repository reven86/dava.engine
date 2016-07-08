#ifndef __DAVAENGINE_CORE_PLATFORM_ANDROID_H__
#define __DAVAENGINE_CORE_PLATFORM_ANDROID_H__

#include "DAVAEngine.h"
#if defined(__DAVAENGINE_ANDROID__)

#include <android/asset_manager.h>
#include <android/asset_manager_jni.h>

namespace DAVA
{
class AndroidSystemDelegate
{
public:
    AndroidSystemDelegate(JavaVM* vm);
    virtual ~AndroidSystemDelegate() = default;

    virtual bool DownloadHttpFile(const String& url, const String& documentsPathname) = 0;

    JNIEnv* GetEnvironment() const
    {
        return environment;
    };
    JavaVM* GetVM() const
    {
        return vm;
    };

protected:
    JNIEnv* environment;
    JavaVM* vm;
};

class Thread;
class CorePlatformAndroid : public Core
{
public:
    CorePlatformAndroid(const DAVA::String& cmdLine);

    virtual void CreateAndroidWindow(const char8* docPathEx, const char8* docPathIn, const char8* assets, const char8* logTag, AndroidSystemDelegate* sysDelegate);

    void Quit() override;

    void RenderReset(int32 w, int32 h);
    void ProcessFrame();

    // called from Activity and manage a visible lifetime
    void StartVisible();
    void StopVisible();

    void StartForeground();
    void StopForeground(bool isLock);

    void OnCreateActivity();
    void OnDestroyActivity();

    void KeyUp(int32 keyCode);
    void KeyDown(int32 keyCode);

    void OnInput(Vector<UIEvent>& allInputs);
    void OnGamepadElement(int32 elementKey, float32 value, bool isKeycode);

    void OnGamepadAvailable(bool isAvailable);
    void OnGamepadTriggersAvailable(bool isAvailable);

    bool IsMultitouchEnabled();

    bool DownloadHttpFile(const String& url, const String& documentsPathname);

    AAssetManager* GetAssetManager();
    void SetAssetManager(AAssetManager* mngr);

    const String& GetExternalStoragePathname() const
    {
        return externalStorage;
    };
    const String& GetInternalStoragePathname() const
    {
        return internalStorage;
    };

    AndroidSystemDelegate* GetAndroidSystemDelegate() const;

    int32 GetViewWidth() const;
    int32 GetViewHeight() const;

    void SetNativeWindow(void* nativeWindow);

private:
    void QuitAction();
    void ProcessWithoutDrawing();

    void ApplyPendingViewSize();

private:
    int32 pendingWidth = 0;
    int32 pendingHeight = 0;
    int32 backbufferWidth = 0;
    int32 backbufferHeight = 0;

    bool wasCreated = false;
    bool renderIsActive = false;
    bool viewSizeChanged = false;

    bool foreground = false;

    AndroidSystemDelegate* androidDelegate = nullptr;

    String externalStorage;
    String internalStorage;
};
};
#endif // #if defined(__DAVAENGINE_ANDROID__)
#endif // __DAVAENGINE_CORE_PLATFORM_ANDROID_H__
