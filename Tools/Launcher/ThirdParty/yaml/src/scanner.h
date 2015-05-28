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


#ifndef SCANNER_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define SCANNER_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) || (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || (__GNUC__ >= 4)) // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif


#include <ios>
#include <string>
#include <queue>
#include <stack>
#include <set>
#include <map>
#include "ptr_vector.h"
#include "stream.h"
#include "token.h"

namespace YAML
{
	class Node;
	class RegEx;

	class Scanner
	{
	public:
		Scanner(std::istream& in);
		~Scanner();

		// token queue management (hopefully this looks kinda stl-ish)
		bool empty();
		void pop();
		Token& peek();

	private:
		struct IndentMarker {
			enum INDENT_TYPE { MAP, SEQ, NONE };
			enum STATUS { VALID, INVALID, UNKNOWN };
			IndentMarker(int column_, INDENT_TYPE type_): column(column_), type(type_), status(VALID), pStartToken(0) {}
		
			int column;
			INDENT_TYPE type;
			STATUS status;
			Token *pStartToken;
		};
		
		enum FLOW_MARKER { FLOW_MAP, FLOW_SEQ };
	
	private:	
		// scanning
		void EnsureTokensInQueue();
		void ScanNextToken();
		void ScanToNextToken();
		void StartStream();
		void EndStream();
		Token *PushToken(Token::TYPE type);
		
		bool InFlowContext() const { return !m_flows.empty(); }
		bool InBlockContext() const { return m_flows.empty(); }
		int GetFlowLevel() const { return m_flows.size(); }
		
		Token::TYPE GetStartTokenFor(IndentMarker::INDENT_TYPE type) const;
		IndentMarker *PushIndentTo(int column, IndentMarker::INDENT_TYPE type);
		void PopIndentToHere();
		void PopAllIndents();
		void PopIndent();
		int GetTopIndent() const;

		// checking input
		bool CanInsertPotentialSimpleKey() const;
		bool ExistsActiveSimpleKey() const;
		void InsertPotentialSimpleKey();
		void InvalidateSimpleKey();
		bool VerifySimpleKey();
		void PopAllSimpleKeys();
		
		void ThrowParserException(const std::string& msg) const;

		bool IsWhitespaceToBeEaten(char ch);
		const RegEx& GetValueRegex() const;

		struct SimpleKey {
			SimpleKey(const Mark& mark_, int flowLevel_);

			void Validate();
			void Invalidate();
			
			Mark mark;
			int flowLevel;
			IndentMarker *pIndent;
			Token *pMapStart, *pKey;
		};

		// and the tokens
		void ScanDirective();
		void ScanDocStart();
		void ScanDocEnd();
		void ScanBlockSeqStart();
		void ScanBlockMapSTart();
		void ScanBlockEnd();
		void ScanBlockEntry();
		void ScanFlowStart();
		void ScanFlowEnd();
		void ScanFlowEntry();
		void ScanKey();
		void ScanValue();
		void ScanAnchorOrAlias();
		void ScanTag();
		void ScanPlainScalar();
		void ScanQuotedScalar();
		void ScanBlockScalar();

	private:
		// the stream
		Stream INPUT;

		// the output (tokens)
		std::queue<Token> m_tokens;

		// state info
		bool m_startedStream, m_endedStream;
		bool m_simpleKeyAllowed;
		bool m_canBeJSONFlow;
		std::stack<SimpleKey> m_simpleKeys;
		std::stack<IndentMarker *> m_indents;
		ptr_vector<IndentMarker> m_indentRefs; // for "garbage collection"
		std::stack<FLOW_MARKER> m_flows;
	};
}

#endif // SCANNER_H_62B23520_7C8E_11DE_8A39_0800200C9A66

