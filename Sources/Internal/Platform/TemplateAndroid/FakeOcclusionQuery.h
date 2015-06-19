/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __DAVAENGINE_FAKE_OCCLUSION_H__
#define __DAVAENGINE_FAKE_OCCLUSION_H__

#include "Base/BaseTypes.h"
#if defined(__DAVAENGINE_ANDROID__)

#include <android/api-level.h>

#if (__ANDROID_API__ < 18)
#include "Render/RenderBase.h"


typedef void (*GL_GEN_QUERIES) (GLsizei n, GLuint* ids);
typedef void (*GL_DELETE_QUERIES) (GLsizei n, const GLuint* ids);
typedef void (*GL_BEGIN_QUERY) (GLenum target, GLuint id);
typedef void (*GL_END_QUERY) (GLenum target);
typedef void (*GL_GET_QUERY_OBJECTUIV) (GLuint id, GLenum pname, GLuint* params);

static GL_GEN_QUERIES glGenQueries;
static GL_DELETE_QUERIES glDeleteQueries;
static GL_BEGIN_QUERY glBeginQuery;
static GL_END_QUERY glEndQuery;
static GL_GET_QUERY_OBJECTUIV glGetQueryObjectuiv;

#define GL_ANY_SAMPLES_PASSED                            0x8C2F
#define GL_QUERY_RESULT                                  0x8866
#define GL_QUERY_RESULT_AVAILABLE                        0x8867

extern void glGenQueries_Fake(GLsizei n, GLuint* ids);
extern void glDeleteQueries_Fake(GLsizei n, const GLuint* ids);
extern void glBeginQuery_Fake(GLenum target, GLuint id);
extern void glEndQuery_Fake(GLenum target);
extern void glGetQueryObjectuiv_Fake(GLuint id, GLenum pname, GLuint* params);

extern void InitFakeOcclusion();

#endif //#if __ANDROID_API__ < 18

#endif //#if defined(__DAVAENGINE_ANDROID__)

#endif /* defined(__DAVAENGINE_FAKE_OCCLUSION_H__) */
