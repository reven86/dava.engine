#if defined(__DAVAENGINE_COREV2__)

#include "Engine/Private/iOS/Window/ViewControlleriOS.h"

#if defined(__DAVAENGINE_QT__)
// TODO: plarform defines
#elif defined(__DAVAENGINE_IPHONE__)

#include "Engine/Private/iOS/Window/WindowNativeBridgeiOS.h"

@implementation ViewControlleriOS

- (id)init:(DAVA::Private::WindowNativeBridgeiOS*)nativeBridge
{
    [super init];
    bridge = nativeBridge;
    return self;
}

- (void)loadView
{
    bridge->loadView();
}

- (void)viewWillTransitionToSize:(CGSize)size withTransitionCoordinator:(id<UIViewControllerTransitionCoordinator>)coordinator
{
    bridge->viewWillTransitionToSize(size.width, size.height);
    [super viewWillTransitionToSize:size withTransitionCoordinator:coordinator];
}

@end

#endif // __DAVAENGINE_IPHONE__
#endif // __DAVAENGINE_COREV2__
