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



#ifndef __DAVAENGINE_ASSERT_ACTION_H__
#define __DAVAENGINE_ASSERT_ACTION_H__

#include "DAVAConfig.h"

#ifdef __DAVAENGINE_AUTOTESTING__

#include "Autotesting/Action.h"

#include "FileSystem/VariantType.h"

namespace DAVA
{

class Getter : public Action
{
public:
    Getter();
    Getter(const VariantType &_value);
    virtual ~Getter() {};

    virtual const VariantType &Get() { return value; };
protected:
	virtual String Dump();

    VariantType value;
};

class ControlGetter : public Getter
{
public:
    ControlGetter(const Vector<String> &_controlPath);
    virtual ~ControlGetter() {};

   virtual const VariantType &Get();

protected:
	virtual String Dump();
    
	Vector<String> controlPath;
};

class ControlTextGetter : public ControlGetter
{
public:
    ControlTextGetter(const Vector<String> &_controlPath);
    virtual ~ControlTextGetter() {};

    virtual void Execute();
};

class ControlBoolGetter : public ControlGetter
{
public:
    ControlBoolGetter(const Vector<String> &_controlPath);
    virtual ~ControlBoolGetter() {};

    virtual void Execute();
};

class AssertAction : public Action
{
public:
    AssertAction(const String &_message = "");
    virtual ~AssertAction();

    void SetExpectedGetter(Getter *_expected);
    void SetActualGetter(Getter *_actual);

    virtual void Execute();
protected:
	virtual String Dump();

    String message;
    Getter *expected;
    Getter *actual;
};

// template <class T> class Getter : public Action
// {
// public:
//     Getter() {};
//     Getter(const T &_value) : Action(), value(_value) {};
//     virtual ~Getter() {};
// 
//     virtual const T &Get() { return value; };
// protected:
//     T value;
// };
// 
// template <class T> class ControlGetter : public Getter<T>
// {
// public:
//     ControlGetter(const Vector<String> &_controlPath) : Getter(), controlPath(_controlPath) {};
//     virtual ~ControlGetter() {};
// 
//    virtual const T &Get();
// 
// protected:
//     Vector<String> controlPath;
// };

// class ControlTextGetter : public ControlGetter<WideString>
// {
// public:
//     ControlTextGetter(const Vector<String> &_controlPath) : ControlGetter(_controlPath) {};
//     virtual ~ControlTextGetter() {};
// 
//     virtual void Execute();
// };
// 
// class ControlBoolGetter : public ControlGetter<bool>
// {
// public:
//     ControlBoolGetter(const Vector<String> &_controlPath) : ControlGetter(_controlPath) {};
//     virtual ~ControlBoolGetter() {};
// 
//     virtual void Execute();
// };


// template <class T> class AssertAction : public Action
// {
// public:
//     AssertAction();
//     virtual ~AssertAction();
// 
//     virtual void Execute();
// protected:
//     Getter<T> *expected;
//     Getter<T> *actual;
// };
// 
// class AssertTextAction : public AssertAction<WideString>
// {
// public:
//     // compare value with result of getter
//     AssertTextAction(const WideString &_expected, const Vector<String> &_actualControlPath); 
//     // compare results of getters
//     AssertTextAction(const Vector<String> &_expectedControlPath, const Vector<String> &_actualControlPath); 
// 
//     virtual ~AssertTextAction();
// };
// 
// class AssertBoolAction : public AssertAction<bool>
// {
// public:
//     // compare value with result of getter
//     AssertBoolAction(bool _expected, const Vector<String> &_actualControlPath); 
//     // compare results of getters
//     AssertBoolAction(const Vector<String> &_expectedControlPath, const Vector<String> &_actualControlPath); 
// 
//     virtual ~AssertBoolAction();
// };

};
#endif //__DAVAENGINE_AUTOTESTING__
#endif //__DAVAENGINE_ASSERT_ACTION_H__