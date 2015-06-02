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


#include <string>
#include <map>
#include <vector>

#include "TtfFont.h"

class FontConvertor
{
public:
    enum eModes
    {
        MODE_INVALID = -1,
        MODE_GENERATE = 0,
        MODE_ADJUST_FONT,
        MODE_ADJUST_TEXTURE,
        MODES_COUNT
    };

    enum eOutputType
    {
        TYPE_INVALID = -1,
        TYPE_DF = 0,
        TYPE_FNT,
        TYPES_COUNT
    };

    struct Params
    {
        std::string filename;
        int maxChar;
        std::string charListFile;
        int spread;
        int scale;
        eModes mode;
        int fontSize;
        int textureSize;
        int charmap;
        eOutputType output;

        Params();
        static Params GetDefault();
    };

    static const int MAX_TEXTURE_SIZE = 4096;
    static const int MIN_TEXTURE_SIZE = 64;

    FontConvertor();
    ~FontConvertor();

    void InitWithParams(Params params);

    static eModes ModeFromString(const std::string& str);
    static eOutputType TypeFromString(const std::string& str);

    bool Convert();

private:
    struct CharDescription
    {
	    int id;
        int x;
        int y;
	    int width;
        int height;
	    float xOffset;
        float yOffset;
	    float xAdvance;
        std::map<int, float> kernings;
    };

private:
    Params params;
    TtfFont font;
    std::map<int, CharDescription> chars;
    std::vector<std::pair<int, int> > charGlyphPairs;
    int kerningCount;

    void SetDefaultParams();
    bool FillCharList();
    void FillKerning();
    void LoadCharList(std::vector<int>& charList);

    bool GeneratePackedList(int fontSize, int textureSize);
    void GenerateOutputImage();
    void GenerateFontDescription();
    void StoreDf();
    void StoreFnt();

    unsigned char* BuildDistanceField(const unsigned char* inputBuf, int inWidth, int inHeight);
    float FindSignedDistance(int centerX, int centerY,
                             const unsigned char* inputBuf, int width, int height);
    unsigned char DistanceToAlpha(float distance);

    bool AdjustFontSize(int& size);
    bool AdjustTextureSize(int& size);
};