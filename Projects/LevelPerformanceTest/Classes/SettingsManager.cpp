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


#include "SettingsManager.h"

#define H 10.f
#define ANGLE_SPEED 80.f
#define VELOCITY 40.f
#define XSIZE 100.f
#define YSIZE 100.f
#define MIN_FPS 15
#define MIN_FPS_SECTOR_COUNT 10
#define COLOR_TRANSPARENCY 127

static const FastName LANDSCAPE_NODE_NAME("Landscape");

SettingsManager sm;

void SettingsManager::InitWithFile(const FilePath &filename)
{
    landscapeNodeName = LANDSCAPE_NODE_NAME;
	landscapePartitioningSize.x = XSIZE;
	landscapePartitioningSize.y = YSIZE;
	cameraElevation = H;
	cameraRotationSpeed = ANGLE_SPEED;
	cameraMovementSpeed = VELOCITY;
	minFps = MIN_FPS;
	minFpsSectorCount = MIN_FPS_SECTOR_COUNT;
	colorTransparency = COLOR_TRANSPARENCY;

	fpsList.push_back(-std::numeric_limits<float32>::infinity());
	colorList.push_back(Color(255.f, 255.f, 255.f, COLOR_TRANSPARENCY) / 255.f);

    YamlParser* parser = YamlParser::Create(filename);
    if(parser)
    {
        YamlNode* rootNode = parser->GetRootNode();
        if(rootNode)
        {
            const YamlNode* node;
            
            node = rootNode->Get("landscapeNodeName");
            if(node)
            {
                landscapeNodeName = node->AsFastName();
            }
            
            node = rootNode->Get("landscapePartitioningSize");
            if(node && node->GetCount() == 2)
            {
				landscapePartitioningSize = node->AsVector2();
            }
            
            node = rootNode->Get("cameraElevation");
            if(node)
            {
                cameraElevation = node->AsFloat();
            }
            
            node = rootNode->Get("cameraRotationSpeed");
            if(node)
            {
                cameraRotationSpeed = node->AsFloat();
            }
            
            node = rootNode->Get("cameraSpeed");
            if(node)
            {
                cameraMovementSpeed = node->AsFloat();
            }
			
			node = rootNode->Get("minFps");
			if(node)
			{
				minFps = node->AsFloat();
			}
			
			node = rootNode->Get("minFpsSectorCount");
			if(node)
			{
				minFpsSectorCount = node->AsInt();
			}
			
			node = rootNode->Get("resultTextureColorTransparency");
			if(node)
			{
				colorTransparency = node->AsInt();
			}
			
			node = rootNode->Get("list");
			if(node)
			{
				int32 listCount = node->GetCount();
				for(int32 i = 0; i < listCount; ++i)
				{
					const YamlNode* listElement = node->Get(i);
					if(listElement == 0 || listElement->GetCount() != 2)
						continue;

					const YamlNode* fpsNode = listElement->Get(0);
					const YamlNode* colorNode = listElement->Get(1);
					
					if(fpsNode == 0 || colorNode == 0 || colorNode->GetCount() != 3)
						continue;

					float32 fpsValue = fpsNode->AsFloat();
					Color color((float32)colorNode->Get(0)->AsInt(),
								(float32)colorNode->Get(1)->AsInt(),
								(float32)colorNode->Get(2)->AsInt(),
								colorTransparency);
					color /= 255.f;
					
					fpsList.push_back(fpsValue);
					colorList.push_back(color);
				}
			}
        }
    }

	fpsList.push_back(std::numeric_limits<float32>::infinity());

    SafeRelease(parser);
}

Vector2 SettingsManager::GetLandscapePartitioningSize() const
{
	return landscapePartitioningSize;
}

const FastName & SettingsManager::GetLandscapeNodeName() const
{
	return landscapeNodeName;
}

float32 SettingsManager::GetCameraElevation() const
{
	return cameraElevation;
}

float32 SettingsManager::GetCameraRotationSpeed() const
{
	return cameraRotationSpeed;
}

float32 SettingsManager::GetCameraMovementSpeed() const
{
	return cameraMovementSpeed;
}

float32 SettingsManager::GetMinFps() const
{
	return minFps;
}

uint32 SettingsManager::GetMinFpsSectorCount() const
{
	return minFpsSectorCount;
}

uint8 SettingsManager::GetColorTransparency() const
{
	return colorTransparency;
}

Color SettingsManager::GetColorByFps(float32 fps) const
{
	for(uint32 i = 0; i < fpsList.size() - 1; ++i)
    {
		if(fps >= fpsList[i] && fps < fpsList[i + 1])
        {
			return colorList[i];
		}
	}
	return colorList[0];
}
