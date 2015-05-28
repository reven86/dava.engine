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


#ifndef PTR_STACK_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define PTR_STACK_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) || (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || (__GNUC__ >= 4)) // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include "yaml-cpp/noncopyable.h"
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <vector>

template <typename T>
class ptr_stack: private YAML::noncopyable
{
public:
	ptr_stack() {}
	~ptr_stack() { clear(); }
	
	void clear() {
		for(unsigned i=0;i<m_data.size();i++)
			delete m_data[i];
		m_data.clear();
	}
	
	std::size_t size() const { return m_data.size(); }
	bool empty() const { return m_data.empty(); }
	
	void push(std::auto_ptr<T> t) {
		m_data.push_back(NULL);
		m_data.back() = t.release();
	}
	std::auto_ptr<T> pop() {
		std::auto_ptr<T> t(m_data.back());
		m_data.pop_back();
		return t;
	}
	T& top() { return *m_data.back(); }
	const T& top() const { return *m_data.back(); }
	
private:
	std::vector<T*> m_data;
};

#endif // PTR_STACK_H_62B23520_7C8E_11DE_8A39_0800200C9A66
