#include "PreProcess.h"

#include "../rhi_Type.h"
#include "rhi_Utils.h"

#include "MCPP/mcpp_lib.h"

#include <stdio.h>
#include <stdarg.h>

#define RHI_SHADER_SOURCE_BUFFER_SIZE (64 << 10)

ShaderPreprocessScope::ShaderPreprocessScope()
{
    mcpp__startup();
}

ShaderPreprocessScope::~ShaderPreprocessScope()
{
    mcpp__shutdown();
}

void PreProcessText(const char* text, const char** arg, unsigned argCount, std::string* result)
{
    DVASSERT(text);
    const char* argv[128];
    int argc = 0;
    DVASSERT(argCount < countof(argv) - 2);

    argv[argc++] = "<mcpp>"; // we just need first arg
    argv[argc++] = "-P"; // do not output #line directives
    // it doesn't work as desired with '//' style comments (commented block inserted BEFORE non-commented text)
    //  argv[argc++] = "-C";    // keep comments
    for (const char **a = arg, **a_end = arg + argCount; a != a_end; ++a)
        argv[argc++] = *a;
    argv[argc++] = MCPP_Text;

    char localBuffer[RHI_SHADER_SOURCE_BUFFER_SIZE] = {};
    _mcpp_preprocessed_text.buffer = localBuffer;
    _mcpp_preprocessed_text.pos = 0;
    {
        mcpp__set_input(text, static_cast<unsigned>(strlen(text)));
        mcpp_set_out_func(&mcpp_fputc_impl, &mcpp_fputs_impl, &mcpp_fprintf_impl);
        mcpp_lib_main(argc, const_cast<char**>(argv));
        mcpp__cleanup();
    }
    *result = std::string(localBuffer);
    _mcpp_preprocessed_text.buffer = nullptr;
    _mcpp_preprocessed_text.pos = 0;
}

//------------------------------------------------------------------------------

void SetPreprocessCurFile(const char* filename)
{
    mcpp__set_cur_file(filename);
}
