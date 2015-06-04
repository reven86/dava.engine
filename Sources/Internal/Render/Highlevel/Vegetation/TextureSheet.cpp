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


#include "Render/Highlevel/Vegetation/TextureSheet.h"
#include "FileSystem/YamlParser.h"
#include "FileSystem/YamlNode.h"

namespace DAVA
{
void TextureSheet::Load(const FilePath &yamlPath)
{
    if(yamlPath.Exists())
    {
        YamlParser *parser = YamlParser::Create(yamlPath);
        YamlNode *rootNode = parser->GetRootNode();
        
        cells.clear();
        
        if(NULL != rootNode)
        {
            for(uint32 i = 0; i < rootNode->GetCount(); ++i)
            {
                if(rootNode->GetItemKeyName(i) == "cell")
                {
                    const YamlNode *cellNode = rootNode->Get(i);
                    const YamlNode *cellType = cellNode->Get("type");
                    const YamlNode *cellScale = cellNode->Get("scale");
                    const YamlNode *cellCoords = cellNode->Get("coords");
                    
                    TextureSheetCell c;
                    
                    if(NULL != cellType)
                    {
                        c.geometryId = cellType->AsUInt32();
                    }
                    
                    if(NULL != cellScale)
                    {
                        c.geometryScale = cellScale->AsVector2();
                    }
                    
                    for(uint32 j = 0; j < cellCoords->GetCount(); ++j)
                    {
                        if(j < MAX_CELL_TEXTURE_COORDS)
                        {
                            const YamlNode *singleCellCoord = cellCoords->Get(j);
                            c.coords[j] = singleCellCoord->AsVector2();
                        }
                        else
                        {
                            DVASSERT(0 && "Too much vertexes");
                        }
                    }
                    
                    cells.push_back(c);
                }
            }
        }
        
        parser->Release();
    }
}
}
