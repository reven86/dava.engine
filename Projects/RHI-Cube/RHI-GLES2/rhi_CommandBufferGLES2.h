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


#ifndef __RHI_COMMANDBUFFERGLES2_H__
#define __RHI_COMMANDBUFFERGLES2_H__

    #include "../rhi_Pool.h"
    
//-    #include "Base/BaseTypes.h"
//-    using DAVA::uint32;


namespace rhi
{

class
CommandBuffer_t
{
public:
                CommandBuffer_t();
                ~CommandBuffer_t();

    void        begin();
    void        end();
    void        replay();

    void        command( uint32 cmd );
    void        command( uint32 cmd, uint32 arg1 );
    void        command( uint32 cmd, uint32 arg1, uint32 arg2 );
    void        command( uint32 cmd, uint32 arg1, uint32 arg2, uint32 arg3 );
    void        command( uint32 cmd, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4 );
    void        command( uint32 cmd, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4, uint32 arg5 );
    void        command( uint32 cmd, uint32 arg1, uint32 arg2, uint32 arg3, uint32 arg4, uint32 arg5, uint32 arg6 );


private:

    static const uint32   EndCmd = 0xFFFFFFFF;

    std::vector<uint32> _cmd;
};

typedef Pool<CommandBuffer_t>   CommandBufferPool;


} // namespace rhi


#endif // __RHI_COMMANDBUFFERGLES2_H__

 