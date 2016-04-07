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


#ifndef __FRAMEWORK__TIMEZONES_H__
#define __FRAMEWORK__TIMEZONES_H__

#include "Base/FastName.h"

namespace DAVA
{
namespace TimeZoneHelper
{
// Timezone names map!
// taken here http://unicode.org/repos/cldr/trunk/common/supplemental/windowsZones.xml
// Regexes for parse XML:
// First: replace '^((?!(001|//)).)*\n?' to ''
// Second: replace '.*other="([\s\w.()\-+]*)".*type="([\s\w/+\-_//]*)".*' to '{ FastName("\1"), FastName("\2") },'
Map<FastName, FastName> namesMap =
{
  { FastName("Dateline Standard Time"), FastName("Etc/GMT+12") },
  { FastName("UTC-11"), FastName("Etc/GMT+11") },
  { FastName("Hawaiian Standard Time"), FastName("Pacific/Honolulu") },
  { FastName("Alaskan Standard Time"), FastName("America/Anchorage") },
  { FastName("Pacific Standard Time"), FastName("America/Los_Angeles") },
  { FastName("US Mountain Standard Time"), FastName("America/Phoenix") },
  { FastName("Mountain Standard Time (Mexico)"), FastName("America/Chihuahua") },
  { FastName("Mountain Standard Time"), FastName("America/Denver") },
  { FastName("Central America Standard Time"), FastName("America/Guatemala") },
  { FastName("Central Standard Time"), FastName("America/Chicago") },
  { FastName("Central Standard Time (Mexico)"), FastName("America/Mexico_City") },
  { FastName("Canada Central Standard Time"), FastName("America/Regina") },
  { FastName("SA Pacific Standard Time"), FastName("America/Bogota") },
  { FastName("Eastern Standard Time (Mexico)"), FastName("America/Cancun") },
  { FastName("Eastern Standard Time"), FastName("America/New_York") },
  { FastName("US Eastern Standard Time"), FastName("America/Indianapolis") },
  { FastName("Venezuela Standard Time"), FastName("America/Caracas") },
  { FastName("Paraguay Standard Time"), FastName("America/Asuncion") },
  { FastName("Atlantic Standard Time"), FastName("America/Halifax") },
  { FastName("Central Brazilian Standard Time"), FastName("America/Cuiaba") },
  { FastName("SA Western Standard Time"), FastName("America/La_Paz") },
  { FastName("Newfoundland Standard Time"), FastName("America/St_Johns") },
  { FastName("E. South America Standard Time"), FastName("America/Sao_Paulo") },
  { FastName("SA Eastern Standard Time"), FastName("America/Cayenne") },
  { FastName("Argentina Standard Time"), FastName("America/Buenos_Aires") },
  { FastName("Greenland Standard Time"), FastName("America/Godthab") },
  { FastName("Montevideo Standard Time"), FastName("America/Montevideo") },
  { FastName("Bahia Standard Time"), FastName("America/Bahia") },
  { FastName("Pacific SA Standard Time"), FastName("America/Santiago") },
  { FastName("UTC-02"), FastName("Etc/GMT+2") },
  { FastName("Azores Standard Time"), FastName("Atlantic/Azores") },
  { FastName("Cape Verde Standard Time"), FastName("Atlantic/Cape_Verde") },
  { FastName("Morocco Standard Time"), FastName("Africa/Casablanca") },
  { FastName("UTC"), FastName("Etc/GMT") },
  { FastName("GMT Standard Time"), FastName("Europe/London") },
  { FastName("Greenwich Standard Time"), FastName("Atlantic/Reykjavik") },
  { FastName("W. Europe Standard Time"), FastName("Europe/Berlin") },
  { FastName("Central Europe Standard Time"), FastName("Europe/Budapest") },
  { FastName("Romance Standard Time"), FastName("Europe/Paris") },
  { FastName("Central European Standard Time"), FastName("Europe/Warsaw") },
  { FastName("W. Central Africa Standard Time"), FastName("Africa/Lagos") },
  { FastName("Namibia Standard Time"), FastName("Africa/Windhoek") },
  { FastName("Jordan Standard Time"), FastName("Asia/Amman") },
  { FastName("GTB Standard Time"), FastName("Europe/Bucharest") },
  { FastName("Middle East Standard Time"), FastName("Asia/Beirut") },
  { FastName("Egypt Standard Time"), FastName("Africa/Cairo") },
  { FastName("Syria Standard Time"), FastName("Asia/Damascus") },
  { FastName("E. Europe Standard Time"), FastName("Europe/Chisinau") },
  { FastName("South Africa Standard Time"), FastName("Africa/Johannesburg") },
  { FastName("FLE Standard Time"), FastName("Europe/Kiev") },
  { FastName("Turkey Standard Time"), FastName("Europe/Istanbul") },
  { FastName("Israel Standard Time"), FastName("Asia/Jerusalem") },
  { FastName("Kaliningrad Standard Time"), FastName("Europe/Kaliningrad") },
  { FastName("Libya Standard Time"), FastName("Africa/Tripoli") },
  { FastName("Arabic Standard Time"), FastName("Asia/Baghdad") },
  { FastName("Arab Standard Time"), FastName("Asia/Riyadh") },
  { FastName("Belarus Standard Time"), FastName("Europe/Minsk") },
  { FastName("Russian Standard Time"), FastName("Europe/Moscow") },
  { FastName("E. Africa Standard Time"), FastName("Africa/Nairobi") },
  { FastName("Iran Standard Time"), FastName("Asia/Tehran") },
  { FastName("Arabian Standard Time"), FastName("Asia/Dubai") },
  { FastName("Azerbaijan Standard Time"), FastName("Asia/Baku") },
  { FastName("Russia Time Zone 3"), FastName("Europe/Samara") },
  { FastName("Mauritius Standard Time"), FastName("Indian/Mauritius") },
  { FastName("Georgian Standard Time"), FastName("Asia/Tbilisi") },
  { FastName("Caucasus Standard Time"), FastName("Asia/Yerevan") },
  { FastName("Afghanistan Standard Time"), FastName("Asia/Kabul") },
  { FastName("West Asia Standard Time"), FastName("Asia/Tashkent") },
  { FastName("Ekaterinburg Standard Time"), FastName("Asia/Yekaterinburg") },
  { FastName("Pakistan Standard Time"), FastName("Asia/Karachi") },
  { FastName("India Standard Time"), FastName("Asia/Calcutta") },
  { FastName("Sri Lanka Standard Time"), FastName("Asia/Colombo") },
  { FastName("Nepal Standard Time"), FastName("Asia/Katmandu") },
  { FastName("Central Asia Standard Time"), FastName("Asia/Almaty") },
  { FastName("Bangladesh Standard Time"), FastName("Asia/Dhaka") },
  { FastName("N. Central Asia Standard Time"), FastName("Asia/Novosibirsk") },
  { FastName("Myanmar Standard Time"), FastName("Asia/Rangoon") },
  { FastName("SE Asia Standard Time"), FastName("Asia/Bangkok") },
  { FastName("North Asia Standard Time"), FastName("Asia/Krasnoyarsk") },
  { FastName("China Standard Time"), FastName("Asia/Shanghai") },
  { FastName("North Asia East Standard Time"), FastName("Asia/Irkutsk") },
  { FastName("Singapore Standard Time"), FastName("Asia/Singapore") },
  { FastName("W. Australia Standard Time"), FastName("Australia/Perth") },
  { FastName("Taipei Standard Time"), FastName("Asia/Taipei") },
  { FastName("Ulaanbaatar Standard Time"), FastName("Asia/Ulaanbaatar") },
  { FastName("North Korea Standard Time"), FastName("Asia/Pyongyang") },
  { FastName("Tokyo Standard Time"), FastName("Asia/Tokyo") },
  { FastName("Korea Standard Time"), FastName("Asia/Seoul") },
  { FastName("Yakutsk Standard Time"), FastName("Asia/Yakutsk") },
  { FastName("Cen. Australia Standard Time"), FastName("Australia/Adelaide") },
  { FastName("AUS Central Standard Time"), FastName("Australia/Darwin") },
  { FastName("E. Australia Standard Time"), FastName("Australia/Brisbane") },
  { FastName("AUS Eastern Standard Time"), FastName("Australia/Sydney") },
  { FastName("West Pacific Standard Time"), FastName("Pacific/Port_Moresby") },
  { FastName("Tasmania Standard Time"), FastName("Australia/Hobart") },
  { FastName("Magadan Standard Time"), FastName("Asia/Magadan") },
  { FastName("Vladivostok Standard Time"), FastName("Asia/Vladivostok") },
  { FastName("Russia Time Zone 10"), FastName("Asia/Srednekolymsk") },
  { FastName("Central Pacific Standard Time"), FastName("Pacific/Guadalcanal") },
  { FastName("Russia Time Zone 11"), FastName("Asia/Kamchatka") },
  { FastName("New Zealand Standard Time"), FastName("Pacific/Auckland") },
  { FastName("UTC+12"), FastName("Etc/GMT-12") },
  { FastName("Fiji Standard Time"), FastName("Pacific/Fiji") },
  { FastName("Tonga Standard Time"), FastName("Pacific/Tongatapu") },
  { FastName("Samoa Standard Time"), FastName("Pacific/Apia") },
  { FastName("Line Islands Standard Time"), FastName("Pacific/Kiritimati") }
};
}
}
#endif