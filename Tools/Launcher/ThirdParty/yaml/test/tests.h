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


#ifndef TESTS_H_62B23520_7C8E_11DE_8A39_0800200C9A66
#define TESTS_H_62B23520_7C8E_11DE_8A39_0800200C9A66

#if defined(_MSC_VER) || (defined(__GNUC__) && (__GNUC__ == 3 && __GNUC_MINOR__ >= 4) || (__GNUC__ >= 4)) // GCC supports "pragma once" correctly since 3.4
#pragma once
#endif

#include <string>

namespace Test {
	void RunAll();

	namespace Parser {
		// scalar tests
		void SimpleScalar(std::string& inputScalar, std::string& desiredOutput);
		void MultiLineScalar(std::string& inputScalar, std::string& desiredOutput);
		void LiteralScalar(std::string& inputScalar, std::string& desiredOutput);
		void FoldedScalar(std::string& inputScalar, std::string& desiredOutput);
		void ChompedFoldedScalar(std::string& inputScalar, std::string& desiredOutput);
		void ChompedLiteralScalar(std::string& inputScalar, std::string& desiredOutput);
		void FoldedScalarWithIndent(std::string& inputScalar, std::string& desiredOutput);
		void ColonScalar(std::string& inputScalar, std::string& desiredOutput);
		void QuotedScalar(std::string& inputScalar, std::string& desiredOutput);
		void CommaScalar(std::string& inputScalar, std::string& desiredOutput);
		void DashScalar(std::string& inputScalar, std::string& desiredOutput);
		void URLScalar(std::string& inputScalar, std::string& desiredOutput);

		// misc tests
		bool SimpleSeq();
		bool SimpleMap();
		bool FlowSeq();
		bool FlowMap();
		bool FlowMapWithOmittedKey();
		bool FlowMapWithOmittedValue();
		bool FlowMapWithSoloEntry();
		bool FlowMapEndingWithSoloEntry();
		bool QuotedSimpleKeys();
		bool CompressedMapAndSeq();
		bool NullBlockSeqEntry();
		bool NullBlockMapKey();
		bool NullBlockMapValue();
		bool SimpleAlias();
		bool AliasWithNull();
		bool AnchorInSimpleKey();
		bool AliasAsSimpleKey();
		bool ExplicitDoc();
		bool MultipleDocs();
		bool ExplicitEndDoc();
		bool MultipleDocsWithSomeExplicitIndicators();
	}
}

#endif // TESTS_H_62B23520_7C8E_11DE_8A39_0800200C9A66
