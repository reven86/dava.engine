#pragma once

#include <Base/BaseTypes.h>
#include <Base/FastName.h>
#include <Base/RefPtr.h>
#include <Scene3D/Scene.h>

namespace DAVA
{
class CustomPropertiesComponent;
} // namespace DAVA

namespace DAEConverter
{
struct ImportParams
{
    ImportParams() = default;
    ImportParams(const ImportParams& other) = delete;
    ImportParams& operator=(const ImportParams& other) = delete;
    ~ImportParams();

    DAVA::Map<DAVA::FastName, DAVA::Component*> customPropertiesMap;
    DAVA::Map<DAVA::FastName, DAVA::Component*> lodComponentsMap;
    DAVA::Map<DAVA::FastName, DAVA::RefPtr<DAVA::KeyedArchive>> materialsMap;
};

void RestoreSceneParams(DAVA::RefPtr<DAVA::Scene> scene, const DAVA::FilePath& scenePath, ImportParams* params);
void AccumulateImportParams(DAVA::RefPtr<DAVA::Scene> scene, const DAVA::FilePath& scenePath, ImportParams* params);

} // namespace DAEConverter
