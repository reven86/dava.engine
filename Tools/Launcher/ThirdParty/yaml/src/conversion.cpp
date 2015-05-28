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


#include "yaml-cpp/conversion.h"
#include <algorithm>

////////////////////////////////////////////////////////////////
// Specializations for converting a string to specific types

namespace
{
	// we're not gonna mess with the mess that is all the isupper/etc. functions
	bool IsLower(char ch) { return 'a' <= ch && ch <= 'z'; }
	bool IsUpper(char ch) { return 'A' <= ch && ch <= 'Z'; }
	char ToLower(char ch) { return IsUpper(ch) ? ch + 'a' - 'A' : ch; }

	std::string tolower(const std::string& str)
	{
		std::string s(str);
		std::transform(s.begin(), s.end(), s.begin(), ToLower);
		return s;
	}

	template <typename T>
	bool IsEntirely(const std::string& str, T func)
	{
		for(std::size_t i=0;i<str.size();i++)
			if(!func(str[i]))
				return false;

		return true;
	}

	// IsFlexibleCase
	// . Returns true if 'str' is:
	//   . UPPERCASE
	//   . lowercase
	//   . Capitalized
	bool IsFlexibleCase(const std::string& str)
	{
		if(str.empty())
			return true;

		if(IsEntirely(str, IsLower))
			return true;

		bool firstcaps = IsUpper(str[0]);
		std::string rest = str.substr(1);
		return firstcaps && (IsEntirely(rest, IsLower) || IsEntirely(rest, IsUpper));
	}
}

namespace YAML
{
	bool Convert(const std::string& input, bool& b)
	{
		// we can't use iostream bool extraction operators as they don't
		// recognize all possible values in the table below (taken from
		// http://yaml.org/type/bool.html)
		static const struct {
			std::string truename, falsename;
		} names[] = {
			{ "y", "n" },
			{ "yes", "no" },
			{ "true", "false" },
			{ "on", "off" },
		};

		if(!IsFlexibleCase(input))
			return false;

		for(unsigned i=0;i<sizeof(names)/sizeof(names[0]);i++) {
			if(names[i].truename == tolower(input)) {
				b = true;
				return true;
			}

			if(names[i].falsename == tolower(input)) {
				b = false;
				return true;
			}
		}

		return false;
	}
	
	bool Convert(const std::string& input, _Null& /*output*/)
	{
		return input.empty() || input == "~" || input == "null" || input == "Null" || input == "NULL";
	}
}

