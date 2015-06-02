#include "TtfFont.h"

using namespace std;

FtLibrary::FtLibrary()
:   libraryInit(false)
{
    int err = FT_Init_FreeType(&library);
    if (err)
    {
        printf("Library init error");
    }
    else
    {
        libraryInit = true;
    }
}

FtLibrary::~FtLibrary()
{
    if (libraryInit)
    {
        FT_Done_FreeType(library);
        libraryInit = false;
    }
}

bool FtLibrary::IsInit()
{
    return libraryInit;
}

FT_Library& FtLibrary::GetLibrary()
{
    return library;
}


TtfFont::TtfFont()
:   faceInit(false)
,   size(32)
{
}

TtfFont::~TtfFont()
{
    DeInit();
}

bool TtfFont::Init(const std::string& file)
{
    if (!FtLibrary::Instance()->IsInit())
    {
        return false;
    }

    int res = FT_New_Face(FtLibrary::Instance()->GetLibrary(), file.c_str(), 0, &face);
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

void TtfFont::SetSize(int size)
{
    FT_Set_Pixel_Sizes(face, size, 0);
    this->size = size;
}

int TtfFont::GetSize() const
{
    return size;
}

FT_Face& TtfFont::GetFace()
{
    return face;
}

float TtfFont::GetLineHeight()
{
    return FT_MulFix(face->bbox.yMax - face->bbox.yMin, face->size->metrics.y_scale) / 64.f;
}

float TtfFont::GetBaseline() const
{
	return FT_MulFix(face->bbox.yMax, face->size->metrics.y_scale) / 64.f;
}

bool TtfFont::SetCharMap(int charmap)
{
    if (charmap >= 0 && charmap < face->num_charmaps)
    {
        int res = FT_Set_Charmap(face, face->charmaps[charmap]);
        if (res == 0)
        {
            return true;
        }
    }

    return false;
}

int TtfFont::GetCharMap() const
{
    for (int i = 0; i < face->num_charmaps; ++i)
    {
        if (face->charmap == face->charmaps[i])
        {
            return i;
        }
    }

    return -1;
}
