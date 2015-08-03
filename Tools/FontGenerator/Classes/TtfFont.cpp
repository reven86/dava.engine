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


#include "TtfFont.h"

using namespace std;
using namespace DAVA;

TtfFont::TtfFont()
    : faceInit(false)
    , libraryInit(false)
    , size(32)
{
    auto err = FT_Init_FreeType(&library);
    if (err)
    {
        printf("Library init error");
    }
    else
    {
        libraryInit = true;
    }
}

TtfFont::~TtfFont()
{
    DeInit();
    if (libraryInit)
    {
        FT_Done_FreeType(library);
        libraryInit = false;
    }
}

bool TtfFont::Init(const String& file)
{
    DeInit();

    if (!libraryInit)
    {
        return false;
    }

    auto res = FT_New_Face(library, file.c_str(), 0, &face);
    if (res)
    {
        faceInit = false;
        printf("%s: init error\n", file.c_str());
        return false;
    }

    res = FT_Set_Pixel_Sizes(face, size, 0);
    if (res)
    {
        faceInit = false;
        printf("%s: init error\n", file.c_str());
        return false;
    }

    faceInit = true;
    fileName = file;

    return true;
}

void TtfFont::DeInit()
{
    if (faceInit)
    {
        FT_Done_Face(face);
        faceInit = false;
    }
}

void TtfFont::SetSize(int32 _size)
{
    FT_Set_Pixel_Sizes(face, 0, _size);
    size = _size;
}

int32 TtfFont::GetSize() const
{
    return size;
}

FT_Face& TtfFont::GetFace()
{
    return face;
}

float32 TtfFont::GetLineHeight()
{
    return FT_MulFix(face->bbox.yMax - face->bbox.yMin, face->size->metrics.y_scale) / 64.f;
}

float32 TtfFont::GetBaseline() const
{
    return FT_MulFix(face->bbox.yMax, face->size->metrics.y_scale) / 64.f;
}

bool TtfFont::SetCharMap(int32 charmap)
{
    if (charmap >= 0 && charmap < face->num_charmaps)
    {
        auto res = FT_Set_Charmap(face, face->charmaps[charmap]);
        if (res == 0)
        {
            return true;
        }
    }

    return false;
}

int32 TtfFont::GetCharMap() const
{
    for (auto i = 0; i < face->num_charmaps; ++i)
    {
        if (face->charmap == face->charmaps[i])
        {
            return i;
        }
    }

    return -1;
}
