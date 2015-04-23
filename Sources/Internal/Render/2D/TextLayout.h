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

#include "Base/BaseTypes.h"
#include "Render/2D/Font.h"
#include "Utils/BiDiHelper.h"

namespace DAVA
{

class TextLayout
{
public:
    TextLayout();
    TextLayout(const bool useBiDi);
    virtual ~TextLayout();

    void Reset(const WideString& input, const Font& font);
    void Seek(const uint32 position);

    const WideString& GetText() const;
    const WideString& GetPreparedText() const;
    const WideString GetVisualText(const bool trimEnd) const;
    const bool IsRtl() const;

    bool HasNext();
    void Next(const float32 lineWidth, const bool bySymbols = false);
    void NextByWords(const float32 lineWidth);
    void NextBySymbols(const float32 lineWidth);

    const WideString& GetPreparedLine() const;
    const WideString GetVisualLine(const bool trimEnd) const;
    
private:
    const WideString BuildVisualString(const WideString& _input, const bool trimEnd) const;

    WideString inputText;
    WideString preparedText;
    WideString preparedLine;
    
    bool useBiDi;
    bool isRtl;
    Vector<float32> characterSizes;
    Vector<uint8> breaks;
    BiDiHelper bidiHelper;
    uint32 fromPos;
};

inline const WideString& TextLayout::GetPreparedLine() const
{
    return preparedLine;
}

inline const WideString& TextLayout::GetText() const
{
    return inputText;
}

inline const WideString& TextLayout::GetPreparedText() const
{
    return preparedText;
}

inline const bool TextLayout::IsRtl() const
{
    return useBiDi && isRtl;
}

}