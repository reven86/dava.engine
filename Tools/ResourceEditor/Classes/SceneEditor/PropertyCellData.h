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




#ifndef PROPERTY_VALUE
#define PROPERTY_VALUE

#include "DAVAEngine.h"

using namespace DAVA;
class PropertyCell;
class PropertyCellData : public BaseObject
{
public:
    enum ValueType 
    {
        PROP_VALUE_STRING = 0,
        PROP_VALUE_INTEGER,
        PROP_VALUE_FLOAT,
        PROP_VALUE_BOOL,
        PROP_VALUE_MATRIX4,
        PROP_VALUE_COMBO_BOX,
        PROP_VALUE_SECTION,
        PROP_VALUE_MESSAGE,
        PROP_VALUE_COLOR,
        PROP_VALUE_SLIDER,
        PROP_VALUE_SUBSECTION,
        PROP_VALUE_TEXTUREPREVIEW,
        PROP_VALUE_DISTANCE,
        PROP_VALUE_FILEPATH,
        
        PROP_VALUE_COUNT
    };
protected:
    virtual ~PropertyCellData();
public:
    PropertyCellData(int _valueType);

    int32 GetValueType();

    
    int32 GetInt();
    float32 GetFloat();
    const String& GetString();
    bool GetBool();
    const String& GetExtensionFilter();
    const Vector<String> & GetStringVector();
    int32 GetItemIndex(); 
    const Matrix4 & GetMatrix4() const;
    bool GetIsSectionOpened(); 
    int32 GetSectionElementsCount();
    const Message & GetMessage();
    bool GetClearDataEnabled();
    const Color &GetColor();
    float32 GetSliderValue();
    float32 GetSliderMinValue();
    float32 GetSliderMaxValue();
    float32 *GetDistances();
    int32 *GetTriangles();
    int32 GetDistancesCount();
    
    const FilePath & GetFilePath() const;
    
    Texture *GetTexture();

    void SetInt(int32 newInt);
    void SetFloat(float32 newFloat);
    void SetString(const String& newString);
    void SetBool(bool newBool);
    void SetExtensionFilter(const String& newString);
    void SetStringVector(const Vector<String> &newStrings);
    void SetItemIndex(int32 newItemIndex);
    void SetMatrix4(const Matrix4 & _matrix);
    void SetIsSectionOpened(bool isSectionOpened);
    void SetSectionElementsCount(int32 sectionElementsCount);
    void SetMessage(const Message &newMessage);
    void SetClearDataEnabled(bool enabled);
    void SetColor(const Color& newColor);
    void SetSliderValue(float32 newMin, float32 newMax, float32 newValue);
    void SetSliderValue(float32 newValue);
    void SetTexture(Texture *newTexture);
    void SetDistances(float32 *newDistances, int32 count);
    void SetTriangles(int32 *newTriangles, int32 count);
    void SetDistance(float32 newDistance, int32 index);
    void SetFilePath(const FilePath &pathname);
    
    int32 cellType;
    String key;
    bool isEditable;
    int index;
    
    PropertyCell *currentCell;
protected:
    int32 valueType;
    
    int32 intValue;
    float32 floatValue;
    String stringValue;
    bool boolValue;

    String extensionFilter;
    
    Vector<String> stringVector;

    Matrix4 matrix4;
    
    Message messageValue;
    
    Color color;
    
    float32 sliderValueMax;
    float32 sliderValueMin;
    float32 sliderValue;
    
    Texture *texture;
    
    float32 *distances;
    int32 *triangles;
    int32 distanceCount;
    
    FilePath filePath;
};

#endif