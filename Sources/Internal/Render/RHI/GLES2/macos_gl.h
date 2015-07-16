#if !defined __MACOS_GL_H__
#define __MACOS_GL_H__

void macos_gl_init(void * glView);
void macos_gl_end_frame();
void macos_gl_acquire_context();
void macos_gl_release_context();

#endif
