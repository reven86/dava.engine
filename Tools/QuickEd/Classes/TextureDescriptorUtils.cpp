#include "TextureDescriptorUtils.h"

using namespace DAVA;

bool TextureDescriptorUtils::CreateDescriptorIfNeed(const FilePath& pngPathname)
{
    FilePath descriptorPathname = TextureDescriptor::GetDescriptorPathname(pngPathname);
    if (false == FileSystem::Instance()->IsFile(descriptorPathname))
    {
        TextureDescriptor* descriptor = new TextureDescriptor();
        descriptor->Save(descriptorPathname);
        delete descriptor;

        return true;
    }

    return false;
}
