/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#include "TexturePathValidator.h"

#include "CommandLine/TextureDescriptor/TextureDescriptorUtils.h"

TexturePathValidator::TexturePathValidator(const QStringList& value)
:   PathValidator(value)
{
}

bool TexturePathValidator::ValidateInternal(const QVariant &v)
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
        DAVA::String file = v.toString().toStdString();
        DAVA::FilePath filePath = DAVA::FilePath(file);
        if (!filePath.IsEmpty() && filePath.Exists())
        {
            if (filePath.GetExtension() == ".png")
            {
				DAVA::FilePath texFile = DAVA::TextureDescriptor::GetDescriptorPathname(filePath);
				bool wasCreated = TextureDescriptorUtils::CreateDescriptorIfNeed(texFile);
				if(wasCreated)
				{	// we need to reload textures in case we change path on existing on disk
					DAVA::TexturesMap texturesMap = DAVA::Texture::GetTextureMap();

					DAVA::TexturesMap::iterator found = texturesMap.find(FILEPATH_MAP_KEY(texFile));
					if(found != texturesMap.end())
					{
						DAVA::Texture *tex = found->second;
						tex->Reload();
					}
				}

                v = QVariant(QString::fromStdString(texFile.GetAbsolutePathname()));
            }
        }
    }
}
