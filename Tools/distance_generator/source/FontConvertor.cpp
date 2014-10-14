#include "FontConvertor.h"
#include "BinPacker/BinPacker.hpp"
#include "LodePng/lodepng.h"
#include "UTF8Utils.h"

#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <sstream>

using namespace std;

#define NOT_DEF_CHAR 0xffff

FontConvertor::Params::Params()
:   filename("")
,   maxChar(-1)
,   charListFile("")
,   spread(-1)
,   scale(-1)
,   mode(FontConvertor::MODE_INVALID)
,   fontSize(-1)
,   textureSize(-1)
,   charmap(-1)
,   output(TYPE_DF)
{
}

FontConvertor::Params FontConvertor::Params::GetDefault()
{
    Params params;
    params.fontSize = 32;
    params.maxChar = 128;
    params.mode = MODE_GENERATE;
    params.scale = 16;
    params.spread = 2;
    params.textureSize = 512;
    params.charmap = 0;
    params.output = TYPE_DF;

    return params;
}

FontConvertor::eModes FontConvertor::ModeFromString(const std::string& str)
{
    if (str == "generate")
    {
        return FontConvertor::MODE_GENERATE;
    }
    else if (str == "adjustfont")
    {
        return FontConvertor::MODE_ADJUST_FONT;
    }
    else if (str == "adjusttexture")
    {
        return FontConvertor::MODE_ADJUST_TEXTURE;
    }

    return FontConvertor::MODE_INVALID;
}

FontConvertor::eOutputType FontConvertor::TypeFromString(const std::string& str)
{
    if (str == "df")
    {
        return FontConvertor::TYPE_DF;
    }
    else if (str == "fnt")
    {
        return FontConvertor::TYPE_FNT;
    }

    return FontConvertor::TYPE_INVALID;
}

FontConvertor::FontConvertor()
{
    SetDefaultParams();
}

FontConvertor::~FontConvertor()
{
}

void FontConvertor::InitWithParams(Params params)
{
    Params def = Params::GetDefault();

    if (params.fontSize <= 0)
    {
        params.fontSize = def.fontSize;
    }
    if (params.maxChar <= 0)
    {
        params.maxChar = def.maxChar;
    }
    if (params.mode <= MODE_INVALID || params.mode >= MODES_COUNT)
    {
        params.mode = def.mode;
    }
    if (params.scale <= 0)
    {
        params.scale = def.scale;
    }
    if (params.spread < 0)
    {
        params.spread = def.spread;
    }
    if (params.textureSize < MIN_TEXTURE_SIZE || params.textureSize > MAX_TEXTURE_SIZE)
    {
        params.textureSize = def.textureSize;
    }
    if (params.charmap < 0)
    {
        params.charmap = def.charmap;
    }
    if (params.output <= TYPE_INVALID || params.output >= TYPES_COUNT)
    {
        params.output = def.output;
    }

    this->params = params;
}

void FontConvertor::SetDefaultParams()
{
    this->params = Params::GetDefault();
}

bool FontConvertor::Convert()
{
    if (!font.Init(params.filename))
    {
        return false;
    }

    font.SetCharMap(params.charmap);
    if (!FillCharList())
    {
        return false;
    }

    bool ok = true;
    switch (params.mode)
    {
        case MODE_ADJUST_FONT:
            ok = AdjustFontSize(params.fontSize);
            break;

        case MODE_ADJUST_TEXTURE:
            ok = AdjustTextureSize(params.textureSize);
            break;

        case MODE_GENERATE:
            break;

        default:
            break;
    }

    if (!ok)
    {
        cerr << endl << "Error: chars will not fit into texture" << endl;
        return false;
    }

    cout << "Packing chars into texture...";
    if (!GeneratePackedList(params.fontSize, params.textureSize))
    {
        cerr << endl << "Error: chars will not fit into texture" << endl;
        return false;
    }
    cout << " Done" << endl << endl;

    font.SetSize(params.fontSize * params.scale);

    FillKerning();
    GenerateOutputImage();
    GenerateFontDescription();

    return true;
}

void FontConvertor::StoreDf()
{
    ofstream outFile;
    outFile.open(params.filename + ".df");

    outFile << "font:" << endl;
    outFile << "  name: " << "" << endl;
    outFile << "  size: " << params.fontSize << endl;
    outFile << "  spread: " << params.spread << endl;

    outFile << "  lineHeight: " << (int)ceil(font.GetLineHeight()) << endl;
    outFile << "  baselineHeight: " << (int)ceil(font.GetBaseline()) << endl;
    outFile << "  scaleW: " << params.textureSize << endl;
    outFile << "  scaleH: " << params.textureSize << endl;

    outFile << "  chars: " << endl;
    map<int, CharDescription>::iterator it;
    for (it = chars.begin(); it != chars.end(); ++it)
    {
        CharDescription& desc = it->second;

        float u, u2, v, v2;
        u = (float)desc.x / params.textureSize;
        v = (float)desc.y / params.textureSize;
        u2 = (float)(desc.x + desc.width) / params.textureSize;
        v2 = (float)(desc.y + desc.height) / params.textureSize;

        outFile << "    " << desc.id << ": {xoffset: " << desc.xOffset << ", yoffset: " << desc.yOffset;
        outFile << ", width: " << desc.width << ", height: " << desc.height << ", xadvance: " << desc.xAdvance;
        outFile << ", u: " << u << ", v: " << v << ", u2: " << u2 << ", v2: " << v2 << "}" << endl;
    }

    if (kerningCount)
    {
        outFile << "  kerning:" << endl;

        for (it = chars.begin(); it != chars.end(); ++it)
        {
            CharDescription& desc = it->second;

            ostringstream os;
            int cnt = 0;
            map<int, float>::iterator kernIt;
            for (kernIt = desc.kernings.begin(); kernIt != desc.kernings.end(); ++kernIt)
            {
                if (cnt++)
                {
                    os << ", ";
                }

                os << kernIt->first << ": " << kernIt->second;
            }

            if (cnt)
            {
                outFile << "    " << desc.id << ": {" << os.str() << "}" << endl;
            }
        }
    }

    outFile.close();
}

void FontConvertor::StoreFnt()
{
    ofstream outFile;
    outFile.open(params.filename + ".fnt");

    outFile << "info face=" << "" << " size=" << params.fontSize << " spread=" << params.spread << endl;
    outFile << "common lineHeight=" << (int)ceil(font.GetLineHeight());
    outFile << " baselineHeight=" << (int)ceil(font.GetBaseline());
    outFile << " scaleW=" << params.textureSize << " scaleH=" << params.textureSize << endl;
    outFile << "chars count=" << chars.size() << endl;

    map<int, CharDescription>::iterator it;
    for (it = chars.begin(); it != chars.end(); ++it)
    {
        CharDescription& desc = it->second;

        outFile << "char id=" << desc.id << " x=" << desc.x << " y=" << desc.y << " width=" << desc.width;
        outFile << " height=" << desc.height << " xoffset=" << desc.xOffset << " yoffset=" << desc.yOffset;
        outFile << " xadvance=" << desc.xAdvance << endl;
    }

    outFile << "kernings count=" << kerningCount << endl;
    for (it = chars.begin(); it != chars.end(); ++it)
    {
        CharDescription& desc = it->second;
        map<int, float>::iterator kernIt;
        for (kernIt = desc.kernings.begin(); kernIt != desc.kernings.end(); ++kernIt)
        {
            outFile << "kerning first=" << desc.id << " second=" << kernIt->first << " amount=" << kernIt->second << endl;
        }
    }

    outFile.close();
}

void FontConvertor::GenerateFontDescription()
{
    cout  << "Storing font description...";

    int oldSize = font.GetSize();
    font.SetSize(params.fontSize);

    if (params.output == TYPE_DF)
    {
        StoreDf();
    }
    else if (params.output == TYPE_FNT)
    {
        StoreFnt();
    }

    font.SetSize(oldSize);

    cout << " Done" << endl << endl;
}

void FontConvertor::FillKerning()
{
    cout << "Preparing kerning information..." << endl;

    FT_Face& face = font.GetFace();
    kerningCount = 0;

    std::map<int, CharDescription>::iterator i, j;

    int percDone = 0;
    int cnt = 0;
    int size = (int)chars.size();
    for (i = chars.begin(); i != chars.end(); ++i)
    {
        int charIndexLeft = FT_Get_Char_Index(face, i->second.id);

        for (j = chars.begin(); j != chars.end(); ++j)
        {
            int charIndexRight = FT_Get_Char_Index(face, j->second.id);

            FT_Vector kerning;
            if (!FT_Get_Kerning(face, charIndexLeft, charIndexRight, FT_KERNING_DEFAULT, &kerning))
            {
                if (kerning.x)
                {
                    float scaledKern = kerning.x / 64.f / params.scale;
                    i->second.kernings[j->second.id] = scaledKern;
                    ++kerningCount;
                }
            }
        }
        ++cnt;

        int p = (int)floor(((float)cnt / size) * 100);
        if (p > percDone)
        {
            percDone = p;
            cout << setw(3) << p << "% done" << endl;
        }
    }

    cout << kerningCount << " kerning pairs found" << endl << endl;
}

bool FontConvertor::FillCharList()
{
    cout << "Preparing char list...";

    vector<int> unfilteredChars;

    if (params.charListFile != "")
    {
        LoadCharList(unfilteredChars);
    }
    else
    {
        for (int i = 0; i <= params.maxChar; ++i)
        {
            unfilteredChars.push_back(i);
        }
    }

    int oldSize = font.GetSize();
    font.SetSize(Params::GetDefault().fontSize);

    FT_Face& face = font.GetFace();

    int err;
    for(int i = 0; i < unfilteredChars.size(); ++i)
	{
        int glyphIndex = FT_Get_Char_Index(face, unfilteredChars[i]);
        if (glyphIndex)
        {
            err = FT_Load_Glyph(face, glyphIndex, 0);
			if (!err)
			{
				err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
				if (!err)
				{
                    charGlyphPairs.push_back(make_pair(unfilteredChars[i], glyphIndex));
                }
            }
        }
    }

    int glyphIndex = FT_Get_Char_Index(face, (int)' ');
    if (!glyphIndex)
    {
        cerr << "Font doesn't contain space character. This font could not be converted." << endl;
        return false;
    }
    charGlyphPairs.push_back(make_pair(NOT_DEF_CHAR, glyphIndex));

    font.SetSize(oldSize);

    cout << " " << charGlyphPairs.size() << " characters found" << endl << endl;
    return true;
}

void FontConvertor::LoadCharList(vector<int>& charList)
{
    vector<string> strings;

    ifstream inFile(params.charListFile);
    if (inFile.is_open())
    {
        string line;
        while (getline(inFile, line))
        {
            strings.push_back(line);
        }

        inFile.close();
    }

    for (int i = 0; i < strings.size(); ++i)
    {
        wstring ws;
        UTF8Utils::EncodeToWideString((const char*)strings[i].c_str(), 1024, ws);

        int j = 0;
        while (ws[j] != 0)
        {
            charList.push_back(ws[j++]);
        }
    }

    sort(charList.begin(), charList.end());
    charList.erase(unique(charList.begin(), charList.end()), charList.end());
}

bool FontConvertor::AdjustFontSize(int& size)
{
    cout << "Adjusting font size..." << endl;
    int curSize = Params::GetDefault().fontSize >> 1;

    bool keepGoing = true;
    while (keepGoing)
    {
        curSize <<= 1;

        cout << curSize << " ";
        keepGoing = GeneratePackedList(curSize, params.textureSize);
    }

    int step = curSize >> 2;
    while (step)
    {
        curSize += (keepGoing ? 1 : -1) * step;
        step >>= 1;

        cout << curSize << " ";
        keepGoing = GeneratePackedList(curSize, params.textureSize);
    }

    while (!keepGoing && curSize > 1)
    {
        --curSize;

        cout << curSize << " ";
        keepGoing = GeneratePackedList(curSize, params.textureSize);
    }

    cout << endl << "Done" << endl;
    cout << endl << "Font size: " << curSize << endl;

    if (!keepGoing)
    {
        return false;
    }

    size = curSize;

    return true;
}

bool FontConvertor::AdjustTextureSize(int& size)
{
    cout << "Adjusting texture size..." << endl;

    int curSize = Params::GetDefault().textureSize >> 1;

    bool keepGoing = false;
    while (!keepGoing && curSize <= MAX_TEXTURE_SIZE)
    {
        curSize <<= 1;

        cout << curSize << " ";
        keepGoing = GeneratePackedList(params.fontSize, curSize);
    }

    if (!keepGoing)
    {
        return false;
    }

    while (keepGoing && curSize >= MIN_TEXTURE_SIZE)
    {
        curSize >>= 1;

        cout << curSize << " ";
        keepGoing = GeneratePackedList(params.fontSize, curSize);
    }

    curSize <<= 1;
    if (curSize < MIN_TEXTURE_SIZE)
    {
        curSize = MIN_TEXTURE_SIZE;
    }

    cout << endl << "Done" << endl;
    cout << "Texture size: " << curSize << endl << endl;

    size = curSize;

    return true;
}

void FontConvertor::GenerateOutputImage()
{
    cout << "Converting..." << endl;

    FT_Face& face = font.GetFace();

    std::vector<unsigned char> imgData(4 * params.textureSize * params.textureSize, 0x00);

    int err;
    int percentDone = 0;

    int i = 0;
    vector<pair<int, int> >::const_iterator iter = charGlyphPairs.begin();
    for (;iter != charGlyphPairs.end(); ++iter)
	{
        int glyphIndex = iter->second;

        err = FT_Load_Glyph(face, glyphIndex, 0);
        if (!err)
        {
            err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
            if (!err)
            {
                int glyphWidth = face->glyph->bitmap.width;
                int glyphHeight = face->glyph->bitmap.rows;
                int glyphPitch = face->glyph->bitmap.pitch;

                //	oversize the holding buffer by spread value to be filled in with distance blur
                int charWidth = glyphWidth + params.scale * (params.spread * 2);
                int charHeight = glyphHeight + params.scale * (params.spread * 2);

                unsigned char *charBuf = 0;

                if (iter->first == NOT_DEF_CHAR)
                {
                    int glyphAdvance = (int)face->glyph->advance.x / 64;

                    glyphWidth = glyphAdvance;
                    float leading = face->size->metrics.height + face->size->metrics.descender;
                    leading /= (float)64.f;
                    
                    glyphHeight = (int)(leading * 0.75f);

                    charWidth = glyphWidth + params.scale * (params.spread * 2);
                    charHeight = glyphHeight + params.scale * (params.spread * 2);

                    charBuf = new unsigned char[charWidth * charHeight];
                    memset(charBuf, 0, sizeof(unsigned char) * charWidth * charHeight);

                    int lineWidth = min(charWidth, charHeight);
                    lineWidth = max(1, lineWidth / 8);

                    for (int j = 0; j < glyphHeight; ++j)
                    {
                        for (int i = 0; i < lineWidth; ++i)
                        {
                            int y = (j + params.scale * params.spread) * charWidth;
                            int x = i + params.scale * params.spread;

                            charBuf[y + x] = 255;
                            charBuf[y + charWidth - x] = 255;
                        }
                    }

                    for (int i = 0; i < glyphWidth; ++i)
                    {
                        for (int j = 0; j < lineWidth; ++j)
                        {
                            int y = (j + params.scale * params.spread) * charWidth;
                            int x = i + params.scale * params.spread;

                            int maxY = (charHeight - 1) * charWidth;

                            charBuf[y + x] = 255;
                            charBuf[maxY - y + x] = 255;
                        }
                    }
                }
                else
                {
                    charBuf = new unsigned char[charWidth * charHeight];
                    memset(charBuf, 0, sizeof(unsigned char) * charWidth * charHeight);

                    //	copy the glyph into the buffer to be smoothed
                    unsigned char *glyphBuf = face->glyph->bitmap.buffer;
                    for (int j = 0; j < glyphHeight; ++j)
                    {
                        for (int i = 0; i < glyphWidth; ++i)
                        {
                            //check if corresponding bit is set
                            unsigned char glyphVal = glyphBuf[j * glyphPitch + (i >> 3)];
                            unsigned char val = (glyphVal >> (7 - (i & 7))) & 1;

                            int x = i + params.scale * params.spread;
                            int y = (j + params.scale * params.spread) * charWidth;
                            charBuf[y + x] = 255 * val;
                        }
                    }
                }

                CharDescription& desc = chars[iter->first];
                int distanceCharX = desc.x;
                int distanceCharY = desc.y;
                int distanceCharWidth = desc.width;
                int distanceCharHeight = desc.height;

                unsigned char* distanceBuf = BuildDistanceField(charBuf, charWidth, charHeight);

                for (int i = 0; i < distanceCharHeight; ++i)
                {
                    int offset = ((distanceCharY + i) * params.textureSize + distanceCharX) * 4;

                    for (int j = 0; j < distanceCharWidth; ++j)
                    {
                        imgData[offset + 0] = 0xff;
                        imgData[offset + 1] = 0xff;
                        imgData[offset + 2] = 0xff;
                        imgData[offset + 3] = distanceBuf[i * distanceCharWidth + j];
                        offset += 4;
                    }
                }

                delete[] charBuf;
                delete[] distanceBuf;
            }
        }
        int p = floor(((float)i++ / charGlyphPairs.size()) * 100);
        if (p > percentDone)
        {
            cout << setw(3) << p << "% converted" << endl;
            percentDone = p;
        }
    }
    cout << "Done" << endl << endl;

    cout << "Storing image...";
    LodePNG::Encoder encoder;
    encoder.getSettings().zlibsettings.windowSize = 512;
    std::vector<unsigned char> buffer;

    encoder.encode(buffer,imgData.data(), params.textureSize, params.textureSize);
    LodePNG::saveFile(buffer, params.filename + ".png");
    cout << " Done" << endl << endl;
}

unsigned char* FontConvertor::BuildDistanceField(const unsigned char* inputBuf, int inWidth, int inHeight)
{
    int outWidth = inWidth / params.scale;
    int outHeight = inHeight / params.scale;

    unsigned char* outBuf = new unsigned char[outWidth * outHeight];

    for (int y = 0; y < outHeight; ++y)
    {
        for (int x = 0; x < outWidth; ++x)
        {
            int centerX = x * params.scale + params.scale / 2;
            int centerY = y * params.scale + params.scale / 2;

            float dist = FindSignedDistance(centerX, centerY, inputBuf, inWidth, inHeight);
            outBuf[y * outWidth + x] = DistanceToAlpha(dist);
        }
    }

    return outBuf;
}

unsigned char FontConvertor::DistanceToAlpha(float distance)
{
    float alpha = 0.5f + 0.5f * (distance / (params.spread * params.scale));
    alpha = min(1.f, max(0.f, alpha));
    return (unsigned char)(alpha * 0xff);
}

float FontConvertor::FindSignedDistance(int centerX, int centerY,
                                        const unsigned char* inputBuf, int width, int height)
{
    bool baseVal = inputBuf[centerY * width + centerX];
    int scaledSpread = params.spread * params.scale;

    int startX = max(0, centerX - scaledSpread);
    int endX = min(width - 1, centerX + scaledSpread);
    int startY = max(0, centerY - scaledSpread);
    int endY = min(height - 1, centerY + scaledSpread);

    int minSquareDist = scaledSpread * scaledSpread;

    for (int y = startY; y <= endY; ++y)
    {
        for (int x = startX; x <= endX; ++x)
        {
            bool curVal = inputBuf[y * width + x];
            if (baseVal != curVal)
            {
                int dx = centerX - x;
                int dy = centerY - y;
                int squareDist = dx * dx + dy * dy;

                if (squareDist < minSquareDist)
                {
                    minSquareDist = squareDist;
                }
            }
        }
    }

    float minDist = sqrtf(minSquareDist);
    return (baseVal ? 1 : -1) * min(minDist, (float)scaledSpread);
}

bool FontConvertor::GeneratePackedList(int fontSize, int textureSize)
{
    chars.clear();

    int oldSize = font.GetSize();
    font.SetSize(fontSize * params.scale);
    FT_Face& face = font.GetFace();

    vector<int> rectInfo;

    int err;

    // Here we should determine max glyph height, to get proper yOffset later.
    int maxBitmapTop = 0;
    vector<pair<int, int> >::const_iterator iter = charGlyphPairs.begin();
    for (;iter != charGlyphPairs.end(); ++iter)
    {
        int glyphIndex = iter->second;
        err = FT_Load_Glyph(face, glyphIndex, 0);
        if (!err)
        {
            err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
            if (!err)
            {
                if(maxBitmapTop < face->glyph->bitmap_top)
                {
                    maxBitmapTop = face->glyph->bitmap_top;
                }
            }
        }
    }
    maxBitmapTop /= (float)params.scale;
    iter = charGlyphPairs.begin();
    
    for (;iter != charGlyphPairs.end(); ++iter)
	{
        if (iter->first == NOT_DEF_CHAR)
        {
            int glyphIndex = FT_Get_Char_Index(face, ' ');
            err = FT_Load_Glyph(face, glyphIndex, 0);
            if (!err)
            {
                err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
                if (!err)
                {
                    FontConvertor::CharDescription charDesc;
                    int glyphAdvance = (int)face->glyph->advance.x / 64;

                    int glyphWidth = glyphAdvance;

                    float leading = face->size->metrics.height + face->size->metrics.descender;
                    leading /= (float)64.f;

                    int glyphHeight = (int)(leading * 0.75f);

                    int rectWidth = glyphWidth + params.scale * params.spread * 2;
                    int rectHeight = glyphHeight + params.scale * params.spread * 2;

                    int scaledWidth = rectWidth / params.scale;
                    int scaledHeight = rectHeight / params.scale;

                    rectInfo.push_back(scaledWidth);
                    rectInfo.push_back(scaledHeight);

                    charDesc.id = iter->first;
                    charDesc.width = scaledWidth;
                    charDesc.height = scaledHeight;

                    charDesc.x = -1;
                    charDesc.y = -1;

                    charDesc.xOffset = face->glyph->bitmap_left / (float)params.scale;
                    charDesc.yOffset = leading * 0.25f / params.scale;
                    charDesc.xAdvance = face->glyph->advance.x / params.scale / 64.f;

                    chars[charDesc.id] = charDesc;
                }
            }
        }
        else
        {
            int glyphIndex = iter->second;

            err = FT_Load_Glyph(face, glyphIndex, 0);
            if (!err)
            {
                err = FT_Render_Glyph(face->glyph, FT_RENDER_MODE_MONO);
                if (!err)
                {
                    FontConvertor::CharDescription charDesc;
                    int glyphWidth = face->glyph->bitmap.width;
                    int glyphHeight = face->glyph->bitmap.rows;

                    int rectWidth = 0;
                    int rectHeight = 0;

                    if (glyphWidth)
                    {
                        rectWidth = glyphWidth + params.scale * params.spread * 2;
                    }
                    if (glyphHeight)
                    {
                        rectHeight = glyphHeight + params.scale * params.spread * 2;
                    }

                    int scaledWidth = rectWidth / params.scale;
                    int scaledHeight = rectHeight / params.scale;

                    rectInfo.push_back(scaledWidth);
                    rectInfo.push_back(scaledHeight);

                    charDesc.id = iter->first;
                    charDesc.width = scaledWidth;
                    charDesc.height = scaledHeight;

                    charDesc.x = -1;
                    charDesc.y = -1;

                    charDesc.xOffset = face->glyph->bitmap_left / (float)params.scale;
                    charDesc.yOffset = maxBitmapTop - face->glyph->bitmap_top / (float)params.scale;
                    charDesc.xAdvance = face->glyph->advance.x / params.scale / 64.f;

                    chars[charDesc.id] = charDesc;
                }
            }
        }
	}

    font.SetSize(oldSize);

	BinPacker bp;
    vector<vector<int> > packedInfo;
    bp.Pack(rectInfo, packedInfo, textureSize, false);

	if(packedInfo.size() == 1)
	{
		unsigned int cnt = (unsigned int)packedInfo[0].size();
		for (unsigned int i = 0; i < cnt; i += 4)
		{
			//	index, x, y, rotated
			unsigned int index = charGlyphPairs[packedInfo[0][i]].first;
			chars[index].x = packedInfo[0][i + 1];
			chars[index].y = packedInfo[0][i + 2];
		}
        return true;
	}

    return false;
}
