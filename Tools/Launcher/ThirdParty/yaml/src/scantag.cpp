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


#include "scanner.h"
#include "regex.h"
#include "exp.h"
#include "yaml-cpp/exceptions.h"

namespace YAML
{
	const std::string ScanVerbatimTag(Stream& INPUT)
	{
		std::string tag;
		
		// eat the start character
		INPUT.get();
		
		while(INPUT) {
			if(INPUT.peek() == Keys::VerbatimTagEnd) {
				// eat the end character
				INPUT.get();
				return tag;
			}
		
			int n = Exp::URI().Match(INPUT);
			if(n <= 0)
				break;
			
			tag += INPUT.get(n);
		}

		throw ParserException(INPUT.mark(), ErrorMsg::END_OF_VERBATIM_TAG);
	}
	
	const std::string ScanTagHandle(Stream& INPUT, bool& canBeHandle)
	{
		std::string tag;
		canBeHandle = true;
		Mark firstNonWordChar;
		
		while(INPUT) {
			if(INPUT.peek() == Keys::Tag) {
				if(!canBeHandle)
					throw ParserException(firstNonWordChar, ErrorMsg::CHAR_IN_TAG_HANDLE);
				break;
			}

			int n = 0;
			if(canBeHandle) {
				n = Exp::Word().Match(INPUT);
				if(n <= 0) {
					canBeHandle = false;
					firstNonWordChar = INPUT.mark();
				}
			}
			
			if(!canBeHandle)
				n = Exp::Tag().Match(INPUT);

			if(n <= 0)
				break;
			
			tag += INPUT.get(n);
		}

		return tag;
	}
	
	const std::string ScanTagSuffix(Stream& INPUT)
	{
		std::string tag;
		
		while(INPUT) {
			int n = Exp::Tag().Match(INPUT);
			if(n <= 0)
				break;
			
			tag += INPUT.get(n);
		}
		
		if(tag.empty())
			throw ParserException(INPUT.mark(), ErrorMsg::TAG_WITH_NO_SUFFIX);
		
		return tag;
	}
}

