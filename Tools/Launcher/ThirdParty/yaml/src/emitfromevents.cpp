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


#include "yaml-cpp/emitfromevents.h"
#include "yaml-cpp/emitter.h"
#include "yaml-cpp/null.h"
#include <cassert>
#include <sstream>

namespace {
	std::string ToString(YAML::anchor_t anchor) {
		std::stringstream stream;
		stream << anchor;
		return stream.str();
	}
}

namespace YAML
{
	EmitFromEvents::EmitFromEvents(Emitter& emitter): m_emitter(emitter)
	{
	}
	
	void EmitFromEvents::OnDocumentStart(const Mark&)
	{
	}
	
	void EmitFromEvents::OnDocumentEnd()
	{
	}
	
	void EmitFromEvents::OnNull(const Mark&, anchor_t anchor)
	{
		BeginNode();
		EmitProps("", anchor);
		m_emitter << Null;
	}
	
	void EmitFromEvents::OnAlias(const Mark&, anchor_t anchor)
	{
		BeginNode();
		m_emitter << Alias(ToString(anchor));
	}
	
	void EmitFromEvents::OnScalar(const Mark&, const std::string& tag, anchor_t anchor, const std::string& value)
	{
		BeginNode();
		EmitProps(tag, anchor);
		m_emitter << value;
	}
	
	void EmitFromEvents::OnSequenceStart(const Mark&, const std::string& tag, anchor_t anchor)
	{
		BeginNode();
		EmitProps(tag, anchor);
		m_emitter << BeginSeq;
		m_stateStack.push(State::WaitingForSequenceEntry);
	}
	
	void EmitFromEvents::OnSequenceEnd()
	{
		m_emitter << EndSeq;
		assert(m_stateStack.top() == State::WaitingForSequenceEntry);
		m_stateStack.pop();
	}
	
	void EmitFromEvents::OnMapStart(const Mark&, const std::string& tag, anchor_t anchor)
	{
		BeginNode();
		EmitProps(tag, anchor);
		m_emitter << BeginMap;
		m_stateStack.push(State::WaitingForKey);
	}

	void EmitFromEvents::OnMapEnd()
	{
		m_emitter << EndMap;
		assert(m_stateStack.top() == State::WaitingForKey);
		m_stateStack.pop();
	}

	void EmitFromEvents::BeginNode()
	{
		if(m_stateStack.empty())
			return;
		
		switch(m_stateStack.top()) {
			case State::WaitingForKey:
				m_emitter << Key;
				m_stateStack.top() = State::WaitingForValue;
				break;
			case State::WaitingForValue:
				m_emitter << Value;
				m_stateStack.top() = State::WaitingForKey;
				break;
			default:
				break;
		}
	}
	
	void EmitFromEvents::EmitProps(const std::string& tag, anchor_t anchor)
	{
		if(!tag.empty() && tag != "?")
			m_emitter << VerbatimTag(tag);
		if(anchor)
			m_emitter << Anchor(ToString(anchor));
	}
}
