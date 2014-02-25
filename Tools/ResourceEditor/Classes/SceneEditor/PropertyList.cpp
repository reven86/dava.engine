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




#include "PropertyList.h"

#include "PropertyCell.h"

#include "ControlsFactory.h"

PropertyList::PropertyList(const Rect &rect, PropertyListDelegate *propertiesDelegate)
:UIControl(rect)
{
    delegate = propertiesDelegate;
    background->SetDrawType(UIControlBackground::DRAW_FILL);
    background->SetColor(Color(0.5f, 0.5f, 0.5f, 0.5f));

    propsList = new UIList(Rect(0,0, size.x, size.y), UIList::ORIENTATION_VERTICAL);
    ControlsFactory::SetScrollbar(propsList);
    propsList->SetDelegate(this);
    AddControl(propsList);
    currentSection = NULL;
}

PropertyList::~PropertyList()
{
    ReleaseProperties();
    
    SafeRelease(propsList);
}

void PropertyList::AddStringProperty(const String &propertyName, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_STRING);
    p->cellType = PropertyCell::PROP_CELL_TEXT;
    p->SetString("");
    AddProperty(p, propertyName, propEditType);
}

void PropertyList::AddIntProperty(const String &propertyName, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_INTEGER);
    p->cellType = PropertyCell::PROP_CELL_TEXT;
    p->SetInt(0);
    AddProperty(p, propertyName, propEditType);
}

void PropertyList::AddFloatProperty(const String &propertyName, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_FLOAT);
    p->cellType = PropertyCell::PROP_CELL_TEXT;
    p->SetFloat(0.f);
    AddProperty(p, propertyName, propEditType);
}


void PropertyList::AddFilepathProperty(const String &propertyName, const String &extensionFilter, bool clearDataEnabled, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_FILEPATH);
    p->cellType = PropertyCell::PROP_CELL_FILEPATH;
    p->SetFilePath(FilePath());
    p->SetClearDataEnabled(clearDataEnabled);
    p->SetExtensionFilter(extensionFilter);
    AddProperty(p, propertyName, propEditType);
}

void PropertyList::AddBoolProperty(const String &propertyName, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_BOOL);
    p->cellType = PropertyCell::PROP_CELL_BOOL;
    p->SetBool(false);
    AddProperty(p, propertyName, propEditType);
}

void PropertyList::AddProperty(PropertyCellData *newProp, const String &propertyName, editableType propEditType)
{
    newProp->key = propertyName;
    newProp->isEditable = propEditType == PROPERTY_IS_EDITABLE;
    newProp->index = props.size();
    props.push_back(newProp);
    propsMap[propertyName] = newProp;
    if (currentSection)
    {
        currentSection->SetSectionElementsCount(currentSection->GetSectionElementsCount() + 1);
    }
    propsList->Refresh();
}

void PropertyList::SetStringPropertyValue(const String &propertyName, const String &newText)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetString(newText);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

void PropertyList::SetIntPropertyValue(const String &propertyName, int32 newIntValue)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetInt(newIntValue);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

void PropertyList::SetFloatPropertyValue(const String &propertyName, float32 newFloatValue)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetFloat(newFloatValue);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

void PropertyList::SetFilepathPropertyValue(const String &propertyName, const FilePath &currentFilepath)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetFilePath(currentFilepath);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

void PropertyList::SetBoolPropertyValue(const String &propertyName, bool newBoolValue)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetBool(newBoolValue);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}


const String &PropertyList::GetStringPropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetString();   
}

int32 PropertyList::GetIntPropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetInt();   
}

float32 PropertyList::GetFloatPropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetFloat();   
}

const FilePath &PropertyList::GetFilepathPropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetFilePath();
}

bool PropertyList::GetBoolPropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetBool();   
}

bool PropertyList::IsPropertyAvaliable(const String &propertyName)
{
	Map<String, PropertyCellData*>::const_iterator it;
	it = propsMap.find(propertyName);
    
    return it != propsMap.end();
}


PropertyCellData *PropertyList::PropertyByName(const String &propertyName)
{
	Map<String, PropertyCellData*>::const_iterator it;
	it = propsMap.find(propertyName);
    
    DVASSERT(it != propsMap.end());
    
    return it->second;
}

void PropertyList::OnPropertyChanged(PropertyCellData *changedProperty)
{
    if(!delegate)   return;
    
    //ADD Combobox
    switch (changedProperty->GetValueType())
    {
        case PropertyCellData::PROP_VALUE_STRING:
        {
            switch (changedProperty->cellType)
            {
                case PropertyCell::PROP_CELL_TEXT:
                    delegate->OnStringPropertyChanged(this, changedProperty->key, changedProperty->GetString());
                    break;
            }
        }
            break;
        case PropertyCellData::PROP_VALUE_INTEGER:
            delegate->OnIntPropertyChanged(this, changedProperty->key, changedProperty->GetInt());
            break;
        case PropertyCellData::PROP_VALUE_FLOAT:
            delegate->OnFloatPropertyChanged(this, changedProperty->key, changedProperty->GetFloat());
            break;
        case PropertyCellData::PROP_VALUE_BOOL:
            delegate->OnBoolPropertyChanged(this, changedProperty->key, changedProperty->GetBool());
            break;
        case PropertyCellData::PROP_VALUE_COMBO_BOX:
            delegate->OnComboIndexChanged(this, changedProperty->key, changedProperty->GetItemIndex(), changedProperty->GetStringVector()[changedProperty->GetItemIndex()]);
            break;
        case PropertyCellData::PROP_VALUE_MATRIX4:
            delegate->OnMatrix4Changed(this, changedProperty->key, changedProperty->GetMatrix4());
            break;
        case PropertyCellData::PROP_VALUE_SECTION:
            delegate->OnSectionExpanded(this, changedProperty->key, changedProperty->GetIsSectionOpened());
            propsList->Refresh();
            break;
        case PropertyCellData::PROP_VALUE_COLOR:
            delegate->OnColorPropertyChanged(this, changedProperty->key, changedProperty->GetColor());
            break;
        case PropertyCellData::PROP_VALUE_SUBSECTION:
            break;
        case PropertyCellData::PROP_VALUE_SLIDER:
            delegate->OnSliderPropertyChanged(this, changedProperty->key, changedProperty->GetSliderValue());
            break;
        case PropertyCellData::PROP_VALUE_TEXTUREPREVIEW:
            delegate->OnTexturePreviewPropertyChanged(this, changedProperty->key, changedProperty->GetBool());
            break;
        case PropertyCellData::PROP_VALUE_DISTANCE:
            delegate->OnDistancePropertyChanged(this, changedProperty->key, changedProperty->GetFloat(), changedProperty->GetInt());
            break;
        case PropertyCellData::PROP_VALUE_FILEPATH:
            delegate->OnFilepathPropertyChanged(this, changedProperty->key, changedProperty->GetFilePath());
            break;
    }
}



int32 PropertyList::ElementsCount(UIList *)
{
    int32 count = 0;
    for (int32 i = 0; i < (int32)props.size(); i++)
    {
        if (props[i]->GetValueType() == PropertyCellData::PROP_VALUE_SECTION)
        {
            if (props[i]->GetIsSectionOpened())
            {
                count += props[i]->GetSectionElementsCount();
            }
            else 
            {
                count++;
            }

            i += props[i]->GetSectionElementsCount()-1;
        }
        else 
        {
            count++;
        }
    }
    return count;
}

int32 PropertyList::GetRealIndex(int32 index)
{
    int32 realIndex = 0;
    for (int32 i = 0; i <= index; i++) 
    {
        if (realIndex > 0 && props[realIndex - 1]->GetValueType() == PropertyCellData::PROP_VALUE_SECTION)
        {
            if (!props[realIndex - 1]->GetIsSectionOpened())
            {
                realIndex += props[realIndex - 1]->GetSectionElementsCount();
            }
            else 
            {
                realIndex++;
            }
        }
        else 
        {
            realIndex++;
        }
    }
    return realIndex - 1;
}

UIListCell *PropertyList::CellAtIndex(UIList *forList, int32 index)
{
    
    index = GetRealIndex(index);
    
    PropertyCell *c = (PropertyCell *)forList->GetReusableCell(PropertyCell::GetTypeName(props[index]->cellType));
    if (!c) 
    {
        switch (props[index]->cellType) 
        {
            case PropertyCell::PROP_CELL_TEXT:
                c = new PropertyTextCell(this, props[index], size.x);
                break;
            case PropertyCell::PROP_CELL_FILEPATH:
                c = new PropertyFilepathCell(this, props[index], size.x);
                break;
            case PropertyCell::PROP_CELL_BOOL:
                c = new PropertyBoolCell(this, props[index], size.x);
                break;
            case PropertyCell::PROP_CELL_COMBO:
                c = new PropertyComboboxCell(this, props[index], size.x);
                break;
            case PropertyCell::PROP_CELL_MATRIX4:
                c = new PropertyMatrix4Cell(this, props[index], size.x);
                break;
            case PropertyCell::PROP_CELL_SECTION:
                c = new PropertySectionCell(this, props[index], size.x);
                break;
            case PropertyCell::PROP_CELL_BUTTON:
                c = new PropertyButtonCell(this, props[index], size.x);
                break;
            case PropertyCell::PROP_CELL_COLOR:
                c = new PropertyColorCell(this, props[index], size.x);
                break;
            case PropertyCell::PROP_CELL_SUBSECTION:
                c = new PropertySubsectionCell(this, props[index], size.x);
                break;
            case PropertyCell::PROP_CELL_SLIDER:
                c = new PropertySliderCell(this, props[index], size.x);
                break;
            case PropertyCell::PROP_CELL_TEXTUREPREVIEW:
                c = new PropertyTexturePreviewCell(this, props[index], size.x);
                break;
            case PropertyCell::PROP_CELL_DISTANCE:
                c = new PropertyDistanceCell(this, props[index], size.x);
                break;
        }
    }
    else 
    {
        c->SetData(props[index]);
    }


    
    return c;
}

int32 PropertyList::CellHeight(UIList *, int32 index)
{
    index = GetRealIndex(index);
    switch (props[index]->cellType) 
    {
        case PropertyCell::PROP_CELL_TEXT:
            return (int32)PropertyTextCell::GetHeightForWidth(size.x);
        case PropertyCell::PROP_CELL_FILEPATH:
            return (int32)PropertyFilepathCell::GetHeightForWidth(size.x);
        case PropertyCell::PROP_CELL_BOOL:
            return (int32)PropertyBoolCell::GetHeightForWidth(size.x);
        case PropertyCell::PROP_CELL_COMBO:
            return (int32)PropertyComboboxCell::GetHeightForWidth(size.x);
        case PropertyCell::PROP_CELL_MATRIX4:
            return (int32)PropertyMatrix4Cell::GetHeightForWidth(size.x);
        case PropertyCell::PROP_CELL_SECTION:
            return (int32)PropertySectionCell::GetHeightForWidth(size.x);
        case PropertyCell::PROP_CELL_BUTTON:
            return (int32)PropertyButtonCell::GetHeightForWidth(size.x);
        case PropertyCell::PROP_CELL_COLOR:
            return (int32)PropertyColorCell::GetHeightForWidth(size.x);
        case PropertyCell::PROP_CELL_SUBSECTION:
            return (int32)PropertySubsectionCell::GetHeightForWidth(size.x);
        case PropertyCell::PROP_CELL_SLIDER:
            return (int32)PropertySliderCell::GetHeightForWidth(size.x);
        case PropertyCell::PROP_CELL_TEXTUREPREVIEW:
            return (int32)PropertyTexturePreviewCell::GetHeightForWidth(size.x);
        case PropertyCell::PROP_CELL_DISTANCE:
            return (int32)PropertyDistanceCell::GetHeightForWidth(size.x, props[index]->GetDistancesCount());
    }
    return 50;//todo: rework
}

void PropertyList::OnCellSelected(UIList *, UIListCell *)
{
//    PropertySectionHeaderCell *sectionHeader = dynamic_cast<PropertySectionHeaderCell *> (selectedCell);
//    if(sectionHeader)
//    {
//        sectionHeader->ToggleExpand();
//        forList->Refresh();
//    }
}

//void PropertyList::AddPropertyByData(PropertyCellData *newProp)
//{
//    newProp->index = props.size();
//    props.push_back(SafeRetain(newProp));
//    propsMap[newProp->key] = newProp;
//    propsList->Refresh();
//}

void PropertyList::ReleaseProperties()
{
	propsMap.clear();
    
	for_each(props.begin(), props.end(),  SafeRelease<PropertyCellData>);
    props.clear();
    
    propsList->Refresh();
    currentSection = NULL;
}


void PropertyList::AddComboProperty(const String &propertyName, const Vector<String> &strings, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_COMBO_BOX);
    p->cellType = PropertyCell::PROP_CELL_COMBO;
    p->SetStringVector(strings);
    p->SetItemIndex(0);
    AddProperty(p, propertyName, propEditType);
}


void PropertyList::SetComboPropertyStrings(const String &propertyName, const Vector<String> &strings)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetStringVector(strings);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

void PropertyList::SetComboPropertyIndex(const String &propertyName, int32 currentStringIndex)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetItemIndex(currentStringIndex);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

const String &PropertyList::GetComboPropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetStringVector()[p->GetItemIndex()];
}

const int32 PropertyList::GetComboPropertyIndex(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetItemIndex();
}

void PropertyList::AddMatrix4Property(const String &propertyName, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_MATRIX4);
    p->cellType = PropertyCell::PROP_CELL_MATRIX4;
    p->SetMatrix4(Matrix4());
    AddProperty(p, propertyName, propEditType);
}

void PropertyList::SetMatrix4PropertyValue(const String &propertyName, const Matrix4 &currentMatrix)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetMatrix4(currentMatrix);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

const Matrix4 & PropertyList::GetMatrix4PropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetMatrix4();
}

void PropertyList::AddSection(const String &sectionName, bool expanded)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_SECTION);
    p->cellType = PropertyCell::PROP_CELL_SECTION;
    p->SetIsSectionOpened(expanded);
    currentSection = p;
    AddProperty(p, sectionName, PROPERTY_IS_EDITABLE);
}

void PropertyList::SetSectionIsOpened(const String &sectionName, bool isOpened)
{
    PropertyCellData *p = PropertyByName(sectionName);
    p->SetIsSectionOpened(isOpened);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
    propsList->Refresh();
}

bool PropertyList::GetSectionIsOpened(const String &sectionName)
{
    PropertyCellData *p = PropertyByName(sectionName);
    return p->GetIsSectionOpened();
}


void PropertyList::AddMessageProperty(const String &propertyName, const Message &newMessage)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_MESSAGE);
    p->cellType = PropertyCell::PROP_CELL_BUTTON;
    p->SetMessage(newMessage);
    AddProperty(p, propertyName, PROPERTY_IS_READ_ONLY);
}

void PropertyList::SetMessagePropertyValue(const String &propertyName, const Message &newMessage)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetMessage(newMessage);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

const List<UIControl*> & PropertyList::GetVisibleCells()
{
    return propsList->GetVisibleCells();
}

void PropertyList::AddColorProperty(const String &propertyName)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_COLOR);
    p->cellType = PropertyCell::PROP_CELL_COLOR;
    p->SetColor(Color());
    AddProperty(p, propertyName, PROPERTY_IS_EDITABLE);
}

void PropertyList::SetColorPropertyValue(const String &propertyName, const Color &newColor)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetColor(newColor);
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

const Color & PropertyList::GetColorPropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetColor();
}

void PropertyList::AddSubsection(const String &subsectionName)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_SUBSECTION);
    p->cellType = PropertyCell::PROP_CELL_SUBSECTION;
    AddProperty(p, subsectionName, PROPERTY_IS_READ_ONLY);
}


void PropertyList::AddSliderProperty(const String &propertyName, bool showEdges)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_SLIDER);
    p->cellType = PropertyCell::PROP_CELL_SLIDER;
    p->SetSliderValue(0.f, 1.0f, 0.5f);
    p->SetBool(showEdges);
    
    AddProperty(p, propertyName, PROPERTY_IS_EDITABLE);
}

void PropertyList::SetSliderPropertyValue(const String &propertyName, float32 newMinValue, float32 newMaxValue, float32 newValue)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetSliderValue(newMinValue, newMaxValue, newValue);
    
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

float32 PropertyList::GetSliderPropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetSliderValue();
}

void PropertyList::AddTexturePreviewProperty(const String &propertyName, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_TEXTUREPREVIEW);
    p->cellType = PropertyCell::PROP_CELL_TEXTUREPREVIEW;
    p->SetBool(false);
    p->SetTexture(NULL);
    AddProperty(p, propertyName, propEditType);
}

void PropertyList::SetTexturePreviewPropertyValue(const String &propertyName, bool newBoolValue, DAVA::Texture *newTexture)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetBool(newBoolValue);
    p->SetTexture(newTexture);
    
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

bool PropertyList::GetTexturePreviewPropertyValue(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    return p->GetBool();   
}

void PropertyList::AddDistanceProperty(const String &propertyName, editableType propEditType)
{
    PropertyCellData *p = new PropertyCellData(PropertyCellData::PROP_VALUE_DISTANCE);
    p->cellType = PropertyCell::PROP_CELL_DISTANCE;
    p->SetDistances(NULL, 0);
    p->SetTriangles(NULL, 0);
    AddProperty(p, propertyName, propEditType);
}

void PropertyList::SetDistancePropertyValue(const String &propertyName, float32 *distances, int32 *triangles, int32 count)
{
    PropertyCellData *p = PropertyByName(propertyName);
    p->SetDistances(distances, count);
    p->SetTriangles(triangles, count);
    
    if (p->currentCell) 
    {
        p->currentCell->SetData(p);
    }
}

int32 PropertyList::GetDistancePropertyCount(const String &propertyName)
{
    PropertyCellData *p = PropertyByName(propertyName);
    
    return p->GetDistancesCount();
}


float32 PropertyList::GetDistancePropertyValue(const String &propertyName, int32 index)
{
    PropertyCellData *p = PropertyByName(propertyName);
    
    DVASSERT((0 <= index) && (index < p->GetDistancesCount()));
    return p->GetDistances()[index];   
}

void PropertyList::SetSize(const Vector2 &newSize)
{
    ControlsFactory::RemoveScrollbar(propsList);

    
    UIControl::SetSize(newSize);
    
    propsList->SetSize(newSize);
    propsList->Refresh();
    
    ControlsFactory::SetScrollbar(propsList);
}

