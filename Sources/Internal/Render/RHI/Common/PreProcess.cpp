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

    #include "PreProcess.h"

    #include "../rhi_Type.h"

    #include "MCPP/mcpp_lib.h"

    #include <stdio.h>
    #include <stdarg.h>

static std::string* _PreprocessedText = nullptr;

//------------------------------------------------------------------------------

static int
_mcpp__fputc(int ch, OUTDEST dst)
{
    switch (dst)
    {
    case MCPP_OUT:
    {
        if (_PreprocessedText)
            _PreprocessedText->push_back((char)ch);
    }
    break;

    case MCPP_ERR:
    {
    }
    break;

    case MCPP_DBG:
    {
    }
    break;

    default:
    {
    }
    }

    return ch;
}

//------------------------------------------------------------------------------

static int
_mcpp__fputs(const char* str, OUTDEST dst)
{
    switch (dst)
    {
    case MCPP_OUT:
    {
        if (_PreprocessedText)
            *_PreprocessedText += str;
    }
    break;

    case MCPP_ERR:
    {
    }
    break;

    case MCPP_DBG:
    {
    }
    break;

    default:
    {
    }
    }

    return 0;
}

//------------------------------------------------------------------------------

static int
_mcpp__fprintf(OUTDEST dst, const char* format, ...)
{
    va_list arglist;
    char buf[2048];
    int count = 0;

    va_start(arglist, format);
    count = vsnprintf(buf, countof(buf), format, arglist);
    va_end(arglist);

    switch (dst)
    {
    case MCPP_OUT:
    {
        if (_PreprocessedText)
            *_PreprocessedText += buf;
    }
    break;

    case MCPP_ERR:
    {
    }
    break;

    case MCPP_DBG:
    {
    }
    break;

    default:
    {
    }
    }

    return count;
}

//------------------------------------------------------------------------------

void PreProcessText(const char* text, std::string* result)
{
    const char* argv[] =
    {
      "<mcpp>", // we just need first arg
      "-P", // do not output #line directives
      // it doesn't work as desired with '//' style comments (commented block inserted BEFORE non-commented text)
      //        "-C",       // keep comments
      MCPP_Text
    };

    _PreprocessedText = result;
    mcpp__set_input(text, static_cast<unsigned>(strlen(text)));

    mcpp_set_out_func(&_mcpp__fputc, &_mcpp__fputs, &_mcpp__fprintf);
    mcpp_lib_main(countof(argv), (char**)argv);
    _PreprocessedText = nullptr;
    mcpp__cleanup();
}

//------------------------------------------------------------------------------

void PreProcessText(const char* text, const char** arg, unsigned argCount, std::string* result)
{
    if (text)
    {
        const char* argv[128];
        int argc = 0;
        DVASSERT(argCount < countof(argv) - 2);

        argv[argc++] = "<mcpp>"; // we just need first arg
        argv[argc++] = "-P"; // do not output #line directives
        // it doesn't work as desired with '//' style comments (commented block inserted BEFORE non-commented text)
        //        argv[argc++] = "-C";    // keep comments
        for (const char **a = arg, **a_end = arg + argCount; a != a_end; ++a)
            argv[argc++] = *a;
        argv[argc++] = MCPP_Text;

        _PreprocessedText = result;
        mcpp__set_input(text, static_cast<unsigned>(strlen(text)));

        mcpp_set_out_func(&_mcpp__fputc, &_mcpp__fputs, &_mcpp__fprintf);
        mcpp_lib_main(argc, (char**)argv);
        _PreprocessedText = nullptr;
        mcpp__cleanup();
    }
    else
    {
        *result = "";
    }
}

//------------------------------------------------------------------------------

void SetPreprocessCurFile(const char* filename)
{
    mcpp__set_cur_file(filename);
}
