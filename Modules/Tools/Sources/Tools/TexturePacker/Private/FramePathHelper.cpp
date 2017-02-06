#include "Tools/TexturePacker/FramePathHelper.h"
#include <Utils/StringFormat.h>

namespace DAVA
{
static const char* DEFAULT_FRAME_EXTENSION = ".png";

FilePath FramePathHelper::GetFramePathRelative(const FilePath& nameWithoutExt, int32 frameIndex)
{
    return FormatFramePath(nameWithoutExt.GetAbsolutePathname(), frameIndex);
}

FilePath FramePathHelper::GetFramePathAbsolute(const FilePath& directory, const String& nameWithoutExt,
                                               int32 frameIndex)
{
    FilePath resultPath(directory, FormatFramePath(nameWithoutExt, frameIndex));
    return resultPath;
}

String FramePathHelper::FormatFramePath(const String& fileNameWithoutExt, int32 frameIndex)
{
    return Format("%s_%d%s", fileNameWithoutExt.c_str(), frameIndex, DEFAULT_FRAME_EXTENSION);
}
};
