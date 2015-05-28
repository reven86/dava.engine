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


#include "tag.h"
#include "directives.h"
#include "token.h"
#include <cassert>
#include <stdexcept>

namespace YAML
{
	Tag::Tag(const Token& token): type(static_cast<TYPE>(token.data))
	{
		switch(type) {
			case VERBATIM:
				value = token.value;
				break;
			case PRIMARY_HANDLE:
				value = token.value;
				break;
			case SECONDARY_HANDLE:
				value = token.value;
				break;
			case NAMED_HANDLE:
				handle = token.value;
				value = token.params[0];
				break;
			case NON_SPECIFIC:
				break;
			default:
				assert(false);
		}
	}

	const std::string Tag::Translate(const Directives& directives)
	{
		switch(type) {
			case VERBATIM:
				return value;
			case PRIMARY_HANDLE:
				return directives.TranslateTagHandle("!") + value;
			case SECONDARY_HANDLE:
				return directives.TranslateTagHandle("!!") + value;
			case NAMED_HANDLE:
				return directives.TranslateTagHandle("!" + handle + "!") + value;
			case NON_SPECIFIC:
				// TODO:
				return "!";
			default:
				assert(false);
		}
		throw std::runtime_error("yaml-cpp: internal error, bad tag type");
	}
}

