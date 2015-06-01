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


#include "HeightMapValidator.h"
#include "Utils/StringFormat.h"
#include <QMessageBox>

HeightMapValidator::HeightMapValidator(const QStringList& value):
	PathValidator(value),
    notifyMessage("")
{
}

void HeightMapValidator::ErrorNotifyInternal(const QVariant &v) const
{
    QMessageBox::warning(NULL, "Wrong file selected", notifyMessage.c_str(), QMessageBox::Ok);
}

bool HeightMapValidator::ValidateInternal(const QVariant &v)
{
    if(!PathValidator::ValidateInternal(v))
    {
        notifyMessage = PrepareErrorMessage(v);
        return false;
    }
    
    DAVA::FilePath path(v.toString().toStdString());
    if(path.IsEmpty() || path.IsEqualToExtension(".heightmap"))
    {
        return true;
    }
    else
    {
        auto extension = path.GetExtension();
        auto imageFormat = DAVA::ImageSystem::Instance()->GetImageFormatForExtension(extension);
        
        if(DAVA::IMAGE_FORMAT_UNKNOWN != imageFormat)
        {
            auto imgSystem = DAVA::ImageSystem::Instance()->GetImageFormatInterface(imageFormat);
            DAVA::Size2i size = imgSystem->GetImageInfo(path).GetImageSize();
            if(size.dx != size.dy)
            {
                notifyMessage = DAVA::Format("\"%s\" has wrong size: landscape requires square heightmap.",
                                             path.GetAbsolutePathname().c_str());
                return false;
            }
            
            if(((size.dx & 1) == 0) || !DAVA::IsPowerOf2(size.dx - 1))
            {
                notifyMessage = DAVA::Format("\"%s\" has wrong size: landscape requires square heightmap with size (2^n + 1).",
                                             path.GetAbsolutePathname().c_str());
                return false;
            }
            
            
            DAVA::Vector<DAVA::Image *> imageVector;
            DAVA::ImageSystem::Instance()->Load(path, imageVector);
            DVASSERT(imageVector.size());
            
            DAVA::PixelFormat format = imageVector[0]->GetPixelFormat();
            
            for_each(imageVector.begin(), imageVector.end(), DAVA::SafeRelease<DAVA::Image>);
            if(format == DAVA::FORMAT_A8 ||format == DAVA::FORMAT_A16)
            {
                return true;
            }
            notifyMessage = DAVA::Format("\"%s\" is wrong: png file should be in format A8 or A16.", path.GetAbsolutePathname().c_str());
            return false;
        }
        else
        {
            notifyMessage = DAVA::Format("\"%s\" is wrong: should be *.png, *.tga, *.jpeg or *.heightmap.", path.GetAbsolutePathname().c_str());
            return false;
        }
    }

    return false;
}
