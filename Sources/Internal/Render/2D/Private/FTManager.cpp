#include "FTManager.h"
#include "Logger/Logger.h"
#include "FileSystem/File.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/LocalizationSystem.h"
#include "UI/UIControlSystem.h"

namespace DAVA
{
static const uint32 MAX_FACES = 4;
static const uint32 MAX_SIZES = 16;
static const uint32 MAX_BYTES = 2 * 1024 * 1024;

namespace FTFontDetails
{
FT_Error FaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face* aface)
{
    FaceID* face = static_cast<FaceID*>(face_id);
    FT_Error error = face->OpenFace(library, aface);
    return error;
}
}

FTManager::FTManager()
{
    FT_Error error = FT_Init_FreeType(&library);
    if (error)
    {
        DVASSERT_MSG(false, "FTManager: FT_Init_FreeType failed")
        Logger::Error("FTManager: FT_Init_FreeType failed");
        return;
    }

    error = FTC_Manager_New(library, MAX_FACES, MAX_SIZES, MAX_BYTES, &FTFontDetails::FaceRequester, 0, &manager);
    if (error)
    {
        DVASSERT_MSG(false, "FTManager: FTC_Manager_New failed")
        Logger::Error("FTManager: FTC_Manager_New failed");
        return;
    }

    error = FTC_ImageCache_New(manager, &glyphcache);
    if (error)
    {
        DVASSERT_MSG(false, "FTManager: FTC_ImageCache_New failed")
        Logger::Error("FTManager: FTC_ImageCache_New failed");
    }

    error = FTC_CMapCache_New(manager, &cmapcache);
    if (error)
    {
        DVASSERT_MSG(false, "FTManager: FTC_CMapCache_New failed")
        Logger::Error("FTManager: FTC_CMapCache_New failed");
    }
}

FTManager::~FTManager()
{
    FTC_Manager_Done(manager);
    FT_Done_FreeType(library);
}

FT_Error FTManager::LookupFace(FaceID* faceId, FT_Face* face)
{
    FT_Error error = FTC_Manager_LookupFace(manager, static_cast<FTC_FaceID>(faceId), face);
    DVASSERT(error == FT_Err_Ok);
    return error;
}

FT_Error FTManager::LookupSize(FaceID* faceId, float32 size, FT_Size* ftsize)
{
    FTC_ScalerRec fontScaler =
    {
      static_cast<FTC_FaceID>(faceId),
      static_cast<FT_UInt>(size),
      static_cast<FT_UInt>(size),
      1,
      0,
      0
    };
    FT_Error error = FTC_Manager_LookupSize(manager, &fontScaler, ftsize);
    DVASSERT(error == FT_Err_Ok);
    return error;
}

FT_Error FTManager::LookupGlyph(FaceID* faceId, float32 size, uint32 glyphIndex, FT_Glyph* glyph)
{
    FTC_ScalerRec fontScaler =
    {
      static_cast<FTC_FaceID>(faceId),
      static_cast<FT_UInt>(size),
      static_cast<FT_UInt>(size),
      1,
      0,
      0
    };
    FT_Error error = FTC_ImageCache_LookupScaler(glyphcache, &fontScaler, FT_LOAD_DEFAULT | FT_LOAD_NO_HINTING, glyphIndex, glyph, 0);
    DVASSERT(error == FT_Err_Ok);
    return error;
}

uint32 FTManager::LookupGlyphIndex(FaceID* faceId, uint32 codePoint)
{
    return uint32(FTC_CMapCache_Lookup(cmapcache, static_cast<FTC_FaceID>(faceId), 0, static_cast<FT_UInt32>(codePoint)));
}

void FTManager::RemoveFace(FaceID* faceId)
{
    FTC_Manager_RemoveFaceID(manager, static_cast<FTC_FaceID>(faceId));
}

} // DAVA
