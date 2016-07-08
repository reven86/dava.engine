#ifndef __DAVAENGINE_RENDERVIEWCONTROLLER_H__
#define __DAVAENGINE_RENDERVIEWCONTROLLER_H__


#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_IPHONE__)

#import "Platform/TemplateiOS/BackgroundView.h"

#import <UIKit/UIKit.h>
#import "Platform/TemplateiOS/RenderView.h"

@interface RenderViewController : UIViewController
{
    RenderView* renderView;
    BackgroundView* backgroundView;
}

@property(nonatomic, readonly) BackgroundView* backgroundView;
@property(nonatomic, readonly) RenderView* renderView;

- (GLRenderView*)createGLView;
- (MetalRenderView*)createMetalView;

@end
#endif // #if defined(__DAVAENGINE_IPHONE__)

#endif //__DAVAENGINE_RENDERVIEWCONTROLLER_H__
