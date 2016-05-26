#ifndef __BEAST_PROXY__
#define __BEAST_PROXY__

#include "DAVAEngine.h"

class BeastManager;
struct LightmapAtlasingData;
class BeastProxy : public DAVA::Singleton<BeastProxy>
{
public:
    enum eBeastMode : DAVA::uint32
    {
        MODE_LIGHTMAPS = 0,
        MODE_SPHERICAL_HARMONICS,
        MODE_PREVIEW
    };

    virtual BeastManager* CreateManager();
    virtual void SafeDeleteManager(BeastManager** manager){};
    virtual void Update(BeastManager* manager){};
    virtual bool IsJobDone(BeastManager* manager)
    {
        return false;
    }

    virtual int GetCurTaskProcess(BeastManager* manager) const
    {
        return 0;
    }
    virtual DAVA::String GetCurTaskName(BeastManager* manager) const
    {
        return DAVA::String();
    }

    virtual void Run(BeastManager* manager, DAVA::Scene* scene){};
    virtual void SetLightmapsDirectory(BeastManager* manager, const DAVA::FilePath& path){};
    virtual void SetMode(BeastManager* manager, eBeastMode mode){};

    virtual void UpdateAtlas(BeastManager* manager, DAVA::Vector<LightmapAtlasingData>* atlasData){};
    virtual void Cancel(BeastManager* beastManager){};

    virtual bool WasCancelled(BeastManager* beastManager) const
    {
        return false;
    }
};

#endif //__BEAST_PROXY__
