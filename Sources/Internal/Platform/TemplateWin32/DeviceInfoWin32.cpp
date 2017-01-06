#include "Base/Platform.h"

#if defined(__DAVAENGINE_WIN32__)

#include "Platform/DeviceInfo.h"
#include "Utils/StringFormat.h"
#include "Utils/MD5.h"
#include "Utils/UTF8Utils.h"
#include "Debug/DVAssert.h"
#include "Platform/TemplateWin32/DeviceInfoWin32.h"
#include "Platform/DPIHelper.h"
#include "Base/GlobalEnum.h"
#include "winsock2.h"
#include "Iphlpapi.h"
#include "TimeZones.h"

#include <VersionHelpers.h>

namespace DAVA
{
namespace RegistryReader
{
const WideString oemRegistryPath(L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\OEMInformation");
const WideString infoRegistryPath(L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion");
const WideString manufacturer(L"Manufacturer");
const WideString model(L"Model");
const WideString currentOSVersion(L"CurrentVersion");
const WideString currentBuildNumber(L"CurrentBuildNumber");
const WideString currentBuild(L"CurrentBuild");
const WideString operationSystemName(L"ProductName");

String GetStringForKey(const WideString& path, const WideString& key)
{
    WideString val;

    HKEY hKey;
    LONG openRes = RegOpenKeyExW(HKEY_LOCAL_MACHINE, path.c_str(), 0, KEY_READ, &hKey);
    if (ERROR_SUCCESS == openRes)
    {
        WCHAR szBuffer[512];
        DWORD dwBufferSize = sizeof(szBuffer);
        ULONG error;
        error = RegQueryValueExW(hKey, key.c_str(), 0, NULL, reinterpret_cast<LPBYTE>(szBuffer), &dwBufferSize);
        if (ERROR_SUCCESS == error)
        {
            val = szBuffer;
        }
    }

    String ret = UTF8Utils::EncodeToUTF8(val);
    return ret;
}
}

DeviceInfoPrivate::DeviceInfoPrivate()
{
}

DeviceInfo::ePlatform DeviceInfoPrivate::GetPlatform()
{
    return DeviceInfo::PLATFORM_WIN32;
}

String DeviceInfoPrivate::GetPlatformString()
{
    return GlobalEnumMap<DeviceInfo::ePlatform>::Instance()->ToString(GetPlatform());
}

String DeviceInfoPrivate::GetVersion()
{
    String currentOSVersion = RegistryReader::GetStringForKey(RegistryReader::infoRegistryPath, RegistryReader::currentOSVersion);
    String currentBuildNumber = RegistryReader::GetStringForKey(RegistryReader::infoRegistryPath, RegistryReader::currentBuildNumber);
    if ("" == currentBuildNumber)
    {
        currentBuildNumber = RegistryReader::GetStringForKey(RegistryReader::infoRegistryPath, RegistryReader::currentBuild);
    }
    String operationSystemName = RegistryReader::GetStringForKey(RegistryReader::infoRegistryPath, RegistryReader::operationSystemName);

    String version = currentOSVersion + "." + currentBuildNumber;

    return version;
}

String DeviceInfoPrivate::GetManufacturer()
{
    String manufacturer = RegistryReader::GetStringForKey(RegistryReader::oemRegistryPath, RegistryReader::manufacturer);
    return manufacturer;
}

String DeviceInfoPrivate::GetModel()
{
    String model = RegistryReader::GetStringForKey(RegistryReader::oemRegistryPath, RegistryReader::model);
    return model;
}

String DeviceInfoPrivate::GetLocale()
{
    WCHAR localeBuffer[LOCALE_NAME_MAX_LENGTH];
    int size = GetUserDefaultLocaleName(localeBuffer, LOCALE_NAME_MAX_LENGTH);
    String locale;
    if (0 != size)
    {
        locale = UTF8Utils::EncodeToUTF8(localeBuffer);
    }
    return locale;
}

String DeviceInfoPrivate::GetRegion()
{
    GEOID myGEO = GetUserGeoID(GEOCLASS_NATION);
    int sizeOfBuffer = GetGeoInfo(myGEO, GEO_ISO2, NULL, 0, 0);
    if (0 == sizeOfBuffer)
    {
        return "";
    }

    WCHAR* buffer = new WCHAR[sizeOfBuffer];
    int result = GetGeoInfo(myGEO, GEO_ISO2, buffer, sizeOfBuffer, 0);
    DVASSERT(0 != result);
    String country = UTF8Utils::EncodeToUTF8(buffer);
    delete[] buffer;

    return country;
}

String DeviceInfoPrivate::GetTimeZone()
{
    /*don't remove that code please. it is needed for the nex task*/
    DYNAMIC_TIME_ZONE_INFORMATION timeZoneInformation;
    DWORD ret = GetDynamicTimeZoneInformation(&timeZoneInformation);

    String generalName = TimeZoneHelper::GetGeneralNameByStdName(timeZoneInformation.TimeZoneKeyName);
    DVASSERT(!generalName.empty(), Format("No &s timezone found! Check time zones map", generalName.c_str()).c_str());

    return generalName;
}
String DeviceInfoPrivate::GetHTTPProxyHost()
{
    return "";
}

String DeviceInfoPrivate::GetHTTPNonProxyHosts()
{
    return "";
}

int32 DeviceInfoPrivate::GetHTTPProxyPort()
{
    return 0;
}

#if !defined(__DAVAENGINE_COREV2__)
DeviceInfo::ScreenInfo& DeviceInfoPrivate::GetScreenInfo()
{
    // default win32 dpi is 96 = 100% scaling
    // we should get current and update screen scale
    // in case if application is DPI-Aware
    // see https://msdn.microsoft.com/en-us/library/windows/desktop/dn469266(v=vs.85).aspx
    float32 defaultDPI = 96.0f;
    float32 currDPI = static_cast<float32>(DPIHelper::GetScreenDPI());

    screenInfo.scale = currDPI / defaultDPI;
    return screenInfo;
}

void DeviceInfoPrivate::InitializeScreenInfo()
{
    screenInfo.width = ::GetSystemMetrics(SM_CXSCREEN);
    screenInfo.height = ::GetSystemMetrics(SM_CYSCREEN);

    screenInfo.scale = 1;
}
#endif

int32 DeviceInfoPrivate::GetZBufferSize()
{
    return 24;
}

List<DeviceInfo::StorageInfo> DeviceInfoPrivate::GetStoragesList()
{
    List<DeviceInfo::StorageInfo> l;
    return l;
}

String DeviceInfoPrivate::GetUDID()
{
    ULONG family = AF_INET;
    ULONG flags = GAA_FLAG_SKIP_ANYCAST | GAA_FLAG_SKIP_DNS_SERVER | GAA_FLAG_SKIP_FRIENDLY_NAME | GAA_FLAG_SKIP_MULTICAST | GAA_FLAG_SKIP_UNICAST;

    PIP_ADAPTER_ADDRESSES buf;
    ULONG bufLength = 15000;

    DWORD retVal;

    do
    {
        buf = (PIP_ADAPTER_ADDRESSES)malloc(bufLength);
        retVal = GetAdaptersAddresses(family, flags, 0, buf, &bufLength);

        if (retVal == ERROR_BUFFER_OVERFLOW)
        {
            free(buf);
            bufLength *= 2;
        }
    } while (retVal == ERROR_BUFFER_OVERFLOW);

    String res = "";
    if (retVal == NO_ERROR)
    {
        PIP_ADAPTER_ADDRESSES curAddress = buf;

        if (curAddress)
        {
            if (curAddress->PhysicalAddressLength)
            {
                for (int32 i = 0; i < (int32)curAddress->PhysicalAddressLength; ++i)
                {
                    res += String(Format("%.2x", curAddress->PhysicalAddress[i]));
                }
            }
        }
    }

    if (buf)
    {
        free(buf);
    }

    if (res == "")
    {
        bool idOk = false;
        DWORD bufSize = 1024;
        LPBYTE buf = new BYTE[bufSize];
        HKEY key;
        if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ, &key) == ERROR_SUCCESS)
        {
            if (RegQueryValueEx(key, L"MachineGuid", 0, 0, buf, &bufSize) == ERROR_SUCCESS)
            {
                idOk = true;
            }
            else
            {
                if (RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Cryptography", 0, KEY_READ | KEY_WOW64_64KEY, &key) == ERROR_SUCCESS)
                {
                    if (RegQueryValueEx(key, L"MachineGuid", 0, 0, buf, &bufSize) == ERROR_SUCCESS)
                    {
                        idOk = true;
                    }
                }
            }
        }

        if (idOk)
        {
            WideString wstr = WideString((wchar_t*)buf);
            res = UTF8Utils::EncodeToUTF8(wstr);
        }
        else
        {
            DVASSERT(false && "Invalid UDID");
            res = "Invalid UDID";
        }
        SafeDeleteArray(buf);
    }

    MD5::MD5Digest md5Digest;
    MD5::ForData(reinterpret_cast<const uint8*>(res.c_str()), static_cast<uint32>(res.size()), md5Digest);

    String digest(MD5::MD5Digest::DIGEST_SIZE * 2 + 1, '\0');
    MD5::HashToChar(md5Digest, const_cast<char8*>(digest.data()), static_cast<uint32>(digest.size()));
    return digest;
}

WideString DeviceInfoPrivate::GetName()
{
    //http://msdn.microsoft.com/en-us/library/windows/desktop/ms724295(v=vs.85).aspx
    char16 compName[MAX_COMPUTERNAME_LENGTH + 1];
    uint32 length = MAX_COMPUTERNAME_LENGTH + 1;

    bool nameRecieved = GetComputerNameW(compName, (LPDWORD)&length) != FALSE;
    if (nameRecieved)
    {
        return WideString(compName, length);
    }

    return WideString();
}

eGPUFamily DeviceInfoPrivate::GetGPUFamilyImpl()
{
    return GPU_DX11;
}

DeviceInfo::NetworkInfo DeviceInfoPrivate::GetNetworkInfo()
{
    // For now return default network info for Windows.
    return DeviceInfo::NetworkInfo();
}

bool DeviceInfoPrivate::IsHIDConnected(DeviceInfo::eHIDType type)
{
    //TODO: remove this empty realization and implement detection of HID connection
    if (type == DeviceInfo::HID_MOUSE_TYPE || type == DeviceInfo::HID_KEYBOARD_TYPE)
    {
        return true;
    }
    return false;
}

bool DeviceInfoPrivate::IsTouchPresented()
{
    //TODO: remove this empty realization and implement detection touch
    return false;
}

String DeviceInfoPrivate::GetCarrierName()
{
    return "Not supported";
}
}

#endif // defined(__DAVAENGINE_WIN32__)
