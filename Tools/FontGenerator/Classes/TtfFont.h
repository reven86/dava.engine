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


#ifndef __DISTANCE_GENERATOR__TTFFONT__
#define __DISTANCE_GENERATOR__TTFFONT__

#include <Base/BaseTypes.h>
#include <Base/Singleton.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H

class TtfFont
{
public:
    TtfFont();
    ~TtfFont();

    bool Init(const DAVA::String& file);
    void DeInit();

    void SetSize(DAVA::int32 size);
    DAVA::int32 GetSize() const;

    bool SetCharMap(DAVA::int32 charmap);
    DAVA::int32 GetCharMap() const;

    DAVA::float32 GetLineHeight();
	DAVA::float32 GetBaseline() const;

    FT_Face& GetFace();
protected:
    DAVA::int32 size;
    FT_Face face;
    bool faceInit;
    DAVA::String fileName;
    bool libraryInit;
    FT_Library library;
};

#endif /* defined(__DISTANCE_GENERATOR__TTFFONT__) */
