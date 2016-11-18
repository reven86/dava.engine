#include "FTManager.h"
#include "Logger/Logger.h"
#include "FileSystem/File.h"
#include "FileSystem/FilePath.h"
#include "FileSystem/LocalizationSystem.h"
#include "UI/UIControlSystem.h"

namespace DAVA
{
FTManager::FTManager()
{
    FT_Error error = FT_Init_FreeType(&library);
    if (error)
    {
        Logger::Error("FontManager FT_Init_FreeType failed");
    }
    error = FTC_Manager_New(library, 0, 0, 0, &FaceRequester, 0, &manager);
    if (error)
    {
        Logger::Error("FontManager FTC_Manager_New failed");
    }
    error = FTC_ImageCache_New(manager, &glyphcache);
    if (error)
    {
        Logger::Error("FontManager FTC_ImageCache_New failed");
    }
    error = FTC_CMapCache_New(manager, &cmapcache);
    if (error)
    {
        Logger::Error("FontManager FTC_CMapCache_New failed");
    }
}

FTManager::~FTManager()
{
    FTC_Manager_Done(manager);
    FT_Done_FreeType(library);
}

bool FTManager::LookupFace(FaceID* faceId, FT_Face* face)
{
    FT_Error error = FTC_Manager_LookupFace(manager, static_cast<FTC_FaceID>(faceId), face);
    DVASSERT(error == 0);
    return error == 0;
}

bool FTManager::LookupSize(FaceID* faceId, float32 size, FT_Face* aface, FT_Size* asize)
{
    FTC_ScalerRec fontScaler =
    {
      static_cast<FTC_FaceID>(faceId),
      static_cast<FT_UInt>(UIControlSystem::Instance()->vcs->ConvertVirtualToPhysicalY(size)),
      static_cast<FT_UInt>(UIControlSystem::Instance()->vcs->ConvertVirtualToPhysicalY(size)),
      1,
      0,
      0
    };
    FT_Error error = FTC_Manager_LookupSize(manager, &fontScaler, asize);
    DVASSERT(error == 0);
    if (aface)
    {
        *aface = (*asize)->face;
    }
    return error == 0;
}

bool FTManager::LookupGlyph(FaceID* faceId, float32 size, uint32 glyphIndex, FT_Glyph* glyph)
{
    FTC_ScalerRec fontScaler =
    {
      static_cast<FTC_FaceID>(faceId),
      static_cast<FT_UInt>(UIControlSystem::Instance()->vcs->ConvertVirtualToPhysicalY(size)),
      static_cast<FT_UInt>(UIControlSystem::Instance()->vcs->ConvertVirtualToPhysicalY(size)),
      1,
      0,
      0
    };
    FT_Error error = FTC_ImageCache_LookupScaler(glyphcache, &fontScaler, FT_LOAD_DEFAULT, glyphIndex, glyph, 0);
    return error == 0;
}

uint32 FTManager::LookupGlyphIndex(FaceID* faceId, uint32 codePoint)
{
    return uint32(FTC_CMapCache_Lookup(cmapcache, faceId, 0, static_cast<FT_UInt32>(codePoint)));
}

void FTManager::RemoveFace(FaceID* faceId)
{
    FTC_Manager_RemoveFaceID(manager, static_cast<FTC_FaceID>(faceId));
}

unsigned long FTManager::StreamLoad(FT_Stream stream, unsigned long offset, unsigned char* buffer, unsigned long count)
{
    File* is = reinterpret_cast<File*>(stream->descriptor.pointer);
    if (count == 0)
        return 0;
    is->Seek(int32(offset), File::SEEK_FROM_START);
    return is->Read(buffer, uint32(count));
}

void FTManager::StreamClose(FT_Stream stream)
{
    File* file = reinterpret_cast<File*>(stream->descriptor.pointer);
    SafeRelease(file);
}

FT_Error FTManager::FaceRequester(FTC_FaceID face_id, FT_Library library, FT_Pointer request_data, FT_Face* aface)
{
    FaceID* face = static_cast<FaceID*>(face_id);

    FilePath path(face->fileName);
    FilePath pathName(path);
    pathName.ReplaceDirectory(path.GetDirectory() + (LocalizationSystem::Instance()->GetCurrentLocale() + "/"));

    File* fontFile = File::Create(pathName, File::READ | File::OPEN);
    if (!fontFile)
    {
        fontFile = File::Create(path, File::READ | File::OPEN);
        if (!fontFile)
        {
            Logger::Error("Failed to open font: %s", path.GetStringValue().c_str());
            return FT_Err_Cannot_Open_Resource;
        }
    }

    face->stream = std::make_unique<FT_StreamRec>();
    face->stream->base = 0;
    face->stream->size = static_cast<uint32>(fontFile->GetSize());
    face->stream->pos = 0;
    face->stream->descriptor.pointer = static_cast<void*>(fontFile);
    face->stream->pathname.pointer = 0;
    face->stream->read = &FTManager::StreamLoad;
    face->stream->close = &FTManager::StreamClose;
    face->stream->memory = 0;
    face->stream->cursor = 0;
    face->stream->limit = 0;

    FT_Open_Args args;
    args.flags = FT_OPEN_STREAM;
    args.memory_base = 0;
    args.memory_size = 0;
    args.pathname = 0;
    args.stream = face->stream.get();
    args.driver = 0;
    args.num_params = 0;
    args.params = 0;

    FT_Error error = FT_Open_Face(library, &args, 0, aface);

    //FT_Error error = FT_New_Face(library, path.GetAbsolutePathname().c_str(), 0, aface);
    if (error == FT_Err_Unknown_File_Format)
    {
        Logger::Error("FTInternalFont::FTInternalFont FT_Err_Unknown_File_Format: %s", fontFile->GetFilename().GetStringValue().c_str());
    }
    else if (error)
    {
        Logger::Error("FTInternalFont::FTInternalFont cannot create font(no file?): %s", fontFile->GetFilename().GetStringValue().c_str());
    }
    return error;
}

} // DAVA