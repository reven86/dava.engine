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


#include "Base/GlobalEnum.h"
#include "Render/TextureDescriptor.h"

#include "TextureProperties.h"
#include "Tools/QtPropertyEditor/QtPropertyItem.h"
#include "Tools/LazyUpdater/LazyUpdater.h"

namespace PropertyItemName
{
const DAVA::FastName GenerateMipMaps("generateMipMaps");
const DAVA::FastName IsNormalMap("isNormalMap");
const DAVA::FastName WrapModeS("wrapModeS");
const DAVA::FastName WrapModeT("wrapModeT");
const DAVA::FastName MinFilter("minFilter");
const DAVA::FastName MagFilter("magFilter");
const DAVA::FastName MipFilter("mipFilter");
const DAVA::FastName Format("format");
}

TextureProperties::TextureProperties(QWidget* parent /*= 0*/)
    : QtPropertyEditor(parent)
    , curTextureDescriptor(nullptr)
    , curGPU(DAVA::GPU_ORIGIN)
    , skipPropSizeChanged(false)
{
	SetEditTracking(true);

    DAVA::Function<void()> fn(this, &TextureProperties::ReloadProperties);
    updater = new LazyUpdater(fn, this);

    connect(this, &TextureProperties::PropertyChanged, this, &TextureProperties::OnPropertyChanged);
}

TextureProperties::~TextureProperties()
{
    curTextureDescriptor = nullptr;
}

void TextureProperties::setTextureDescriptor(DAVA::TextureDescriptor *descriptor)
{
	curTextureDescriptor = descriptor;
	origImageSize = QSize(0, 0);

    if (nullptr != curTextureDescriptor)
    {
        // enable this widget
        setEnabled(true);

        // reset mipmap sizes
		// we don't know avaliable mipmap sizes for newly set texture until setOriginalSize() method will be called by user
		MipMapSizesReset();

		// reload all properties for current gpu and from current descriptor 
		ReloadProperties();
	}
	else
	{
		// no texture - disable this widget
		setEnabled(false);
		RemovePropertyAll();
	}
}

void TextureProperties::setTextureGPU(DAVA::eGPUFamily gpu)
{
	if(curGPU != gpu)
	{
		curGPU = gpu;
		ReloadProperties();
	}
}

void TextureProperties::setOriginalImageSize(const QSize &size)
{
	origImageSize = size;

	// Init mipmap sizes based on original image size
	MipMapSizesInit(size.width(), size.height());
}

const DAVA::TextureDescriptor* TextureProperties::getTextureDescriptor()
{
	return curTextureDescriptor;
}

void TextureProperties::Save()
{
    if (nullptr != curTextureDescriptor)
    {
        curTextureDescriptor->Save();
    }
}

void TextureProperties::MipMapSizesInit(int baseWidth, int baseHeight)
{
	int level = 0;

	MipMapSizesReset();
	while(baseWidth > 1 && baseHeight > 1)
	{
		QSize size(baseWidth, baseHeight);
		QString shownKey;

		if(0 == level)
		{
			size = QSize(0, 0);
			shownKey = "Original";
		}
		else
		{
			shownKey.sprintf("%dx%d", baseWidth, baseHeight);
		}

		enumSizes.Register(level, shownKey.toLatin1());
		availableSizes[level] = size;

		level++;
		baseWidth = baseWidth >> 1;
		baseHeight = baseHeight >> 1;
	}

	if(enumSizes.GetCount() > 0)
	{
		LoadCurSizeToProp();
		SetPropertyItemValidValues(propSizes, &enumSizes);
		propSizes->SetEnabled(true);
	}
}

void TextureProperties::MipMapSizesReset()
{
	curSizeLevelObject = 0;
	availableSizes.clear();
	enumSizes.UnregistelAll();
}

void TextureProperties::ReloadProperties()
{
	RemovePropertyAll();

	if(NULL != curTextureDescriptor &&
		curGPU >= 0 &&
		curGPU < DAVA::GPU_DEVICE_COUNT)
	{
		QModelIndex headerIndex;
		DAVA::InspBase *textureDrawSettings = &curTextureDescriptor->drawSettings;
		DAVA::InspBase *textureDataSettings = &curTextureDescriptor->dataSettings;

		// add common texture drawSettings
		headerIndex = AppendHeader("Texture drawSettings");
        propMipMap = AddPropertyItem(PropertyItemName::GenerateMipMaps, textureDataSettings, headerIndex);
        propMipMap->SetCheckable(true);
        propMipMap->SetEditable(false);

        propNormalMap = AddPropertyItem(PropertyItemName::IsNormalMap, textureDataSettings, headerIndex);
        propNormalMap->SetCheckable(true);
        propNormalMap->SetEditable(false);

        //TODO: magic to display introspection info as bool, not int
        bool savedValue = propMipMap->GetValue().toBool();
        propMipMap->SetValue(!savedValue);
        propMipMap->SetValue(savedValue);

        savedValue = propNormalMap->GetValue().toBool();
        propNormalMap->SetValue(!savedValue);
        propNormalMap->SetValue(savedValue);
        //END of TODO

        propWrapModeS = AddPropertyItem(PropertyItemName::WrapModeS, textureDrawSettings, headerIndex);
        propWrapModeT = AddPropertyItem(PropertyItemName::WrapModeT, textureDrawSettings, headerIndex);
        propMinFilter = AddPropertyItem(PropertyItemName::MinFilter, textureDrawSettings, headerIndex);
        propMagFilter = AddPropertyItem(PropertyItemName::MagFilter, textureDrawSettings, headerIndex);
        propMipFilter = AddPropertyItem(PropertyItemName::MipFilter, textureDrawSettings, headerIndex);

        DAVA::InspBase* compressionSettings = &curTextureDescriptor->compression[curGPU];

        // add per-gpu drawSettings
        headerIndex = AppendHeader(GlobalEnumMap<DAVA::eGPUFamily>::Instance()->ToString(curGPU));
        propFormat = AddPropertyItem(PropertyItemName::Format, compressionSettings, headerIndex);

        propSizes = new QtPropertyDataMetaObject(&curSizeLevelObject, DAVA::MetaInfo::Instance<int>());
        AppendProperty("Size", propSizes, headerIndex);
        LoadCurSizeToProp();

        ReloadEnumFormats();
		ReloadEnumWrap();
		ReloadEnumFilters();

		SetPropertyItemValidValues(propWrapModeS, &enumWpar);
		SetPropertyItemValidValues(propWrapModeT, &enumWpar);
		SetPropertyItemValidValues(propMinFilter, &enumFiltersMin);
        SetPropertyItemValidValues(propMagFilter, &enumFiltersMag);
        SetPropertyItemValidValues(propMipFilter, &enumFiltersMip);
        SetPropertyItemValidValues(propFormat, &enumFormats);
        SetPropertyItemValidValues(propSizes, &enumSizes);

		if(0 == enumSizes.GetCount())
		{
			propSizes->SetEnabled(false);
		}

		expandAll();
	}
}

void TextureProperties::ReloadEnumFormats()
{
	const EnumMap *globalFormats = GlobalEnumMap<DAVA::PixelFormat>::Instance();

	enumFormats.UnregistelAll();

	const auto& availableFormats = DAVA::GPUFamilyDescriptor::GetAvailableFormatsForGpu(curGPU);
	for(auto nextFormat : availableFormats)
	{
		enumFormats.Register(nextFormat.first, globalFormats->ToString(nextFormat.first));
	}
}

void TextureProperties::ReloadEnumFilters()
{
    const EnumMap* filterFormats = GlobalEnumMap<rhi::TextureFilter>::Instance();
    const EnumMap* mipFormats = GlobalEnumMap<rhi::TextureMipFilter>::Instance();

    enumFiltersMag.UnregistelAll();
    enumFiltersMin.UnregistelAll();
    enumFiltersMip.UnregistelAll();

    // Mag
    enumFiltersMag.Register(rhi::TEXFILTER_NEAREST, filterFormats->ToString(rhi::TEXFILTER_NEAREST));
    enumFiltersMag.Register(rhi::TEXFILTER_LINEAR, filterFormats->ToString(rhi::TEXFILTER_LINEAR));

    // Min
    enumFiltersMin.Register(rhi::TEXFILTER_NEAREST, filterFormats->ToString(rhi::TEXFILTER_NEAREST));
    enumFiltersMin.Register(rhi::TEXFILTER_LINEAR, filterFormats->ToString(rhi::TEXFILTER_LINEAR));

    //Mip
    if (nullptr != propMipMap)
    {
        if (propMipMap->GetValue().toBool())
        {
            enumFiltersMip.Register(rhi::TEXMIPFILTER_NONE, mipFormats->ToString(rhi::TEXMIPFILTER_NONE));
            enumFiltersMip.Register(rhi::TEXMIPFILTER_NEAREST, mipFormats->ToString(rhi::TEXMIPFILTER_NEAREST));
            enumFiltersMip.Register(rhi::TEXMIPFILTER_LINEAR, mipFormats->ToString(rhi::TEXMIPFILTER_LINEAR));
        }
        else if (nullptr != propMipFilter)
        {
            enumFiltersMip.Register(rhi::TEXMIPFILTER_NONE, mipFormats->ToString(rhi::TEXMIPFILTER_NONE));

            propMipFilter->SetValue(QVariant(rhi::TEXMIPFILTER_NONE));
        }
    }
}

void TextureProperties::ReloadEnumWrap()
{
    const EnumMap* globalFormats = GlobalEnumMap<rhi::TextureAddrMode>::Instance();

    enumWpar.UnregistelAll();

    enumWpar.Register(rhi::TEXADDR_WRAP, globalFormats->ToString(rhi::TEXADDR_WRAP));
    enumWpar.Register(rhi::TEXADDR_CLAMP, globalFormats->ToString(rhi::TEXADDR_CLAMP));
}

QtPropertyDataInspMember* TextureProperties::AddPropertyItem(const DAVA::FastName& name, DAVA::InspBase *object, const QModelIndex &parent)
{
    QtPropertyDataInspMember* ret = nullptr;
    const DAVA::InspInfo* info = object->GetTypeInfo();

    if (nullptr != info)
    {
        const DAVA::InspMember* member = info->Member(name);
        if (nullptr != member)
        {
            ret = new QtPropertyDataInspMember(object, member);
            AppendProperty(member->Name().c_str(), ret, parent);
        }
	}

	return ret;
}

void TextureProperties::SetPropertyItemValidValues(QtPropertyDataInspMember* item, EnumMap *validValues)
{
    if (nullptr != item && nullptr != validValues)
    {
        item->ClearAllowedValues();
        
		for(size_t i = 0; i < validValues->GetCount(); ++i)
		{
			int v;

			if(validValues->GetValue(i, v))
			{
				item->AddAllowedValue(DAVA::VariantType(v), validValues->ToString(v));
			}
		}
	}
}

void TextureProperties::SetPropertyItemValidValues( QtPropertyDataMetaObject* item, EnumMap *validValues )
{
    if (nullptr != item && nullptr != validValues)
    {
        item->ClearAllowedValues();
		for(size_t i = 0; i < validValues->GetCount(); ++i)
		{
			int v;

			if(validValues->GetValue(i, v))
			{
				item->AddAllowedValue(DAVA::VariantType(v), validValues->ToString(v));
			}
		}
	}
}

void TextureProperties::OnItemEdited(const QModelIndex &index)
{
	QtPropertyEditor::OnItemEdited(index);

	QtPropertyData *data = GetProperty(index);
	if(data == propMipMap)
	{
		emit PropertyChanged(PROP_MIPMAP);
	}
    else if(data == propNormalMap)
    {
        emit PropertyChanged(PROP_NORMALMAP);
    }
	else if(data == propFormat)
	{
		emit PropertyChanged(PROP_FORMAT);
	}
    else if (data == propMinFilter || data == propMagFilter || data == propMipFilter)
    {
        emit PropertyChanged(PROP_FILTER);
    }
    else if(data == propWrapModeS || data == propWrapModeT)
	{
		emit PropertyChanged(PROP_WRAP);
	}
	else if(data == propSizes)
	{
		SaveCurSizeFromProp();

		if(!skipPropSizeChanged)
		{
			emit PropertyChanged(PROP_SIZE);
		}
	}

    Save();
}

void TextureProperties::OnPropertyChanged(int type)
{
    if (PROP_MIPMAP == type)
    {
        updater->Update();
    }
}

void TextureProperties::LoadCurSizeToProp()
{
    if (nullptr != curTextureDescriptor && nullptr != propSizes &&
        curGPU >= 0 && curGPU < DAVA::GPU_DEVICE_COUNT)
    {
        QSize curSize(curTextureDescriptor->compression[curGPU].compressToWidth, curTextureDescriptor->compression[curGPU].compressToHeight);
        int level = availableSizes.key(curSize, -1);

        if(-1 != level)
		{
			skipPropSizeChanged = true;
			propSizes->SetValue(level);
            propSizes->UpdateValue(true);
			skipPropSizeChanged = false;
		}
	}
}

void TextureProperties::SaveCurSizeFromProp()
{
    if (nullptr != curTextureDescriptor && nullptr != propSizes &&
        curGPU >= 0 && curGPU < DAVA::GPU_DEVICE_COUNT)
    {
        int level = propSizes->GetValue().toInt();

        if (availableSizes.contains(level))
        {
			DVASSERT(curTextureDescriptor->compression);
			curTextureDescriptor->compression[curGPU].compressToWidth = availableSizes[level].width();
			curTextureDescriptor->compression[curGPU].compressToHeight = availableSizes[level].height();
		}
	}
}
