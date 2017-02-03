#ifndef __DAVAENGINE_DEFINITION_FILE_H__
#define __DAVAENGINE_DEFINITION_FILE_H__

#include "Base/RefPtr.h"
#include "Base/BaseTypes.h"
#include "FileSystem/FilePath.h"
#include "Math/Math2D.h"

namespace DAVA
{
class DefinitionFile : public BaseObject
{
public:
    using Pointer = DAVA::RefPtr<DefinitionFile>;
    using Collection = Vector<Pointer>;

public:
    DefinitionFile() = default;
    DefinitionFile(const DefinitionFile&) = delete;
    DefinitionFile& operator=(const DefinitionFile&) = delete;
    DefinitionFile(DefinitionFile&&) = delete;

    bool Load(const FilePath& filename);
    bool LoadPNGDef(const FilePath& filename, const FilePath& pathToProcess);

    void ClearPackedFrames();
    void LoadPNG(const FilePath& fullname, const FilePath& processDirectoryPath);
    bool LoadPSD(const FilePath& fullname, const FilePath& processDirectoryPath,
                 DAVA::uint32 maxTextureSize, bool retainEmptyPixesl, bool useLayerNames,
                 bool verboseOutput);

    Size2i GetFrameSize(uint32 frame) const;
    int GetFrameWidth(uint32 frame) const;
    int GetFrameHeight(uint32 frame) const;

public:
    FilePath filename;
    Vector<String> frameNames;
    Vector<Rect2i> frameRects;
    uint32 frameCount = 0;
    uint32 spriteWidth = 0;
    uint32 spriteHeight = 0;
};
};


#endif // __DAVAENGINE_DEFINITION_FILE_H__