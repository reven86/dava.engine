#include "GlobalContext.h"

#include "core_common/platform_env.hpp"

namespace NGTLayer
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
} // namespace NGTLayer