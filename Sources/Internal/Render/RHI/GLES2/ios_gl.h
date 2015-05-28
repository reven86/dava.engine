#ifndef __DAVAENGINE_IOSGL_H__
#define __DAVAENGINE_IOSGL_H__
/*
#include <objc/objc.h>

#if defined __objectivec 
@class CAEAGLLayer;
BOOL    ios_GL_resize_from_layer( CAEAGLLayer* layer ); 
#endif
*/
void    ios_GL_init(void * nativeLayer);
void    ios_GL_begin_frame();
void    ios_GL_end_frame();



#endif // __DAVAENGINE_IOSGL_H__
