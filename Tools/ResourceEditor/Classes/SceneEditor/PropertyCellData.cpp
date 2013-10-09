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





#include "PropertyCellData.h"

PropertyCellData::PropertyCellData(int _valueType)
{
    cellType = PROP_VALUE_COUNT;
    key = "";
    isEditable = false;
    
    index = 0;
    currentCell = NULL;
    
    intValue = 0;
    floatValue = 0.f;
    stringValue = "";
    boolValue = false;

    
    sliderValueMax = 0.f;
    sliderValueMin = 0.f;
    sliderValue = 0.f;

    texture = NULL;

    distances = NULL;
    triangles = NULL;
    distanceCount = 0;

    
    valueType = _valueType;
}

PropertyCellData::~PropertyCellData()
{
    SafeDeleteArray(distances);
    SafeDeleteArray(triangles);
}

int32 PropertyCellData::GetValueType()
{
    return valueType;
}


void PropertyCellData::SetBool(bool newBool)
{
    DVASSERT(   (valueType == PROP_VALUE_BOOL) 
             || (valueType == PROP_VALUE_TEXTUREPREVIEW)
             || (valueType == PROP_VALUE_SLIDER));
    boolValue = newBool;
}

bool PropertyCellData::GetBool()
{
    DVASSERT(   (valueType == PROP_VALUE_BOOL) 
             || (valueType == PROP_VALUE_TEXTUREPREVIEW)
             || (valueType == PROP_VALUE_SLIDER));
    return boolValue;
}

void PropertyCellData::SetInt(int32 newInt)
{
    DVASSERT((valueType == PROP_VALUE_INTEGER) || (valueType == PROP_VALUE_DISTANCE));
    intValue = newInt;
}

int32 PropertyCellData::GetInt()
{
    DVASSERT((valueType == PROP_VALUE_INTEGER) || (valueType == PROP_VALUE_DISTANCE));
    return intValue;
}

void PropertyCellData::SetFloat(float32 newFloat)
{
    DVASSERT((valueType == PROP_VALUE_FLOAT) || (valueType == PROP_VALUE_DISTANCE));
    floatValue = newFloat;
}

float32 PropertyCellData::GetFloat()
{
    DVASSERT((valueType == PROP_VALUE_FLOAT) || (valueType == PROP_VALUE_DISTANCE));
    return floatValue;
}

void PropertyCellData::SetString(const String& newString)
{
    DVASSERT(valueType == PROP_VALUE_STRING);
    stringValue = newString;
}

const String& PropertyCellData::GetString()
{
    DVASSERT(valueType == PROP_VALUE_STRING);
    return stringValue;
}

void PropertyCellData::SetExtensionFilter(const String& newString)
{
    DVASSERT(valueType == PROP_VALUE_FILEPATH);
    extensionFilter = newString;
}

const String& PropertyCellData::GetExtensionFilter()
{
    DVASSERT(valueType == PROP_VALUE_FILEPATH);
    return extensionFilter;
}

void PropertyCellData::SetStringVector(const Vector<String> &newStrings)
{
    DVASSERT(valueType == PROP_VALUE_COMBO_BOX);
    stringVector = newStrings;
}

const Vector<String> & PropertyCellData::GetStringVector()
{
    DVASSERT(valueType == PROP_VALUE_COMBO_BOX);
    return stringVector;
}

void PropertyCellData::SetItemIndex(int32 newItemIndex)
{
    DVASSERT(valueType == PROP_VALUE_COMBO_BOX);
    intValue = newItemIndex;
}

int32 PropertyCellData::GetItemIndex()
{
    DVASSERT(valueType == PROP_VALUE_COMBO_BOX);
    return intValue;
}

void PropertyCellData::SetMatrix4(const Matrix4 & _matrix)
{
    DVASSERT(valueType == PROP_VALUE_MATRIX4);

    matrix4 = _matrix;
}

const Matrix4 & PropertyCellData::GetMatrix4() const
{
    DVASSERT(valueType == PROP_VALUE_MATRIX4);

    return matrix4;
}

void PropertyCellData::SetIsSectionOpened(bool isSectionOpened)
{
    DVASSERT(valueType == PROP_VALUE_SECTION);
    boolValue = isSectionOpened;
}

bool PropertyCellData::GetIsSectionOpened()
{
    DVASSERT(valueType == PROP_VALUE_SECTION);
    return boolValue;
}

void PropertyCellData::SetSectionElementsCount(int32 sectionElementsCount)
{
    DVASSERT(valueType == PROP_VALUE_SECTION);
    intValue = sectionElementsCount;
}

int32 PropertyCellData::GetSectionElementsCount()
{
    DVASSERT(valueType == PROP_VALUE_SECTION);
    return intValue;
}

const Message & PropertyCellData::GetMessage()
{
    DVASSERT(valueType == PROP_VALUE_MESSAGE);
    return messageValue;
}

void PropertyCellData::SetMessage(const Message &newMessage)
{
    DVASSERT(valueType == PROP_VALUE_MESSAGE);
    messageValue = newMessage;
}

bool PropertyCellData::GetClearDataEnabled()
{
    DVASSERT(valueType == PROP_VALUE_STRING || valueType == PROP_VALUE_FILEPATH);
    return boolValue;
}

void PropertyCellData::SetClearDataEnabled(bool enabled)
{
    DVASSERT(valueType == PROP_VALUE_STRING || valueType == PROP_VALUE_FILEPATH);
    boolValue = enabled;
}

const Color & PropertyCellData::GetColor()
{
    DVASSERT(valueType == PROP_VALUE_COLOR);
    return color;
}

void PropertyCellData::SetColor(const Color& newColor)
{
    DVASSERT(valueType == PROP_VALUE_COLOR);
    color = newColor;
}

float32 PropertyCellData::GetSliderValue()
{
    DVASSERT(valueType == PROP_VALUE_SLIDER);
    return sliderValue;
}

float32 PropertyCellData::GetSliderMinValue()
{
    DVASSERT(valueType == PROP_VALUE_SLIDER);
    return sliderValueMin;
}

float32 PropertyCellData::GetSliderMaxValue()
{
    DVASSERT(valueType == PROP_VALUE_SLIDER);
    return sliderValueMax;
}

void PropertyCellData::SetSliderValue(float32 newMin, float32 newMax, float32 newValue)
{
    DVASSERT(valueType == PROP_VALUE_SLIDER);
    sliderValue = newValue;
    sliderValueMin = newMin;
    sliderValueMax = newMax;
}

void PropertyCellData::SetSliderValue(float32 newValue)
{
    DVASSERT(valueType == PROP_VALUE_SLIDER);
    sliderValue = newValue;
}

void PropertyCellData::SetTexture(DAVA::Texture *newTexture)
{
    DVASSERT(valueType == PROP_VALUE_TEXTUREPREVIEW);
    texture = newTexture;
}

Texture * PropertyCellData::GetTexture()
{
    DVASSERT(valueType == PROP_VALUE_TEXTUREPREVIEW);
    return texture;
}

void PropertyCellData::SetDistances(float32 *newDistances, int32 count)
{
    DVASSERT(valueType == PROP_VALUE_DISTANCE);

    SafeDeleteArray(distances);
    distances = new float32[count];
    Memcpy(distances, newDistances, count * sizeof(float32));

    distanceCount = count;
}

float32 *PropertyCellData::GetDistances()
{
    DVASSERT(valueType == PROP_VALUE_DISTANCE);
    return distances;
}

void PropertyCellData::SetTriangles(int32 *newTriangles, int32 count)
{
    DVASSERT(valueType == PROP_VALUE_DISTANCE);
    
    SafeDeleteArray(triangles);
    triangles = new int32[count];
    Memcpy(triangles, newTriangles, count * sizeof(float32));
    
    distanceCount = count;
}

int32 *PropertyCellData::GetTriangles()
{
    DVASSERT(valueType == PROP_VALUE_DISTANCE);
    return triangles;
}


int32 PropertyCellData::GetDistancesCount()
{
    DVASSERT(valueType == PROP_VALUE_DISTANCE);
    return distanceCount;
}

void PropertyCellData::SetDistance(float32 newDistance, int32 index)
{
    DVASSERT(valueType == PROP_VALUE_DISTANCE);
    DVASSERT((0 <= index) && (index < distanceCount));
    
    SetFloat(newDistance);
    SetInt(index);
    
    distances[index] = newDistance;
}

void PropertyCellData::SetFilePath(const DAVA::FilePath &pathname)
{
    DVASSERT(valueType == PROP_VALUE_FILEPATH);
    filePath = pathname;
}

const FilePath & PropertyCellData::GetFilePath() const
{
    DVASSERT(valueType == PROP_VALUE_FILEPATH);
    return filePath;
}

