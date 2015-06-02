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


#include "FakeOcclusionQuery.h"

#if (__ANDROID_API__ < 18)

#include <EGL/egl.h>

void glGenQueries_Fake(GLsizei n, GLuint* ids)
{
	DAVA::Logger::Error("calling fake implementation of glGenQueries");
}

void glDeleteQueries_Fake(GLsizei n, const GLuint* ids)
{
	DAVA::Logger::Error("calling fake implementation of glDeleteQueries");
}

void glBeginQuery_Fake(GLenum target, GLuint id)
{
	DAVA::Logger::Error("calling fake implementation of glBeginQuery");
}

void glEndQuery_Fake(GLenum target)
{
	DAVA::Logger::Error("calling fake implementation of glEndQuery");
}

void glGetQueryObjectuiv_Fake(GLuint id, GLenum pname, GLuint* params)
{
	DAVA::Logger::Error("calling fake implementation of glGetQueryObjectuiv");
}

void InitFakeOcclusion()
{
	glGenQueries = NULL;
	glDeleteQueries = NULL;
	glBeginQuery = NULL;
	glEndQuery = NULL;
	glGetQueryObjectuiv = NULL;

	glGenQueries = (GL_GEN_QUERIES) eglGetProcAddress("glGenQueries");
	glDeleteQueries = (GL_DELETE_QUERIES) eglGetProcAddress("glDeleteQueries");
	glBeginQuery = (GL_BEGIN_QUERY) eglGetProcAddress("glBeginQuery");
	glEndQuery = (GL_END_QUERY) eglGetProcAddress("glEndQuery");
	glGetQueryObjectuiv = (GL_GET_QUERY_OBJECTUIV) eglGetProcAddress("glGetQueryObjectuiv");

	if (!glGenQueries    ||
		!glDeleteQueries ||
		!glBeginQuery    ||
		!glEndQuery      ||
		!glGetQueryObjectuiv)
	{
		DAVA::Logger::Error("Replace occlulsion with fake implementation");
		glGenQueries = &glGenQueries_Fake;
		glDeleteQueries = &glDeleteQueries_Fake;
		glBeginQuery = &glBeginQuery_Fake;
		glEndQuery = &glEndQuery_Fake;
		glGetQueryObjectuiv = &glGetQueryObjectuiv_Fake;
	}
}

#endif //if (__ANDROID_API__ < 18)
