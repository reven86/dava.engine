#pragma once

namespace QualityPreferences
{
/** loads quality preferences from application settings and sets them to QualitySettingsSystem */
void Load();

/** gets quality preferences from QualitySettingsSystem and saves them to application settings*/
void Save();
};
