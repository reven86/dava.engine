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


#ifndef REGEX_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define REGEX_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) || (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || (__GNUC__ >= 4)) // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif


#include <vector>
#include <string>

namespace YAML
{
	class Stream;

	enum REGEX_OP { REGEX_EMPTY, REGEX_MATCH, REGEX_RANGE, REGEX_OR, REGEX_AND, REGEX_NOT, REGEX_SEQ };

	// simplified regular expressions
	// . Only straightforward matches (no repeated characters)
	// . Only matches from start of string
	class RegEx
	{
	public:
		RegEx();
		RegEx(char ch);
		RegEx(char a, char z);
		RegEx(const std::string& str, REGEX_OP op = REGEX_SEQ);
		~RegEx() {}

		friend RegEx operator ! (const RegEx& ex);
		friend RegEx operator || (const RegEx& ex1, const RegEx& ex2);
		friend RegEx operator && (const RegEx& ex1, const RegEx& ex2);
		friend RegEx operator + (const RegEx& ex1, const RegEx& ex2);
		
		bool Matches(char ch) const;
		bool Matches(const std::string& str) const;
		bool Matches(const Stream& in) const;
		template <typename Source> bool Matches(const Source& source) const;

		int Match(const std::string& str) const;
		int Match(const Stream& in) const;
		template <typename Source> int Match(const Source& source) const;

	private:
		RegEx(REGEX_OP op);
		
		template <typename Source> bool IsValidSource(const Source& source) const;
		template <typename Source> int MatchUnchecked(const Source& source) const;

		template <typename Source> int MatchOpEmpty(const Source& source) const;
		template <typename Source> int MatchOpMatch(const Source& source) const;
		template <typename Source> int MatchOpRange(const Source& source) const;
		template <typename Source> int MatchOpOr(const Source& source) const;
		template <typename Source> int MatchOpAnd(const Source& source) const;
		template <typename Source> int MatchOpNot(const Source& source) const;
		template <typename Source> int MatchOpSeq(const Source& source) const;

	private:
		REGEX_OP m_op;
		char m_a, m_z;
		std::vector <RegEx> m_params;
	};
}

#include "regeximpl.h"

#endif // REGEX_H_62B23520_7C8E_11DE_8A39_0800200C9A66
