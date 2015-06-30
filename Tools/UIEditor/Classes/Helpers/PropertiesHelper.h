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


#ifndef __UIEditor__PropertiesHelper__
#define __UIEditor__PropertiesHelper__

#include <QString>
#include <QVariant>

#include "BaseMetadata.h"
#include "BaseMetadataParams.h"

namespace DAVA {
    
// Helper class to get/set Metadata Properties.
class PropertiesHelper
{
public:
    // Get/set the property value for the particular param index.
    template <typename T>
        static inline T GetPropertyValue(BaseMetadata* activeMetadata, const QString& propertyName,
                                     BaseMetadataParams::METADATAPARAMID paramID);
    template <typename T>
        static inline void SetPropertyValue(BaseMetadata* activeMetadata, const QString& propertyName,
                                     BaseMetadataParams::METADATAPARAMID paramID, const T& value);
    
    // Get all property values. isPropertyValueDiffers will be set to TRUE in case the values are different
    // for different properties.
	template<typename T>
		static inline T GetAllPropertyValues(BaseMetadata* activeMetadata, const QString& propertyName);

    template<typename T>
        static inline T GetAllPropertyValues(BaseMetadata* activeMetadata, const QString& propertyName,
                                             bool& isPropertyValueDiffers);

    template<typename T>
        static inline void SetAllPropertyValues(BaseMetadata* activeMetadata, const QString& propertyName, const T& value);
};

// Implementation.
template<typename T> inline T PropertiesHelper::GetPropertyValue(BaseMetadata* activeMetadata, const QString& propertyName,
                                                                 BaseMetadataParams::METADATAPARAMID paramID)
{
    if (activeMetadata == NULL)
    {
        return T();
    }

    activeMetadata->SetActiveParamID(paramID);
    const T paramValue = activeMetadata->property(propertyName.toStdString().c_str()).value<T>();

    return paramValue;
}

template<typename T>
    inline void PropertiesHelper::SetPropertyValue(BaseMetadata* activeMetadata, const QString& propertyName,
                                                BaseMetadataParams::METADATAPARAMID paramID,
                                                const T& value)
{
    if (activeMetadata == NULL)
    {
        return;
    }

    BaseMetadataParams::METADATAPARAMID currentParamID = activeMetadata->GetActiveParamID();
    activeMetadata->SetActiveParamID(paramID);
    activeMetadata->setProperty(propertyName.toStdString().c_str(), QVariant::fromValue<T>(value));
    activeMetadata->SetActiveParamID(currentParamID);
}

template<typename T>
    inline T PropertiesHelper::GetAllPropertyValues(BaseMetadata* activeMetadata,
                                                    const QString& propertyName)
{
	bool propertyValueDiffers = false;
	return GetAllPropertyValues<T>(activeMetadata, propertyName, propertyValueDiffers);
}


template<typename T>
    inline T PropertiesHelper::GetAllPropertyValues(BaseMetadata* activeMetadata,
                                                    const QString& propertyName,
                                                    bool& isPropertyValueDiffers)
{
    if (activeMetadata == NULL)
    {
        return T();
    }
        
    int paramsCount = activeMetadata->GetParamsCount();
    T firstValue;
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
		activeMetadata->SetActiveParamID(i);

		for (uint32 stateIndex = 0; stateIndex < activeMetadata->GetStatesCount(); ++stateIndex)
		{
			activeMetadata->SetActiveStateIndex(stateIndex);
			const T& curValue = activeMetadata->property(propertyName.toStdString().c_str()).value<T>();
            if (i == 0 && stateIndex == 0)
            {
                firstValue = curValue;
            }
			else if (curValue != firstValue)
			{
				isPropertyValueDiffers = true;
				break;
			}
		}

		if (isPropertyValueDiffers)
		{
			break;
		}
    }

	activeMetadata->ResetActiveStateIndex();
    return firstValue;
}
    
template<typename T>
    inline void PropertiesHelper::SetAllPropertyValues(BaseMetadata* activeMetadata,
                                                       const QString& propertyName,
                                                       const T& value)
{
    if (activeMetadata == NULL)
    {
        return;
    }

    // Set the same property for all Params inside the metadata.
    int paramsCount = activeMetadata->GetParamsCount();
    for (BaseMetadataParams::METADATAPARAMID i = 0; i < paramsCount; i ++)
    {
        activeMetadata->SetActiveParamID(i);
        activeMetadata->setProperty(propertyName.toStdString().c_str(), QVariant::fromValue<T>(value));
    }
}

}
#include <iostream>

#endif /* defined(__UIEditor__PropertiesHelper__) */
