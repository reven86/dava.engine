#ifndef __DAVEENGINE_UI_UPDATE_SYSTEM_H__
#define __DAVEENGINE_UI_UPDATE_SYSTEM_H__

#include "UISystem.h"
#include "UI/UIControl.h"

namespace DAVA
{

class UIUpdateSystem : public UISystem
{
public:
	static const uint32 TYPE = UISystem::UI_UPDATE_SYSTEM;

	UIUpdateSystem();
	virtual ~UIUpdateSystem();

	virtual uint32 GetRequiredComponents() const;
	virtual uint32 GetType() const;
	virtual void Process() override;

	void SystemUpdate(UIControl* control, float32 timeElapsed);

};

}

#endif // !__DAVEENGINE_UI_UPDATE_SYSTEM_H__
