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


#include "DistanceFontValidator.h"
#include "TextureDescriptorUtils.h"
#include "DAVAEngine.h"
#include <QMessageBox>

using namespace DAVA;

DistanceFontValidator::DistanceFontValidator()
:   notifyMessage("")
{
}

bool DistanceFontValidator::ValidateInternal(QVariant& v)
{
    if (v.type() == QVariant::String)
    {
        FilePath texPath = FilePath(v.toString().toStdString());
        FilePath pngPath = FilePath::CreateWithNewExtension(texPath, ".png");

        if (!FileSystem::Instance()->Exists(pngPath))
        {
            notifyMessage = ".png file for Distance Field Font was not found. Font will not be displayed correctly.";
            return false;
        }

        if (!FileSystem::Instance()->Exists(texPath))
        {
            notifyMessage = ".tex file for Distance Field Font was not found and could not be created. Font will not be displayed correctly.";
            return false;
        }

        return true;
    }

    return false;
}

void DistanceFontValidator::FixupInternal(QVariant& v) const
{
    if (v.type() == QVariant::String)
    {
        FilePath texPath = FilePath(v.toString().toStdString());
        FilePath pngPath = FilePath::CreateWithNewExtension(texPath, ".png");

        if (FileSystem::Instance()->Exists(pngPath))
        {
            if (TextureDescriptorUtils::CreateDescriptorIfNeed(pngPath))
            {
                TexturesMap texturesMap = Texture::GetTextureMap();

                TexturesMap::iterator found = texturesMap.find(FILEPATH_MAP_KEY(texPath));
                if (found != texturesMap.end())
                {
                    Texture* tex = found->second;
                    tex->Reload();
                }

                v = QVariant(QString::fromStdString(texPath.GetAbsolutePathname()));
            }
        }
    }
}

void DistanceFontValidator::ErrorNotifyInternal(const QVariant &v) const
{
    QMessageBox::warning(NULL, "Broken Distance Field font", notifyMessage, QMessageBox::Ok);
}
