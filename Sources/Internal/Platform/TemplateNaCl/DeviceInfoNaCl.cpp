#include "Platform/DeviceInfo.h"

#ifdef __DAVAENGINE_NACL__


namespace DAVA
{

String DeviceInfo::GetVersion()
{
	return "Not yet implemented";
}

String DeviceInfo::GetManufacturer()
{
	return "Not yet implemented";
}

String DeviceInfo::GetModel()
{
	return "Not yet implemented";
}

String DeviceInfo::GetLocale()
{
	return "Not yet implemented";
}

String DeviceInfo::GetRegion()
{
	return "Not yet implemented";
}

String DeviceInfo::GetTimeZone()
{
	return "Not yet implemented";
}

String DeviceInfo::GetUDID()
{
	return "Not yet implemented";
}

eGPUFamily DeviceInfo::GetGPUFamily()
{
	return GPU_PNG;
}

DeviceInfo::NetworkInfo DeviceInfo::GetNetworkInfo()
{
	return DeviceInfo::NetworkInfo();
}

}

#endif