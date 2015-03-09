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