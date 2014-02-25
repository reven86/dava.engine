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




#ifndef PROPERTY_LIST
#define PROPERTY_LIST

#include "DAVAEngine.h"
#include "PropertyCellData.h"
#include "PropertyCell.h"

using namespace DAVA;

class PropertyList;
class PropertyListDelegate
{
public:
    virtual void OnStringPropertyChanged(PropertyList *forList, const String &forKey, const String &newValue){};
    virtual void OnFloatPropertyChanged(PropertyList *forList, const String &forKey, float newValue){};
    virtual void OnIntPropertyChanged(PropertyList *forList, const String &forKey, int newValue){};
    virtual void OnBoolPropertyChanged(PropertyList *forList, const String &forKey, bool newValue){};
    virtual void OnFilepathPropertyChanged(PropertyList *forList, const String &forKey, const FilePath &newValue){};
    virtual void OnComboIndexChanged(PropertyList *forList, const String &forKey, int32 newItemIndex, const String &newItemKey){};
    virtual void OnMatrix4Changed(PropertyList *forList, const String &forKey, const Matrix4 & matrix4){};
    virtual void OnSectionExpanded(PropertyList *forList, const String &forKey, bool isExpanded){};
    virtual void OnColorPropertyChanged(PropertyList *forList, const String &forKey, const Color& newColor){};
    virtual void OnSliderPropertyChanged(PropertyList *forList, const String &forKey, float32 newValue){};
    virtual void OnTexturePreviewPropertyChanged(PropertyList *forList, const String &forKey, bool newValue){};
    virtual void OnDistancePropertyChanged(PropertyList *forList, const String &forKey, float32 newValue, int32 index){};
};



class PropertyList : public UIControl, public UIListDelegate, public PropertyCellDelegate
{
public:
    
    enum editableType 
    {
        PROPERTY_IS_EDITABLE = 0,
        PROPERTY_IS_READ_ONLY
    };
protected:
    virtual ~PropertyList();
public:
    PropertyList(const Rect &rect, PropertyListDelegate *propertiesDelegate);
    
    bool IsPropertyAvaliable(const String &propertyName);

//    void AddPropertyByData(PropertyCellData *newProp);

    void AddStringProperty(const String &propertyName, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddIntProperty(const String &propertyName, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddFloatProperty(const String &propertyName, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddFilepathProperty(const String &propertyName, const String &extensionFilter = ".*", bool clearDataEnabled = true, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddBoolProperty(const String &propertyName, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddComboProperty(const String &propertyName, const Vector<String> &strings, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddMatrix4Property(const String &propertyName, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddSection(const String &sectionName, bool expanded = true);
    void AddMessageProperty(const String &propertyName, const Message &newMessage);
    void AddColorProperty(const String &propertyName);
    void AddSubsection(const String &subsectionName);
    void AddSliderProperty(const String &propertyName, bool showEdges);
    void AddTexturePreviewProperty(const String &propertyName, editableType propEditType = PROPERTY_IS_EDITABLE);
    void AddDistanceProperty(const String &propertyName, editableType propEditType = PROPERTY_IS_EDITABLE);
    
    void SetStringPropertyValue(const String &propertyName, const String &newText);
    void SetIntPropertyValue(const String &propertyName, int32 newIntValue);
    void SetFloatPropertyValue(const String &propertyName, float32 newFloatValue);
    void SetFilepathPropertyValue(const String &propertyName, const FilePath &currentFilepath);
    void SetBoolPropertyValue(const String &propertyName, bool newBoolValue);
    void SetComboPropertyStrings(const String &propertyName, const Vector<String> &strings);
    void SetComboPropertyIndex(const String &propertyName, int32 currentStringIndex);
    void SetMatrix4PropertyValue(const String &propertyName, const Matrix4 &currentMatrix);
    void SetSectionIsOpened(const String &sectionName, bool isOpened);
    void SetMessagePropertyValue(const String &propertyName, const Message &newMessage);
    void SetColorPropertyValue(const String &propertyName, const Color &newColor);
    void SetSliderPropertyValue(const String &propertyName, float32 newMinValue, float32 newMaxValue, float32 newValue);
    void SetTexturePreviewPropertyValue(const String &propertyName, bool newBoolValue, Texture *newTexture);
    void SetDistancePropertyValue(const String &propertyName, float32 *distances, int32 *triangles, int32 count);
    
    const String &GetStringPropertyValue(const String &propertyName);
    int32 GetIntPropertyValue(const String &propertyName);
    float32 GetFloatPropertyValue(const String &propertyName);
    const FilePath &GetFilepathPropertyValue(const String &propertyName);
    bool GetBoolPropertyValue(const String &propertyName);
    const String &GetComboPropertyValue(const String &propertyName);
    const int32 GetComboPropertyIndex(const String &propertyName);
    const Matrix4 & GetMatrix4PropertyValue(const String &propertyName);
    bool GetSectionIsOpened(const String &sectrionName);
    const Color &GetColorPropertyValue(const String &sectrionName);
    float32 GetSliderPropertyValue(const String &propertyName);
    bool GetTexturePreviewPropertyValue(const String &propertyName);

    int32 GetDistancePropertyCount(const String &propertyName);
    float32 GetDistancePropertyValue(const String &propertyName, int32 index);

    virtual int32 ElementsCount(UIList *forList);
	virtual UIListCell *CellAtIndex(UIList *forList, int32 index);
	virtual int32 CellWidth(UIList *forList, int32 index)//calls only for horizontal orientation
	{return (int32)forList->GetSize().x;};
	virtual int32 CellHeight(UIList *forList, int32 index);//calls only for vertical orientation
	virtual void OnCellSelected(UIList *forList, UIListCell *selectedCell);
    
    virtual void OnPropertyChanged(PropertyCellData *changedProperty);

    void ReleaseProperties();
    
    const List<UIControl*> &GetVisibleCells();

    virtual void SetSize(const Vector2 &newSize);

protected:
    
//    PropertyCellData *GetDataForIndex(int32 index);
    
    void AddProperty(PropertyCellData *newProp, const String &propertyName, editableType propEditType);
    PropertyCellData *PropertyByName(const String &propertyName);
    int32 GetRealIndex(int32 index);
    
    PropertyListDelegate *delegate;
    UIList *propsList;
    Vector<PropertyCellData*> props;
    Map<String, PropertyCellData*> propsMap;
    PropertyCellData *currentSection;
};

#endif