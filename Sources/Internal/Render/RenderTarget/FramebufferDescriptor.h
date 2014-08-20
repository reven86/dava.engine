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


#ifndef __DAVAENGINE_FRAMEBUFFERDESCRIPTOR_H__
#define __DAVAENGINE_FRAMEBUFFERDESCRIPTOR_H__

#include "Base/BaseTypes.h"
#include "Base/BaseMath.h"
#include "Base/BaseObject.h"

namespace DAVA
{

class FramebufferDescriptor
{
public:

    enum FramebufferType
    {
        FRAMEBUFFER_INVALID = 0,

        FRAMEBUFFER_COLOR0 = 1,
        FRAMEBUFFER_COLOR1,
        FRAMEBUFFER_COLOR2,
        FRAMEBUFFER_COLOR3,

        FRAMEBUFFER_DEPTH,
        FRAMEBUFFER_STENCIL,

        FRAMEBUFFER_COUNT
    };

    enum FramebufferFormat
    {
        FORMAT_INVALID = 0,
        FORMAT_RGBA8888 = 1,
        FORMAT_A8,
        FORMAT_A16,
        FORMAT_RGBA16161616,

        FORMAT_DEPTH16,
        FORMAT_DEPTH24,
        FORMAT_DEPTH32,

        FORMAT_STENCIL8,

        FORMAT_COUNT
    };

    enum PreRenderAction
    {
        PRE_ACTION_DONTCARE = 1,
        PRE_ACTION_CLEAR = 2,
        PRE_ACTION_LOAD = 3
    };

    enum PostRenderAction
    {
        POST_ACTION_DONTCARE = 1,
        POST_ACTION_STORE = 2,
        POST_ACTION_RESOLVE = 3
    };

public:

    FramebufferDescriptor();
    FramebufferDescriptor(FramebufferDescriptor::FramebufferType type,
                          FramebufferDescriptor::FramebufferFormat format,
                          FramebufferDescriptor::PreRenderAction beforeRender,
                          FramebufferDescriptor::PostRenderAction afterRender,
                          uint32 width,
                          uint32 height);

    void SetFramebufferType(FramebufferDescriptor::FramebufferType type);
    FramebufferDescriptor::FramebufferType GetFramebufferType() const;

    void SetFramebufferFormat(FramebufferDescriptor::FramebufferFormat format);
    FramebufferDescriptor::FramebufferFormat GetFramebufferFormat() const;

    void SetFramebufferWidth(uint32 width);
    uint32 GetFramebufferWidth() const;

    void SetFramebufferHeight(uint32 height);
    uint32 GetFramebufferHeight() const;

    void SetPreRenderAction(FramebufferDescriptor::PreRenderAction action);
    FramebufferDescriptor::PreRenderAction GetPreRenderAction() const;

    void SetPostRenderAction(FramebufferDescriptor::PostRenderAction action);
    FramebufferDescriptor::PostRenderAction GetPostRenderAction() const;

    bool IsValid() const;

protected:

    FramebufferType framebufferType;
    FramebufferFormat framebufferFormat;
    uint32 framebufferHeight;
    uint32 framebufferWidth;

    PreRenderAction preRenderAction;
    PostRenderAction postRenderAction;
};

};

#endif /* defined(__DAVAENGINE_FRAMEBUFFERDESCRIPTOR_H__) */
