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



#include "PathDescriptor.h"
#include "Render/Image/ImageSystem.h"
#include "Render/TextureDescriptor.h"
#include "Render/Highlevel/Heightmap.h"

DAVA::Vector<PathDescriptor> PathDescriptor::descriptors;

void PathDescriptor::InitializePathDescriptors()
{
    descriptors.push_back(PathDescriptor("", "All (*.*)", PathDescriptor::PATH_NOT_SPECIFIED));
    descriptors.push_back(PathDescriptor("customGeometry", "All (*.sc2);;SC2 (*.sc2);", PathDescriptor::PATH_SCENE));
    descriptors.push_back(PathDescriptor("textureSheet", "All (*.tex);;TEX (*.tex)", PathDescriptor::PATH_TEXTURE_SHEET));
    
    QString sourceFileString;
    QString separateSourceFileString;
    
    for(auto formatType : DAVA::TextureDescriptor::sourceTextureTypes)
    {
        QString fileTypeString;
        
        auto extensions = DAVA::ImageSystem::Instance()->GetExtensionsFor(formatType);
        
        for(auto& ex : extensions)
        {
            if(fileTypeString.isEmpty())
            {
                fileTypeString = QString(DAVA::ImageSystem::Instance()->GetImageFormatInterface(formatType)->Name()) + " (*";
            }
            else
            {
                fileTypeString += QString(" *");
            }
            fileTypeString += QString(ex.c_str());
            
            if(sourceFileString.isEmpty())
            {
                sourceFileString = "*";
            }
            else
            {
                sourceFileString += " *";
            }
            sourceFileString += ex.c_str();
        }
        
        fileTypeString += ")";
        
        
        if(!separateSourceFileString.isEmpty())
        {
            separateSourceFileString += QString(";;");
        }
        separateSourceFileString += fileTypeString;
    }
    
    
    QString imageFilter = QString("All (%1);;%2").arg(sourceFileString).arg(separateSourceFileString);
    
    auto texExtension = DAVA::TextureDescriptor::GetDescriptorExtension();
    QString textureFilter = QString("All (*%1 %2);;TEX (*%3);;%4").arg(texExtension.c_str()).arg(sourceFileString).arg(texExtension.c_str()).arg(separateSourceFileString);
    auto heightExtension = DAVA::Heightmap::FileExtension();
    QString heightmapFilter = QString("All (*%1 %2);;Heightmap (*%3);;%4").arg(heightExtension.c_str()).arg(sourceFileString).arg(heightExtension.c_str()).arg(separateSourceFileString);
    
    descriptors.push_back(PathDescriptor("heightmapPath", heightmapFilter, PathDescriptor::PATH_HEIGHTMAP));
    descriptors.push_back(PathDescriptor("densityMap", imageFilter, PathDescriptor::PATH_IMAGE));
    descriptors.push_back(PathDescriptor("texture", textureFilter, PathDescriptor::PATH_TEXTURE));
    descriptors.push_back(PathDescriptor("lightmap", textureFilter, PathDescriptor::PATH_TEXTURE));
    descriptors.push_back(PathDescriptor("densityMap", textureFilter, PathDescriptor::PATH_TEXTURE));
}


PathDescriptor & PathDescriptor::GetPathDescriptor(PathDescriptor::eType type)
{
    for(auto & descr: descriptors)
    {
        if(descr.pathType == type)
        {
            return descr;
        }
    }
 
    return GetPathDescriptor(PATH_NOT_SPECIFIED);
}

