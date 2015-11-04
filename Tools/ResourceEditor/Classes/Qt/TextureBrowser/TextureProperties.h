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


#ifndef __TEXTURE_PROPERTIES_H__
#define __TEXTURE_PROPERTIES_H__

#include "DAVAEngine.h"
#include "Base/EnumMap.h"

#include "Tools/QtPropertyEditor/QtPropertyEditor.h"
#include "Tools/QtPropertyEditor/QtPropertyData.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataMetaObject.h"
#include "Tools/QtPropertyEditor/QtPropertyData/QtPropertyDataInspMember.h"

class LazyUpdater;
class TextureProperties : public QtPropertyEditor
{
	Q_OBJECT

public:
	typedef enum PropType
	{
		PROP_MIPMAP,
        PROP_NORMALMAP,
		PROP_WRAP,
		PROP_FILTER,
		PROP_FORMAT,
		PROP_SIZE
	} PropertiesType;

public:
    TextureProperties(QWidget* parent = nullptr);
    ~TextureProperties() override;

    void setTextureDescriptor(DAVA::TextureDescriptor* descriptor);
    void setTextureGPU(DAVA::eGPUFamily gpu);

    const DAVA::TextureDescriptor* getTextureDescriptor();
    void setOriginalImageSize(const QSize &size);

signals:
	void PropertyChanged(int type);

private slots:

    void OnPropertyChanged(int type);

protected:
    void OnItemEdited(const QModelIndex&) override;

    void Save();

    void MipMapSizesInit(int baseWidth, int baseHeight);
    void MipMapSizesReset();

    void ReloadEnumFormats();
    void ReloadEnumWrap();
    void ReloadEnumFilters();
    void ReloadProperties();

    QtPropertyDataInspMember* AddPropertyItem(const DAVA::FastName& name, DAVA::InspBase* object, const QModelIndex& parent);
    void SetPropertyItemValidValues(QtPropertyDataInspMember* item, EnumMap* validValues);
    void SetPropertyItemValidValues(QtPropertyDataMetaObject* item, EnumMap* validValues);

    void LoadCurSizeToProp();
    void SaveCurSizeFromProp();

protected:
    DAVA::TextureDescriptor* curTextureDescriptor = nullptr;
    DAVA::eGPUFamily curGPU;

    QtPropertyDataInspMember* propMipMap = nullptr;
    QtPropertyDataInspMember* propNormalMap = nullptr;
    QtPropertyDataInspMember* propWrapModeS = nullptr;
    QtPropertyDataInspMember* propWrapModeT = nullptr;
    QtPropertyDataInspMember* propMinFilter = nullptr;
    QtPropertyDataInspMember* propMagFilter = nullptr;
    QtPropertyDataInspMember* propMipFilter = nullptr;
    QtPropertyDataInspMember* propFormat = nullptr;
    QtPropertyDataMetaObject* propSizes = nullptr;

    bool skipPropSizeChanged;

    QSize origImageSize;
    int curSizeLevelObject;

    EnumMap enumFormats;
	EnumMap enumSizes;
	EnumMap enumWpar;
	EnumMap enumFiltersMin;
	EnumMap enumFiltersMag;
    EnumMap enumFiltersMip;

    QMap<int, QSize> availableSizes;

    LazyUpdater* updater = nullptr;
};

#endif // __TEXTURE_PROPERTIES_H__
