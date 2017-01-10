#include "Render/2D/SpriteUtils.h"
#include "Render/2D/Sprite.h"

#include "FileSystem/FilePath.h"

#include "Render/Image/Image.h"
#include "Render/Image/ImageSystem.h"

namespace DAVA
{
namespace SpriteUtils
{
Sprite* CreateFromImagePath(const FilePath& imagepath)
{
    ScopedPtr<Image> image(ImageSystem::LoadSingleMip(imagepath));
    return Sprite::CreateFromImage(image);
}
}
}