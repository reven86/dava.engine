#include "TexturePathValidator.h"

#include "Utils/TextureDescriptor/TextureDescriptorUtils.h"
#include "Classes/Settings/Settings.h"

#include "FileSystem/FileSystem.h"

TexturePathValidator::TexturePathValidator(const QStringList& value)
    : PathValidator(value)
{
}

bool TexturePathValidator::ValidateInternal(const QVariant& v)
{
    bool res = RegExpValidator::ValidateInternal(v);

    QString val = v.toString();
    if (res && val != "")
    {
        res = val.endsWith(QString::fromStdString(DAVA::TextureDescriptor::GetDescriptorExtension()));
    }

    return res;
}

void TexturePathValidator::FixupInternal(QVariant& v) const
{
    if (v.type() == QVariant::String)
    {
        DAVA::FilePath texturePath = DAVA::FilePath(v.toString().toStdString());
        if (DAVA::FileSystem::Instance()->Exists(texturePath) && TextureDescriptorUtils::CreateOrUpdateDescriptor(texturePath))
        {
            DAVA::FilePath descriptorPath = DAVA::TextureDescriptor::GetDescriptorPathname(texturePath);

            auto& texturesMap = DAVA::Texture::GetTextureMap();
            auto found = texturesMap.find(FILEPATH_MAP_KEY(descriptorPath));
            if (found != texturesMap.end())
            {
                found->second->ReloadAs(Settings::GetGPUFormat());
            }

            v = QVariant(QString::fromStdString(descriptorPath.GetAbsolutePathname()));
        }
    }
}
