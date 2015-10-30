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

    #include "../Common/rhi_Private.h"
    #include "../Common/rhi_Pool.h"
    #include "rhi_DX9.h"

    #include "Debug/DVAssert.h"
    #include "FileSystem/Logger.h"
using DAVA::Logger;

    #include "_dx9.h"

namespace rhi
{
//==============================================================================

struct
DepthStencilStateDX9_t
{
    uint32 depthTestEnabled : 1;
    uint32 depthWriteEnabled : 1;
    DWORD depthFunc;

    uint32 stencilEnabled : 1;
    uint32 stencilTwoSided : 1;

    struct
    {
        DWORD func;
        DWORD readMask;
        DWORD writeMask;
        DWORD refValue;
        DWORD failOperation;
        DWORD depthFailOperation;
        DWORD depthStencilPassOperation;
    } stencilFront, stencilBack;
};

typedef ResourcePool<DepthStencilStateDX9_t, RESOURCE_DEPTHSTENCIL_STATE, DepthStencilState::Descriptor, false> DepthStencilStateDX9Pool;
RHI_IMPL_POOL(DepthStencilStateDX9_t, RESOURCE_DEPTHSTENCIL_STATE, DepthStencilState::Descriptor, false);

//------------------------------------------------------------------------------

static DWORD
_CmpFunc(CmpFunc func)
{
    DWORD f = D3DCMP_ALWAYS;

    switch (func)
    {
    case CMP_NEVER:
        f = D3DCMP_NEVER;
        break;
    case CMP_LESS:
        f = D3DCMP_LESS;
        break;
    case CMP_EQUAL:
        f = D3DCMP_EQUAL;
        break;
    case CMP_LESSEQUAL:
        f = D3DCMP_LESSEQUAL;
        break;
    case CMP_GREATER:
        f = D3DCMP_GREATER;
        break;
    case CMP_NOTEQUAL:
        f = D3DCMP_NOTEQUAL;
        break;
    case CMP_GREATEREQUAL:
        f = D3DCMP_GREATEREQUAL;
        break;
    case CMP_ALWAYS:
        f = D3DCMP_ALWAYS;
        break;
    }

    return f;
}

//------------------------------------------------------------------------------

static DWORD
_StencilOp(StencilOperation op)
{
    DWORD s = D3DSTENCILOP_KEEP;

    switch (op)
    {
    case STENCILOP_KEEP:
        s = D3DSTENCILOP_KEEP;
        break;
    case STENCILOP_ZERO:
        s = D3DSTENCILOP_ZERO;
        break;
    case STENCILOP_REPLACE:
        s = D3DSTENCILOP_REPLACE;
        break;
    case STENCILOP_INVERT:
        s = D3DSTENCILOP_INVERT;
        break;
    case STENCILOP_INCREMENT_CLAMP:
        s = D3DSTENCILOP_INCRSAT;
        break;
    case STENCILOP_DECREMENT_CLAMP:
        s = D3DSTENCILOP_DECRSAT;
        break;
    case STENCILOP_INCREMENT_WRAP:
        s = D3DSTENCILOP_INCR;
        break;
    case STENCILOP_DECREMENT_WRAP:
        s = D3DSTENCILOP_DECR;
        break;
    }

    return s;
}

//==============================================================================

static Handle
dx9_DepthStencilState_Create(const DepthStencilState::Descriptor& desc)
{
    Handle handle = DepthStencilStateDX9Pool::Alloc();
    DepthStencilStateDX9_t* state = DepthStencilStateDX9Pool::Get(handle);

    state->depthTestEnabled = desc.depthTestEnabled;
    state->depthWriteEnabled = desc.depthWriteEnabled;
    state->depthFunc = _CmpFunc(CmpFunc(desc.depthFunc));

    state->stencilEnabled = desc.stencilEnabled;
    state->stencilTwoSided = desc.stencilTwoSided;

    state->stencilFront.func = _CmpFunc(CmpFunc(desc.stencilFront.func));
    state->stencilFront.readMask = desc.stencilFront.readMask;
    state->stencilFront.writeMask = desc.stencilFront.writeMask;
    state->stencilFront.refValue = desc.stencilFront.refValue;
    state->stencilFront.failOperation = _StencilOp(StencilOperation(desc.stencilFront.failOperation));
    state->stencilFront.depthFailOperation = _StencilOp(StencilOperation(desc.stencilFront.depthFailOperation));
    state->stencilFront.depthStencilPassOperation = _StencilOp(StencilOperation(desc.stencilFront.depthStencilPassOperation));

    state->stencilBack.func = _CmpFunc(CmpFunc(desc.stencilBack.func));
    state->stencilBack.readMask = desc.stencilBack.readMask;
    state->stencilBack.writeMask = desc.stencilBack.writeMask;
    state->stencilBack.refValue = desc.stencilBack.refValue;
    state->stencilBack.failOperation = _StencilOp(StencilOperation(desc.stencilBack.failOperation));
    state->stencilBack.depthFailOperation = _StencilOp(StencilOperation(desc.stencilBack.depthFailOperation));
    state->stencilBack.depthStencilPassOperation = _StencilOp(StencilOperation(desc.stencilBack.depthStencilPassOperation));

    return handle;
}

//------------------------------------------------------------------------------

static void
dx9_DepthStencilState_Delete(Handle state)
{
    DepthStencilStateDX9Pool::Free(state);
}

//==============================================================================

namespace DepthStencilStateDX9
{
void SetupDispatch(Dispatch* dispatch)
{
    dispatch->impl_DepthStencilState_Create = &dx9_DepthStencilState_Create;
    dispatch->impl_DepthStencilState_Delete = &dx9_DepthStencilState_Delete;
}

void SetToRHI(Handle hstate)
{
    DepthStencilStateDX9_t* state = DepthStencilStateDX9Pool::Get(hstate);

    _D3D9_Device->SetRenderState(D3DRS_ZENABLE, state->depthTestEnabled);
    _D3D9_Device->SetRenderState(D3DRS_ZWRITEENABLE, state->depthWriteEnabled);
    _D3D9_Device->SetRenderState(D3DRS_ZFUNC, state->depthFunc);

    if (state->stencilEnabled)
    {
        _D3D9_Device->SetRenderState(D3DRS_STENCILENABLE, TRUE);

        _D3D9_Device->SetRenderState(D3DRS_STENCILFUNC, state->stencilFront.func);
        _D3D9_Device->SetRenderState(D3DRS_STENCILREF, state->stencilFront.refValue);
        _D3D9_Device->SetRenderState(D3DRS_STENCILMASK, state->stencilFront.readMask);
        _D3D9_Device->SetRenderState(D3DRS_STENCILWRITEMASK, state->stencilFront.writeMask);
        _D3D9_Device->SetRenderState(D3DRS_STENCILZFAIL, state->stencilFront.depthFailOperation);
        _D3D9_Device->SetRenderState(D3DRS_STENCILFAIL, state->stencilFront.failOperation);
        _D3D9_Device->SetRenderState(D3DRS_STENCILPASS, state->stencilFront.depthStencilPassOperation);

        if (state->stencilTwoSided)
        {
            _D3D9_Device->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, TRUE);

            _D3D9_Device->SetRenderState(D3DRS_CCW_STENCILFUNC, state->stencilBack.func);
            //            _D3D9_Device->SetRenderState( D3DRS_CCW_STENCILREF, state->stencilFront.refValue );
            //            _D3D9_Device->SetRenderState( D3DRS_CCW_STENCILMASK, state->stencilFront.readMask );
            //            _D3D9_Device->SetRenderState( D3DRS_CCW_STENCILWRITEMASK, state->stencilFront.writeMask );
            _D3D9_Device->SetRenderState(D3DRS_CCW_STENCILZFAIL, state->stencilBack.depthFailOperation);
            _D3D9_Device->SetRenderState(D3DRS_CCW_STENCILFAIL, state->stencilBack.failOperation);
            _D3D9_Device->SetRenderState(D3DRS_CCW_STENCILPASS, state->stencilBack.depthStencilPassOperation);
        }
        else
        {
            _D3D9_Device->SetRenderState(D3DRS_TWOSIDEDSTENCILMODE, FALSE);
        }
    }
    else
    {
        _D3D9_Device->SetRenderState(D3DRS_STENCILENABLE, FALSE);
    }
}
}

//==============================================================================
} // namespace rhi
