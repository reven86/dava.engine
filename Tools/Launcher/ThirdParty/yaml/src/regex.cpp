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


#include "regex.h"

namespace YAML
{
	// constructors
	RegEx::RegEx(): m_op(REGEX_EMPTY)
	{
	}
	
	RegEx::RegEx(REGEX_OP op): m_op(op)
	{
	}
	
	RegEx::RegEx(char ch): m_op(REGEX_MATCH), m_a(ch)
	{
	}
	
	RegEx::RegEx(char a, char z): m_op(REGEX_RANGE), m_a(a), m_z(z)
	{
	}
	
	RegEx::RegEx(const std::string& str, REGEX_OP op): m_op(op)
	{
		for(std::size_t i=0;i<str.size();i++)
			m_params.push_back(RegEx(str[i]));
	}
	
	// combination constructors
	RegEx operator ! (const RegEx& ex)
	{
		RegEx ret(REGEX_NOT);
		ret.m_params.push_back(ex);
		return ret;
	}
	
	RegEx operator || (const RegEx& ex1, const RegEx& ex2)
	{
		RegEx ret(REGEX_OR);
		ret.m_params.push_back(ex1);
		ret.m_params.push_back(ex2);
		return ret;
	}
	
	RegEx operator && (const RegEx& ex1, const RegEx& ex2)
	{
		RegEx ret(REGEX_AND);
		ret.m_params.push_back(ex1);
		ret.m_params.push_back(ex2);
		return ret;
	}
	
	RegEx operator + (const RegEx& ex1, const RegEx& ex2)
	{
		RegEx ret(REGEX_SEQ);
		ret.m_params.push_back(ex1);
		ret.m_params.push_back(ex2);
		return ret;
	}	
}

