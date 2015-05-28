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


#ifndef STRINGSOURCE_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define STRINGSOURCE_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) || (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || (__GNUC__ >= 4)) // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif


#include <cstddef>

namespace YAML
{
	class StringCharSource
	{
	public:
		StringCharSource(const char *str, std::size_t size): m_str(str), m_size(size), m_offset(0) {}

		operator bool() const { return m_offset < m_size; }
		char operator [] (std::size_t i) const { return m_str[m_offset + i]; }
		bool operator !() const { return !static_cast<bool>(*this); }

		const StringCharSource operator + (int i) const {
			StringCharSource source(*this);
			if(static_cast<int> (source.m_offset) + i >= 0)
				source.m_offset += i;
			else
				source.m_offset = 0;
			return source;
		}
			
		StringCharSource& operator ++ () {
			++m_offset;
			return *this;
		}
		
		StringCharSource& operator += (std::size_t offset) {
			m_offset += offset;
			return *this;
		}
	private:
		const char *m_str;
		std::size_t m_size;
		std::size_t m_offset;
	};
}

#endif // STRINGSOURCE_H_62B23520_7C8E_11DE_8A39_0800200C9A66
