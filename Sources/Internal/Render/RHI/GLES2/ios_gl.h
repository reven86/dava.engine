#ifndef __DAVAENGINE_IOSGL_H__
#define __DAVAENGINE_IOSGL_H__

void    ios_gl_init(void * nativeLayer);
bool    ios_gl_resize_from_layer( void * nativeLayer );
void    ios_gl_begin_frame();
void    ios_gl_end_frame();
void    ios_gl_acquire_context();
void    ios_gl_release_context();
    
#endif // __DAVAENGINE_IOSGL_H__
