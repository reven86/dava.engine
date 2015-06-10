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

#include <cstdarg>
#include <cstdio>
//#include <cmath>

#include "Base/BaseTypes.h"
#include "Debug/DVAssert.h"
//#include "Utils/Utils.h"

namespace DAVA
{

namespace
{

inline size_t FormattedLengthV(const char8* format, va_list args)
{
    DVASSERT(format != nullptr);
#if defined(__DAVAENGINE_WIN32__)
    int result = _vscprintf(format, args);
    DVASSERT(result >= 0);
    return result >= 0 ? static_cast<size_t>(result) : 0;
#else
    int result = vsnprintf(nullptr, 0, format, args);
    DVASSERT(result >= 0);
    return result >= 0 ? static_cast<size_t>(result) : 0;
#endif
}

inline size_t FormattedLengthV(const char16* format, va_list args)
{
    DVASSERT(format != nullptr);
#if defined(__DAVAENGINE_WIN32__)
    int result = _vscwprintf(format, args);
    DVASSERT(result >= 0);
    return result >= 0 ? static_cast<size_t>(result) : 0;
#else
    int result = vswprintf(nullptr, 0, format, args);
    DVASSERT(result >= 0);
    return result >= 0 ? static_cast<size_t>(result) : 0;
#endif
}

}   // unnamed namespace

String FormatVL(const char8* format, va_list args)
{
    String result;
    size_t length = 0;
    {
        va_list xargs;
        va_copy(xargs, args);   // args cannot be used twice without copying
        length = FormattedLengthV(format, xargs);
        va_end(xargs);
    }
    if (length > 0)
    {
        result.resize(length + 1);
        vsnprintf(&*result.begin(), length + 1, format, args);
        result.pop_back();
    }
    return result;
}

String Format(const char8* format, ...)
{
    va_list args;
    va_start(args, format);
    String result = FormatVL(format, args);
    va_end(args);
    return result;
}

WideString FormatVL(const char16* format, va_list args)
{
    WideString result;
    size_t length = 0;
    {
        va_list xargs;
        va_copy(xargs, args);   // args cannot be used twice without copying
        length = FormattedLengthV(format, xargs);
        va_end(xargs);
    }
    if (length > 0)
    {
        result.resize(length + 1);
#if defined(__DAVAENGINE_WIN32__)
        _vsnwprintf(&*result.begin(), length + 1, format, args);
#else
        vsnwprintf(&*result.begin(), length + 1, format, args);
#endif
        result.pop_back();
    }
    return result;
}

WideString Format(const char16* format, ...)
{
    va_list args;
    va_start(args, format);
    WideString result = FormatVL(format, args);
    va_end(args);
    return result;
}

}   // namespace DAVA

#if 0
static const int32 FORMAT_STRING_MAX_LEN = 512;

//! formatting function (use printf syntax)
String Format(const char8 * text, ...)
{
	String str;
	char8 buffer[FORMAT_STRING_MAX_LEN];

	va_list ll;
	va_start(ll, text);
	vsnprintf(buffer,  FORMAT_STRING_MAX_LEN, text, ll);
	va_end(ll);

	str = buffer;
	return str;
}

//  Format(L"") use case with WideString parameter:
//         WideString info( Format(L"%ls", tank->GetName().c_str()) ); // tank->GetName() -> WideString&
//         activeTankInfo->SetText(info);

#define ZEROPAD	1		/* pad with zero */
#define SIGN	2		/* unsigned/signed long */
#define PLUS	4		/* show plus */
#define SPACE	8		/* space if plus */
#define LEFT	16		/* left justified */
#define SPECIAL	32		/* 0x */
#define LARGE	64		/* use 'ABCDEF' instead of 'abcdef' */
#define LONG_LONG  128  /* use to convert long long '%lld' */
  
int32 do_div(int64 &n, int32 base)
{
	int32 __res;
	__res = ((unsigned long long) n) % (unsigned) base;
	n = ((unsigned long long) n) / (unsigned) base;
	return __res;
}
    
    static int32 SkipAtoi(const char16 **s)
    {
        int32 i=0;
        
        while (iswdigit(**s))
        {
            i = i*10 + *((*s)++) - L'0';
        }
        
        return i;
    }
    
    
    static char16 * Number (char16 *str, int64 num, int32 base, int32 size, int32 precision, int32 type)
    {
        const char16 *digits = L"0123456789abcdefghijklmnopqrstuvwxyz";
        if (type & LARGE)
            digits = L"0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";
        if (type & LEFT)
            type &= ~ZEROPAD;
        if (base < 2 || base > 36)
            return 0;
        
        char16 c = (type & ZEROPAD) ? L'0' : L' ';
        char16 sign = 0;
        
        if (type & SIGN)
        {
            if (num < 0)
            {
                sign = L'-';
                num = -num;
                --size;
            }
            else if (type & PLUS)
            {
                sign = L'+';
                --size;
            }
            else if (type & SPACE)
            {
                sign = L' ';
                --size;
            }
        }
        
        if (type & SPECIAL)
        {
            if (base == 16)
            {
                size -= 2;
            }
            else if (base == 8)
            {
                --size;
            }
        }
        
        char16 tmp[66];
        int32 i = 0;
        if (num == 0)
        {
            tmp[i++]='0';
        }
        else
        {
            while (num != 0)
            {
                tmp[i++] = digits[do_div(num,base)];
            }
        }
        
        if (i > precision)
        {
            precision = i;
        }
        
        size -= precision;
        if (!(type & (ZEROPAD+LEFT)))
        {
            while(size-- > 0)
            {
                *str++ = L' ';
            }
        }
        
        if (sign)
        {
            *str++ = sign;
        }
        
        if (type & SPECIAL)
        {
            if (base==8)
            {
                *str++ = L'0';
            }
            else if (base==16)
            {
                *str++ = L'0';
                *str++ = digits[33];
            }
        }
        
        if (!(type & LEFT))
        {
            while (size-- > 0)
            {
                *str++ = c;
            }
        }
        
        while (i < precision--)
        {
            *str++ = '0';
        }
        
        while (i-- > 0)
        {
            *str++ = tmp[i];
        }
        
        while (size-- > 0)
        {
            *str++ = L' ';
        }
        
        return str;
    }
    
    static char16 * Numberf (char16 *str, float64 num, int32 base, int32 size, int32 precision, int32 type)
    {
        bool isNegativeValue = false;
        if (num < 0)
        {
            isNegativeValue = true;
            num = -num;
        }
        
        int32 whole = (int32)num;
        
        num -= whole;
        
        for(int32 i = 0; i < precision; ++i)
        {
            num *= 10;
        }
        
        int32 tail = (int32)num;
        
        num -= tail;
        if (
			// tested on gcc 4.8.1, msvc2013, Apple LLVM version 6.0
#ifdef _MSC_VER
			num >= 0.5f
#else
			num > 0.5f
#endif
			)
        {
            if (precision > 0)
                tail++;
            else if (precision == 0)
                whole++;
        }

		if (tail >= pow(10.f, precision))
		{
			whole++;
			tail -= (int32)pow(10.f, precision);
		}
		
        type = SIGN | LEFT;
        char16 *firstStr = Number(str, whole, 10, -1, -1, type);
        if (isNegativeValue)
        {
            Memmove(str + 1, str, (firstStr - str) * sizeof(char16));
            *str = L'-';
            firstStr++;
        }
        if (base > 0)
        {
            while (firstStr - str < base - precision)
            {
            	Memmove(str + 1, str, (firstStr - str) * sizeof(char16));
                *str = L' ';
                firstStr++;
            }
        }
        if (precision > 0)
        {
            *firstStr++ = '.';
			precision--;
			while (pow(10.f, precision) > tail && precision > 0)
			{
				*firstStr++ = '0';
				precision--;
			}
            
            type = LEFT;
            firstStr = Number(firstStr, tail, 10, -1, -1, type);
        }
        
        return firstStr;
    }
    
    
    int32 Vsnwprintf(char16 *buf, size_t cnt, const char16 *fmt, va_list &args)
    {
        int32 len;
        int64 num;
        int32 i, base;
        const char8 *s;
        const char16 *sw;
        
        int32 flags = 0;		/* flags to number() */
        
        int32 field_width = 0;	/* width of output field */
        int32 precision = 0;		/* min. # of digits for integers; max
                                     number of chars for from string */
        int32 qualifier = 0;		/* 'h', 'l', 'L', 'w' or 'I' for integer fields */
        
        const char16 *strBase = fmt;
        char16 * str = NULL;
        for (str=buf ; *fmt ; ++fmt)
        {
            if (*fmt != L'%')
            {
                *str++ = *fmt;
                continue;
            }
            
            /* process flags */
            const char16 *floatFormatPointer = fmt;
            ++floatFormatPointer;
            
            flags = 0;
        repeat:
            ++fmt;		/* this also skips first '%' */
            
            switch (*fmt)
            {
                case L'-': flags |= LEFT; goto repeat;
                case L'+': flags |= PLUS; goto repeat;
                case L' ': flags |= SPACE; goto repeat;
                case L'#': flags |= SPECIAL; goto repeat;
                case L'0': flags |= ZEROPAD; goto repeat;
            }
            
            /* get field width */
            field_width = -1;
            if (iswdigit(*fmt))
            {
                field_width = SkipAtoi(&fmt);
            }
            else if (*fmt == L'*')
            {
                ++fmt;
                /* it's the next argument */
                field_width = va_arg(args, int32);
                if (field_width < 0)
                {
                    field_width = -field_width;
                    flags |= LEFT;
                }
            }
            
            /* get the precision */
            precision = -1;
            if (*fmt == L'.')
            {
                ++fmt;
                if (iswdigit(*fmt))
                {
                    precision = SkipAtoi(&fmt);
                }
                else if (*fmt == L'*')
                {
                    ++fmt;
                    /* it's the next argument */
                    precision = va_arg(args, int32);
                }
                
                if (precision < 0)
                {
                    precision = 0;
                }
            }
            
            /* get the conversion qualifier */
            qualifier = -1;
            if (*fmt == 'h' || *fmt == 'l' || *fmt == 'L' || *fmt == 'w')
            {
                qualifier = *fmt;
                ++fmt;
            }
            else if (*fmt == 'I' && *(fmt+1) == '6' && *(fmt+2) == '4')
            {
                qualifier = *fmt;
                fmt += 3;
            }
            
            /* default base */
            base = 10;
            
            switch (*fmt)
            {
                case L'c':
                    if (!(flags & LEFT))
                    {
                        while (--field_width > 0)
                        {
                            *str++ = L' ';
                        }
                    }
                    
                    if (qualifier == 'h')
                    {
                        *str++ = (char16) va_arg(args, int32);
                    }
                    else
                    {
                        *str++ = (char16) va_arg(args, int32);
                    }
                    
                    while (--field_width > 0)
                    {
                        *str++ = L' ';
                    }
                    
                    continue;
                    
                case L'C':
                    if (!(flags & LEFT))
                    {
                        while (--field_width > 0)
                        {
                            *str++ = L' ';
                        }
                    }
                    
                    if (qualifier == 'l' || qualifier == 'w')
                    {
                        *str++ = (char16) va_arg(args, int32);
                    }
                    else
                    {
                        *str++ = (char16) va_arg(args, int32);
                    }
                    
                    while (--field_width > 0)
                    {
                        *str++ = L' ';
                    }
                    
                    continue;
                    
                case L's':
                    if (qualifier == 'h')
                    {
                        /* print ascii string */
                        s = va_arg(args, char8 *);
                        if (s == NULL)
                        {
                            s = "<NULL>";
                        }
                        
                        len = static_cast<int32>(strlen (s));
                        if ((uint32)len > (uint32)precision)
                        {
                            len = precision;
                        }
                        
                        if (!(flags & LEFT))
                        {
                            while (len < field_width--)
                            {
                                *str++ = L' ';
                            }
                        }
                        
                        for (i = 0; i < len; ++i)
                        {
                            *str++ = (char16)(*s++);
                        }
                        
                        while (len < field_width--)
                        {
                            *str++ = L' ';
                        }
                    }
                    else
                    {
                        /* print unicode string */
                        sw = va_arg(args, char16 *);
                        if (sw == NULL)
                        {
                            sw = L"<NULL>";
                        }
                        
                        len = static_cast<int32>(wcslen (sw));
                        if ((uint32)len > (uint32)precision)
                        {
                            len = precision;
                        }
                        
                        if (!(flags & LEFT))
                        {
                            while (len < field_width--)
                            {
                                *str++ = L' ';
                            }
                        }
                        
                        for (i = 0; i < len; ++i)
                        {
                            *str++ = *sw++;
                        }
                        
                        while (len < field_width--)
                        {
                            *str++ = L' ';
                        }
                    }
                    continue;
                    
                case L'S':
                    if (qualifier == 'l' || qualifier == 'w')
                    {
                        /* print unicode string */
                        sw = va_arg(args, char16 *);
                        if (sw == NULL)
                        {
                            sw = L"<NULL>";
                        }
                        
                        len = static_cast<int32>(wcslen (sw));
                        if ((uint32)len > (uint32)precision)
                        {
                            len = precision;
                        }
                        
                        if (!(flags & LEFT))
                        {
                            while (len < field_width--)
                            {
                                *str++ = L' ';
                            }
                        }
                        
                        for (i = 0; i < len; ++i)
                        {
                            *str++ = *sw++;
                        }
                        
                        while (len < field_width--)
                        {
                            *str++ = L' ';
                        }
                    }
                    else
                    {
                        /* print ascii string */
                        s = va_arg(args, char8 *);
                        if (s == NULL)
                        {
                            s = "<NULL>";
                        }
                        
                        len = static_cast<int32>(strlen (s));
                        if ((uint32)len > (uint32)precision)
                        {
                            len = precision;
                        }
                        
                        if (!(flags & LEFT))
                        {
                            while (len < field_width--)
                            {
                                *str++ = L' ';
                            }
                        }
                        
                        for (i = 0; i < len; ++i)
                        {
                            *str++ = (char16)(*s++);
                        }
                        
                        while (len < field_width--)
                        {
                            *str++ = L' ';
                        }
                    }
                    continue;
                    
                case L'Z':
                    if (qualifier == 'h')
                    {
                        /* print counted ascii string */
                        char8 * pus = va_arg(args, char8 *);
                        if (pus == NULL)
                        {
                            sw = L"<NULL>";
                            while ((*sw) != 0)
                            {
                                *str++ = *sw++;
                            }
                        }
                        else
                        {
                            for (i = 0; pus[i] && i < (int32)strlen(pus); i++)
                            {
                                *str++ = (char16)(pus[i]);
                            }
                        }
                    }
                    else
                    {
                        /* print counted unicode string */
                        char16* pus = va_arg(args, char16*);
                        if (pus == NULL)
                        {
                            sw = L"<NULL>";
                            while ((*sw) != 0)
                            {
                                *str++ = *sw++;
                            }
                        }
                        else
                        {
                            for (i = 0; pus[i] && i < (int32)wcslen(pus); ++i) // / sizeof(WCHAR); i++)
                            {
                                *str++ = pus[i];
                            }
                        }
                    }
                    continue;
                    
                case L'p':
                    if (field_width == -1)
                    {
                        field_width = 2*sizeof(void *);
                        flags |= ZEROPAD;
                    }
                    
                    str = Number(str,
                                         (unsigned long) va_arg(args, void *), 16,
                                         field_width, precision, flags);
                    continue;
                    
                case L'n':
                    if (qualifier == 'l')
                    {
                        long * ip = va_arg(args, long *);
                        *ip = (str - buf);
                    }
                    else
                    {
                        int32 * ip = va_arg(args, int32 *);
                        *ip = static_cast<int32>(str - buf);
                    }
                    continue;
                    
                    /* integer number formats - set up the flags and "break" */
                case L'o':
                    base = 8;
                    break;
                    
                case L'b':
                    base = 2;
                    break;
                    
                case L'X':
                    flags |= LARGE;
                case L'x':
                    base = 16;
                    break;
                    
                case L'd':
                case L'i':
                    flags |= SIGN;
                case L'u':
                    break;
                    
                case L'l':
                    if (*(fmt) == L'l' && *(fmt + 1) == L'd')
                    {
                        flags |= LONG_LONG | SIGN;
                        fmt++;
                    }
                    else if (*(fmt) == L'l' && *(fmt + 1) == L'u')
                    {
                        flags |= LONG_LONG;
                        fmt++;
                    }
                    break;
                    
                case L'f':
                {
                    base = -1;
                    
                    while (*strBase)
                    {
                        if (iswdigit(*strBase))
                        {
                            if (base == -1)
                                base = 0;
                            base = base * 10 + SkipAtoi(&strBase);
                            continue;
                        }

                        switch (*strBase)
                        {
                            case L'%':
                                ++strBase;
                                continue;

                            default:
                                break;
                        }
                    
                        break;
                    }
                    
                    qualifier = 'f';
                    flags |= SIGN;
                } break;
                    
                default:
                    if (*fmt != L'%')
                    {
                        *str++ = L'%';
                    }
                    if (*fmt)
                    {
                        *str++ = *fmt;
                    }
                    else
                    {
                        --fmt;
                    }
                    continue;
            }
            
            
            if(qualifier == 'f')
            {
                float64 floatValue = va_arg(args, float64);
                
                precision = 0;
                if(*floatFormatPointer != 'f')
                {
                    bool wasPoint = false;
                    while(*floatFormatPointer != 'f')
                    {
                        if('.' == *floatFormatPointer)
                        {
                            wasPoint = true;
                        }
                        else
                        {
                            if(wasPoint)
                            {
                                precision = (precision * 10) + (*floatFormatPointer) - '0';
                            }
                        }
                        ++floatFormatPointer;
                    }
                    
                    if(!wasPoint)
                    {
                        precision = 6;
                    }
                }
                else
                {
                    precision = 6;
                }
                
                str = Numberf(str, floatValue, base, field_width, precision, flags);
            }
            else
            {
                if (qualifier == 'I')
                {
                    num = va_arg(args, uint64);   
                }
                else if (qualifier == 'l')
                {
                    if (flags & (LONG_LONG | SIGN))
                        num = va_arg(args, long long);
                    else if (flags & LONG_LONG)
                        num = va_arg(args, unsigned long long);
                    else
                        num = va_arg(args, unsigned long);
                }
                else if (qualifier == 'h')
                {
                    if (flags & SIGN)
                    {
                        num = va_arg(args, int32);
                    }
                    else
                    {
                        num = va_arg(args, uint32);
                    }
                }
                else 
                {
                    if (flags & SIGN)
                    {
                        num = va_arg(args, int32);
                    }
                    else
                    {
                        num = va_arg(args, uint32);
                    }
                }
                str = Number(str, num, base, field_width, precision, flags);
            }
        }
        
        *str = L'\0';
        return static_cast<int32>(str-buf);
    }
    
//! formatting function (use printf syntax (%ls for WideString))
WideString Format(const char16 * text, ...)
{
	WideString str;
    
    va_list ll;
	va_start(ll, text);

    str = FormatVL(text, ll);

	va_end(ll);

	return str;
}

String FormatVL(const char8 * text, va_list &ll)
{
	String str;
	char8 buffer[FORMAT_STRING_MAX_LEN];

	vsprintf(buffer, text, ll);

	str = buffer;
	return str;
}

WideString FormatVL(const char16 * text, va_list &ll)
{
	WideString str;
	char16 buffer[FORMAT_STRING_MAX_LEN];

    Vsnwprintf(buffer, FORMAT_STRING_MAX_LEN, text, ll);

	str = buffer;
	return str;
}
}; // end of namespace Log
#endif
