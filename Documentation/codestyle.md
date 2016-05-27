CPP Codestyle
========

##Code formatting
####Example
The following example shows properly formatted code sample

```cpp
#pragma once

#include "Base/BaseObject.h"

namespace DAVA
{

class MyClass : public BaseObject
{
public:
    MyClass();

    void Foo(int32 i);
};

inline void MyClass::Foo(int32 i)
{
    std::cout << i << std::endl;
}

}
```

##General

####


####Intendation
Tabs are newer used, we use 4 spaces instead.

####Namespaces
Ontendation is not used inside namespaces
#####namespace DAVA
All dava.framework code is written inside `namespace DAVA`.

```cpp
namespace DAVA
{
class Foo
{
    void Bar();
};
}
```

####Header guard
Header files are guarder by `#pragma once`

####Commented code
Remove commented code, we have revision control system for history. 

##Naming
Names are written in english, with no prefixes/suffixes and no odd shortenings. Name length and descriptiveness should be proportional to name scope: more descriptive names are used in bigger scopes.

```cpp
Image * CreateImageFromMemory(...); //long descriptive function name in public interface
```

```cpp
{
    void * imageData = image->Data(); //imageData is local variable name
    //plenty of code here
}
```

```cpp
for(int32 i = 0; i < width; ++i) //i is only used inside small loop
{
    Read(i);
}
```

####Classes, functions
Camel notation begin with capital letter.

```cpp
class MyObject;
void Foo();
```

####Variables
Camel notation begin with lower-case.

```cpp
MyObject myObjectInstance;
```
In case of names collision add underscore to local variable name.

```cpp
MyObject::MyObject(const String& name_)
:   name(name_)
{...}
```

####Function objects
Function object is both function and variable. We just use the same naming rules as for variables: camel notation begin with lower-case.

```cpp
auto foo()[]{}
```

####Static constants, defines, enums.
All capitals with underscores.

```cpp
static const int MAX_WIDTH = 100;
enum eFrustumPlane 
{
    EFP_LEFT = 0,
    EFP_RIGHT
};
#define HAS_SPECULAR
```

##Types
Use types from `Base\BaseTypes.h` instead of built-in.
This include

* base types (int32, float32, etc.)
* standart library types (String, Vector, etc.)

##Classes
####Class members initialization
In-class member initialization is preferred over all other methods.

```cpp
class MyObject
{
    int32 width = 0;
    int32 height = 0;
    Color color = Color::White();
};
```

####Virtual functions
Either `virtual`, `override` or `final` keyword can be used in function declaration. **Do not** mix `virtual` with `override/final` keywords in one declaration.

```cpp
virtual void Foo(); //declared for the first time
void Bar() override; //overriding virtual function
```

####Inline functions
Separate inline function definition from declaration and use `inline` keyword only in declaration. Declaration should be placed in the end of header file.

```cpp
class MyObject
{
    void Foo();
};
//...
inline void MyObject::Foo()
{
}
```
##CPP 11+ features
####Autos
Usage of auto is limited to
#####lambda function type

```cpp
auto foo = [&](bool arg1, void* arg2)
{
    arg1 = *arg2;
}
```
and
#####shortening of long template types

```cpp
Map<FastName, SmartPointer<ObjectType>> map;
auto iter = map.begin();
```
