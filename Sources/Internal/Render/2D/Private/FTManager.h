#pragma once

#include "Base/BaseTypes.h"

#ifdef __DAVAENGINE_WIN_UAP__
#define generic GenericFromFreeTypeLibrary
#endif

#ifdef __clang__
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wold-style-cast"
#endif

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_CACHE_H

#ifdef __clang__
#pragma clang diagnostic pop
#endif

#ifdef __DAVAENGINE_WIN_UAP__
#undef generic
#endif

namespace DAVA
{
class FTManager final
{
public:
    struct FaceID
    {
        String fileName;
        int32 faceIndex = 0;
        std::unique_ptr<FT_StreamRec> stream;
    };

    FTManager();
    ~FTManager();

    bool LookupFace(FaceID* faceId, FT_Face* face);
    bool LookupSize(FaceID* faceId, float32 size, FT_Face* aface, FT_Size* asize);
    bool LookupGlyph(FaceID* faceId, float32 size, uint32 glyphIndex, FT_Glyph* glyph);
    uint32 LookupGlyphIndex(FaceID* faceId, uint32 codePoint);
    void RemoveFace(FaceID* faceId);

private:
    static unsigned long FTManager::StreamLoad(FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count);
    static void FTManager::StreamClose(FT_Stream stream);
    static FT_Error FTManager::FaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face* aface);

    FT_Library library = nullptr;
    FTC_Manager manager = nullptr;
    FTC_ImageCache glyphcache = nullptr;
    FTC_CMapCache cmapcache = nullptr;
};

} // DAVA