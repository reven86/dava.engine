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


#ifndef SINGLEDOCPARSER_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define SINGLEDOCPARSER_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) || (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || (__GNUC__ >= 4)) // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif


#include "yaml-cpp/anchor.h"
#include "yaml-cpp/noncopyable.h"
#include <string>
#include <map>
#include <memory>

namespace YAML
{
	struct Directives;
	struct Mark;
	struct Token;
	class CollectionStack;
	class EventHandler;
	class Node;
	class Scanner;
	
	class SingleDocParser: private noncopyable
	{
	public:
		SingleDocParser(Scanner& scanner, const Directives& directives);
		~SingleDocParser();

		void HandleDocument(EventHandler& eventHandler);

	private:
		void HandleNode(EventHandler& eventHandler);
		
		void HandleSequence(EventHandler& eventHandler);
		void HandleBlockSequence(EventHandler& eventHandler);
		void HandleFlowSequence(EventHandler& eventHandler);
		
		void HandleMap(EventHandler& eventHandler);
		void HandleBlockMap(EventHandler& eventHandler);
		void HandleFlowMap(EventHandler& eventHandler);
		void HandleCompactMap(EventHandler& eventHandler);
		void HandleCompactMapWithNoKey(EventHandler& eventHandler);
		
		void ParseProperties(std::string& tag, anchor_t& anchor);
		void ParseTag(std::string& tag);
		void ParseAnchor(anchor_t& anchor);
		
		anchor_t RegisterAnchor(const std::string& name);
		anchor_t LookupAnchor(const Mark& mark, const std::string& name) const;
		
	private:
		Scanner& m_scanner;
		const Directives& m_directives;
		std::auto_ptr<CollectionStack> m_pCollectionStack;
		
		typedef std::map<std::string, anchor_t> Anchors;
		Anchors m_anchors;
		
		anchor_t m_curAnchor;
	};
}

#endif // SINGLEDOCPARSER_H_62B23520_7C8E_11DE_8A39_0800200C9A66
