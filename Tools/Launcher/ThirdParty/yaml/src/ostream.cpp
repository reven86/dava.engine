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


#include "yaml-cpp/ostream.h"
#include <cstring>

namespace YAML
{
	ostream::ostream(): m_buffer(0), m_pos(0), m_size(0), m_row(0), m_col(0)
	{
		reserve(1024);
	}
	
	ostream::~ostream()
	{
		delete [] m_buffer;
	}
	
	void ostream::reserve(unsigned size)
	{
		if(size <= m_size)
			return;
		
		char *newBuffer = new char[size];
		std::memset(newBuffer, 0, size * sizeof(char));
		std::memcpy(newBuffer, m_buffer, m_size * sizeof(char));
		delete [] m_buffer;
		m_buffer = newBuffer;
		m_size = size;
	}
	
	void ostream::put(char ch)
	{
		if(m_pos >= m_size - 1)   // an extra space for the NULL terminator
			reserve(m_size * 2);
		
		m_buffer[m_pos] = ch;
		m_pos++;
		
		if(ch == '\n') {
			m_row++;
			m_col = 0;
		} else
			m_col++;
	}

	ostream& operator << (ostream& out, const char *str)
	{
		std::size_t length = std::strlen(str);
		for(std::size_t i=0;i<length;i++)
			out.put(str[i]);
		return out;
	}
	
	ostream& operator << (ostream& out, const std::string& str)
	{
		out << str.c_str();
		return out;
	}
	
	ostream& operator << (ostream& out, char ch)
	{
		out.put(ch);
		return out;
	}
}
