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

#include "GlobalContext.h"

#include "core_common/platform_env.hpp"

namespace DAVA
{
static IComponentContext* s_context = nullptr;
void SetGlobalContext(IComponentContext* context)
{
    s_context = context;

    // It's a workaround. It's need because our application is't plugin and we don't have Component Context in Enviroment.
    // It we use some code, which try to queryInterface through "global" Context::queryInterface, we will break on assert
    // Need to discuss with NGT team.
    const char ENV_VAR_NAME[] = "PLUGIN_CONTEXT_PTR";
    if (context)
    {
        auto ptr = reinterpret_cast<uintptr_t>(context);
        char buf[33] = {};
        size_t i = sizeof(buf) - 2;
        while (true)
        {
            char digit = ptr % 16;

            if (digit < 10)
            {
                buf[i] = '0' + digit;
            }
            else
            {
                buf[i] = 'a' + digit - 10;
            }

            ptr = ptr / 16;

            if (ptr == 0 || i == 0)
            {
                break;
            }

            --i;
        }

        Environment::setValue(ENV_VAR_NAME, buf + i);
    }
    else
    {
        Environment::unsetValue(ENV_VAR_NAME);
    }
}

IComponentContext* GetGlobalContext()
{
    return s_context;
}
}