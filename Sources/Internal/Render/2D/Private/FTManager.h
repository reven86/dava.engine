#pragma once

#include "Base/BaseTypes.h"
#include "FTInclude.h"

namespace DAVA
{
class FTManager final
{
public:
    struct FaceID
    {
        virtual FT_Error OpenFace(FT_Library library, FT_Face* ftface) = 0;
    };

    FTManager();
    ~FTManager();

    FT_Error LookupFace(FaceID* faceId, FT_Face* face);
    FT_Error LookupSize(FaceID* faceId, float32 size, FT_Size* ftsize);
    FT_Error LookupGlyph(FaceID* faceId, float32 size, uint32 glyphIndex, FT_Glyph* glyph);
    uint32 LookupGlyphIndex(FaceID* faceId, uint32 codePoint);
    void RemoveFace(FaceID* faceId);

private:
    static FT_Error FaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face* aface);

    FT_Library library = nullptr;
    FTC_Manager manager = nullptr;
    FTC_ImageCache glyphcache = nullptr;
    FTC_CMapCache cmapcache = nullptr;
};

} // DAVA
