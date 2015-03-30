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

#include "Render/OGLHelpers.h"

#include "Core/Core.h"


#if defined(__DAVAENGINE_OPENGL__)
namespace DAVA
{
    
int32 GetHalfFloatID()
{
#if RHI_COMPLETE
#if defined(__DAVAENGINE_ANDROID__)
    return GL_HALF_FLOAT_OES;
#elif defined(__DAVAENGINE_IPHONE__)
    const Core::eRenderer renderer = RenderManager::Instance()->GetRenderer();
    if((Core::RENDERER_OPENGL_ES_2_0 == renderer) || (Core::RENDERER_OPENGL_ES_1_0 == renderer))
    {
        return GL_HALF_FLOAT_OES;
    }
    else
    {
        return GL_HALF_FLOAT;
    }
#else
    return GL_HALF_FLOAT;
#endif

#else
    return 0;
#endif //RHI_COMPLETE
}
    
};
#endif // #if defined(__DAVAENGINE_OPENGL__)

