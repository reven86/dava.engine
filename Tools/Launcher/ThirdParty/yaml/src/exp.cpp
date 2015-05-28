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


#include "exp.h"
#include "yaml-cpp/exceptions.h"
#include <sstream>

namespace YAML
{
	namespace Exp
	{
		unsigned ParseHex(const std::string& str, const Mark& mark)
		{
			unsigned value = 0;
			for(std::size_t i=0;i<str.size();i++) {
				char ch = str[i];
				int digit = 0;
				if('a' <= ch && ch <= 'f')
					digit = ch - 'a' + 10;
				else if('A' <= ch && ch <= 'F')
					digit = ch - 'A' + 10;
				else if('0' <= ch && ch <= '9')
					digit = ch - '0';
				else
					throw ParserException(mark, ErrorMsg::INVALID_HEX);

				value = (value << 4) + digit;
			}

			return value;
		}

		std::string Str(unsigned ch)
		{
			return std::string(1, static_cast<char>(ch));
		}

		// Escape
		// . Translates the next 'codeLength' characters into a hex number and returns the result.
		// . Throws if it's not actually hex.
		std::string Escape(Stream& in, int codeLength)
		{
			// grab string
			std::string str;
			for(int i=0;i<codeLength;i++)
				str += in.get();

			// get the value
			unsigned value = ParseHex(str, in.mark());

			// legal unicode?
			if((value >= 0xD800 && value <= 0xDFFF) || value > 0x10FFFF) {
				std::stringstream msg;
				msg << ErrorMsg::INVALID_UNICODE << value;
				throw ParserException(in.mark(), msg.str());
			}

			// now break it up into chars
			if(value <= 0x7F)
				return Str(value);
			else if(value <= 0x7FF)
				return Str(0xC0 + (value >> 6)) + Str(0x80 + (value & 0x3F));
			else if(value <= 0xFFFF)
				return Str(0xE0 + (value >> 12)) + Str(0x80 + ((value >> 6) & 0x3F)) + Str(0x80 + (value & 0x3F));
			else
				return Str(0xF0 + (value >> 18)) + Str(0x80 + ((value >> 12) & 0x3F)) +
					Str(0x80 + ((value >> 6) & 0x3F)) + Str(0x80 + (value & 0x3F));
		}

		// Escape
		// . Escapes the sequence starting 'in' (it must begin with a '\' or single quote)
		//   and returns the result.
		// . Throws if it's an unknown escape character.
		std::string Escape(Stream& in)
		{
			// eat slash
			char escape = in.get();

			// switch on escape character
			char ch = in.get();

			// first do single quote, since it's easier
			if(escape == '\'' && ch == '\'')
				return "\'";

			// now do the slash (we're not gonna check if it's a slash - you better pass one!)
			switch(ch) {
				case '0': return std::string(1, '\x00');
				case 'a': return "\x07";
				case 'b': return "\x08";
				case 't':
                case '\t': return "\x09";
				case 'n': return "\x0A";
				case 'v': return "\x0B";
				case 'f': return "\x0C";
				case 'r': return "\x0D";
				case 'e': return "\x1B";
				case ' ': return "\x20";
				case '\"': return "\"";
				case '\'': return "\'";
				case '\\': return "\\";
				case '/': return "/";
				case 'N': return "\x85";
				case '_': return "\xA0";
				case 'L': return "\xE2\x80\xA8";  // LS (#x2028)
				case 'P': return "\xE2\x80\xA9";  // PS (#x2029)
				case 'x': return Escape(in, 2);
				case 'u': return Escape(in, 4);
				case 'U': return Escape(in, 8);
			}

			std::stringstream msg;
			throw ParserException(in.mark(), std::string(ErrorMsg::INVALID_ESCAPE) + ch);
		}
	}
}
