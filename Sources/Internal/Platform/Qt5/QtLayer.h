#ifndef __DAVAENGINE_QT_LAYER_H__
#define __DAVAENGINE_QT_LAYER_H__

#include "Base/Singleton.h"
#include "Base/BaseTypes.h"

#include "UI/UIEvent.h"

namespace DAVA
{
class QtLayerDelegate
{
public:
    virtual ~QtLayerDelegate()
    {
    }

    virtual void Quit() = 0;
};

class QtLayer
: public Singleton<QtLayer>
{
public:
    QtLayer();
    virtual ~QtLayer();

    void OnSuspend();
    void OnResume();

    void AppStarted();
    void AppFinished();

    void Resize(int32 width, int32 height, float64 dpr);
    void ProcessFrame();

    void* CreateAutoreleasePool();
    void ReleaseAutoreleasePool(void* pool);

    void Quit();
    void SetDelegate(QtLayerDelegate* delegate);

    bool IsDAVAEngineEnabled() const
    {
        return isDAVAEngineEnabled;
    };

    void KeyPressed(Key key, uint64 timestamp);
    void KeyReleased(Key key, uint64 timestamp);

    void MouseEvent(const UIEvent& event);

#ifdef __DAVAENGINE_MACOS__
    static void MakeAppForeground(bool foreground = true);
    static void RestoreMenuBar();
#endif

private:
    QtLayerDelegate* delegate;
    bool isDAVAEngineEnabled;
};
}


#endif // __DAVAENGINE_QT_LAYER_H__
