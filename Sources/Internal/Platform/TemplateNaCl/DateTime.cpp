#ifdef __DAVAENGINE_NACL__

#include "../../Platform/DateTime.h"
#include "../../Utils/Utils.h"
#include "../../FileSystem/LocalizationSystem.h"

namespace DAVA
{

WideString DateTime::AsWString(const wchar_t* format) const
{
	return L"";
}

int32 DateTime::GetLocalTimeZoneOffset()
{
	
	return 0;
}

};

#endif