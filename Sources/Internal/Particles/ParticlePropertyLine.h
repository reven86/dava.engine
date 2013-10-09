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


#ifndef __DAVAENGINE_PARTICLES_PROPERTY_LINE_H__
#define __DAVAENGINE_PARTICLES_PROPERTY_LINE_H__

#include "FileSystem/YamlParser.h"
#include "Base/RefPtr.h"
#include <limits>

namespace DAVA
{

template<class T>
class PropertyLine : public BaseObject
{
public:

	struct PropertyKey
	{
		float32 t;
		T value;
	};

	Vector<PropertyKey> keys;
	Vector<PropertyKey> & GetValues() { return keys; };


	virtual const T & GetValue(float32 t) = 0;

	virtual PropertyLine<T>* Clone() {return 0;};
};

class PropertyLineYamlReader
{
public:
	static RefPtr<PropertyLine<float32> > CreateFloatPropertyLineFromYamlNode(const YamlNode * parentNode, const String & propertyName, RefPtr<PropertyLine<float32> > defaultPropertyLine = RefPtr< PropertyLine<float32> >());
	static RefPtr<PropertyLine<Vector2> > CreateVector2PropertyLineFromYamlNode(const YamlNode * parentNode, const String & propertyName, RefPtr<PropertyLine<Vector2> > defaultPropertyLine = RefPtr< PropertyLine<Vector2> >());
    static RefPtr<PropertyLine<Vector3> > CreateVector3PropertyLineFromYamlNode(const YamlNode * parentNode, const String & propertyName, RefPtr<PropertyLine<Vector3> > defaultPropertyLine = RefPtr< PropertyLine<Vector3> >());
	static RefPtr<PropertyLine<Color> >  CreateColorPropertyLineFromYamlNode(const YamlNode * parentNode, const String & propertyName, RefPtr<PropertyLine<Color> > defaultPropertyLine = RefPtr< PropertyLine<Color> >());
};

class PropertyLineYamlWriter
{
public:
    // Write the single value and whole property line to the YAML node.
    template<class T> static void WritePropertyValueToYamlNode(YamlNode* parentNode, const String& propertyName, T propertyValue);
    template<class T> static YamlNode* WritePropertyLineToYamlNode(YamlNode* parentNode, const String& propertyName, RefPtr<PropertyLine<T> > propertyLine);
    
    // Specific implementation for writing Color values.
    static YamlNode* WriteColorPropertyLineToYamlNode(YamlNode* parentNode, const String& propertyName, RefPtr<PropertyLine<Color> > propertyLine);
    
protected:
    // Convert Color to Vector4 value.
    static Vector4 ColorToVector(const Color& color);
};

/*#define PROPERTY_RETAIN(type, name)\
private:\
type property##name;\
public:\
void Set##name(type _v)\
{\
	SafeRelease(property##name);\
	property##name = SafeRetain(_v);\
}\
type Get##name(){return property##name;};
*/


template<class T>
class PropertyLineValue : public PropertyLine<T>
{
public:

	PropertyLineValue(T _value)
	{
		typename PropertyLine<T>::PropertyKey v;
		v.t = 0;
		v.value = _value;
		PropertyLine<T>::keys.push_back(v);
	}	

	const T & GetValue(float32 /*t*/) { return PropertyLine<T>::keys[0].value; }

	PropertyLine<T>* Clone()
	{	
		if (this == 0)return 0;
		return new PropertyLineValue<T>(PropertyLine<T>::keys[0].value); 
	}	
};

template<class T>
class PropertyLineKeyframes : public PropertyLine<T>
{
public:
	T resultValue;
	
	const T & GetValue(float32 t) 
	{
		int32 keysSize = (int32)PropertyLine<T>::keys.size();
		if(t > PropertyLine<T>::keys[keysSize - 1].t)
		{
			return PropertyLine<T>::keys[keysSize - 1].value;
		}
		if (PropertyLine<T>::keys.size() == 2)
		{
			if (t < PropertyLine<T>::keys[1].t)
			{
				float ti = (t - PropertyLine<T>::keys[0].t) / (PropertyLine<T>::keys[1].t - PropertyLine<T>::keys[0].t);
				resultValue = PropertyLine<T>::keys[0].value + (PropertyLine<T>::keys[1].value - PropertyLine<T>::keys[0].value) * ti;
				return resultValue;
			}
            else 
			{
                return PropertyLine<T>::keys[1].value;
			}
		}
		else if (PropertyLine<T>::keys.size() == 1) return PropertyLine<T>::keys[0].value; 
		else
		{
			int32 l = BinaryFind(t, 0, (int32)PropertyLine<T>::keys.size() - 1);

			float ti = (t - PropertyLine<T>::keys[l].t) / (PropertyLine<T>::keys[l + 1].t - PropertyLine<T>::keys[l].t);
			resultValue = PropertyLine<T>::keys[l].value + (PropertyLine<T>::keys[l + 1].value - PropertyLine<T>::keys[l].value) * ti;
		}
		return resultValue;
	}

	int32 BinaryFind(float32 t, int32 l, int32 r)
	{
		if (l + 1 == r)	// we've found a solution
		{
			return l;
		}

		int32 m = (l + r) >> 1; //l + (r - l) / 2 = l + r / 2 - l / 2 = (l + r) / 2; 
		if (t <= PropertyLine<T>::keys[m].t)return BinaryFind(t, l, m);
		else return BinaryFind(t, m, r);
	}

	void AddValue(float32 t, T value)
	{
		typename PropertyLine<T>::PropertyKey key;
		key.t = t;
		key.value = value;
		PropertyLine<T>::keys.push_back(key);
	}
	
	PropertyLine<T>* Clone()
	{ 
		if (this == 0)return 0;
		PropertyLineKeyframes<T>* clone =  new PropertyLineKeyframes<T>();
		clone->keys = PropertyLine<T>::keys;
		return clone;
	}	
};

template <class T> class PropValue
{
public:
    float32 t;
    T v;
    PropValue(float32 t, const T& v)
    {
        this->t = t;
        this->v = v;
    };
};

// A wrapper for Property Line, which allows easy access to the values.
template <class T> class PropLineWrapper
{
public:
    PropLineWrapper(RefPtr< PropertyLine<T> > propertyLine)
    {
        Init(propertyLine);
    }
    PropLineWrapper()
    {
    }

    virtual ~PropLineWrapper()
    {
};

    void Init(RefPtr< PropertyLine<T> > propertyLine);
    
    Vector< PropValue<T> >* GetPropsPtr();
	Vector< PropValue<T> > GetProps() const;
    RefPtr< PropertyLine<T> > GetPropLine() const;

private:
    Vector< PropValue<T> > values;
    float32 minT;
    float32 maxT;
};

template <class T>
void PropLineWrapper<T>::Init(RefPtr< PropertyLine<T> > propertyLine)
{
    values.clear();
    minT = std::numeric_limits<float32>::infinity();
    maxT = -std::numeric_limits<float32>::infinity();
    
    PropertyLineValue<T> *pv;
    PropertyLineKeyframes<T> *pk;

    pk = dynamic_cast< PropertyLineKeyframes<T> *>(propertyLine.Get());
    pv = dynamic_cast< PropertyLineValue<T> *>(propertyLine.Get());

    if (pk)
    {
        for (uint32 i = 0; i < pk->keys.size(); ++i)
        {
            float32 t = pk->keys[i].t;
            minT = Min(minT, t);
            maxT = Max(maxT, t);

            values.push_back(PropValue<T>(t, pk->keys[i].value));
        }
    }
    else if (pv)
    {
        values.push_back(PropValue<T>(0, pv->GetValue(0)));
    }
}

template <class T>
Vector< PropValue<T> >* PropLineWrapper<T>::GetPropsPtr()
{
    return &values;
}

template <class T>
Vector< PropValue<T> > PropLineWrapper<T>::GetProps() const
{
	return values;
}
		
template <class T>
RefPtr< PropertyLine<T> > PropLineWrapper<T>::GetPropLine() const
{
    if (values.size() > 1)
    {
        //return PropertyLineKeyframes
        RefPtr< PropertyLineKeyframes<T> > lineKeyFrames(new PropertyLineKeyframes<T>());
        for (uint32 i = 0; i < values.size(); ++i)
        {
            lineKeyFrames->AddValue(values[i].t, values[i].v);
        }
        return lineKeyFrames;
    }
    else if (values.size() == 1)
    {
        //return PropertyLineValue
        RefPtr< PropertyLineValue<T> > lineValue(new PropertyLineValue<T>(values[0].v));
        return lineValue;
    }
    return RefPtr< PropertyLine<T> >();
}

// Writer logic.
template<class T>
void PropertyLineYamlWriter::WritePropertyValueToYamlNode(YamlNode* parentNode, const String& propertyName,
                                                          T propertyValue)
{
    parentNode->Set(propertyName, propertyValue);
}

template<class T>
YamlNode* PropertyLineYamlWriter::WritePropertyLineToYamlNode(YamlNode* parentNode, const String& propertyName,
                                                              RefPtr<PropertyLine<T> > propertyLine)
{
    // Write the property line.
    Vector<PropValue<T> > wrappedPropertyValues = PropLineWrapper<T>(propertyLine).GetProps();
    if (wrappedPropertyValues.empty())
    {
        return NULL;
    }

    if (wrappedPropertyValues.size() == 1)
    {
        // This has to be single string value.
        parentNode->Set(propertyName, wrappedPropertyValues.at(0).v);
        return NULL;
    }
    
    // Create the child array node.
    YamlNode* childNode = new YamlNode(YamlNode::TYPE_ARRAY);
    for (typename Vector<PropValue<T> >::iterator iter = wrappedPropertyValues.begin();
         iter != wrappedPropertyValues.end(); iter ++)
    {
        childNode->AddValueToArray((*iter).t);
        childNode->AddValueToArray((*iter).v);
    }
    
    parentNode->AddNodeToMap(propertyName, childNode);
    return childNode;
}

};

#endif // __DAVAENGINE_PARTICLES_PROPERTY_LINE_H__