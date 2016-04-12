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


#ifndef PREFERENCES_REGISTRATOR
#define PREFERENCES_REGISTRATOR

namespace DAVA
{
class InspBase;
}

class PreferencesRegistrator
{
public:
    using DefaultValuesList = DAVA::Map<DAVA::String, DAVA::VariantType>;
    PreferencesRegistrator(const DAVA::InspInfo* inspInfo, const DefaultValuesList& defaultValues = DefaultValuesList());
    PreferencesRegistrator(void* objPtr, DAVA::InspBase* inspBase);
    ~PreferencesRegistrator();

private:
    void* objectPtr = nullptr;
    DAVA::InspBase* trackedIntrospection = nullptr;
};

#define REGISTER_PREFERENCES \
    PreferencesRegistrator preferencesRegistrator = PreferencesRegistrator(static_cast<void*>(this), static_cast<DAVA::InspBase*>(this));

#define PREFERENCES_DEFAULT(name, value) \
    {DAVA::String(name), VariantType(value)}

#define REGISTER_PREFERENCES_ON_START(Classname, defaultValues) \
    namespace Classname##_local {\
        PreferencesRegistrator preferencesRegistrator(Classname::TypeInfo(), PreferencesRegistrator::DefaultValuesList({ defaultValues })); \
    }   

#define REGISTER_PREFERENCES_ON_START2(Classname) \
    namespace Classname##_local {\
        PreferencesRegistrator preferencesRegistrator(Classname::TypeInfo()); \
    }

template <typename T, typename UI>
struct UiInitializer
{
    UiInitializer(T* t, UI* uiForm)
    {
        uiForm->setupUi(t);
    }
};
#define INIT_QT_UI(Class, ui) \
    friend class UiInitializer; \
    UiInitializer init(Class, ui);


#endif //PREFERENCES_REGISTRATOR
