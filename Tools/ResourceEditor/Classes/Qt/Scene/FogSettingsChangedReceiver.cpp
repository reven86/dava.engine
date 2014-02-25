#include "Scene/SceneEditor2.h"
#include "Scene/SceneSignals.h"

#include "Classes/Commands2/InspMemberModifyCommand.h"

#include "Scene/FogSettingsChangedReceiver.h"

FogSettingsChangedReceiver::FogSettingsChangedReceiver()
{
	QObject::connect(SceneSignals::Instance(), SIGNAL(CommandExecuted(SceneEditor2 *, const Command2*, bool)),
					 this, SLOT(SceneCommandExecuted(SceneEditor2 *, const Command2*, bool)));
}

FogSettingsChangedReceiver::~FogSettingsChangedReceiver()
{
}

void FogSettingsChangedReceiver::SceneCommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo)
{
	if (command->GetId() == CMDID_INSP_MEMBER_MODIFY)
	{
		const InspMemberModifyCommand* cmd = (const InspMemberModifyCommand*)command;
		String name = cmd->member->Name();
		if (name == "fogDensity" || name == "fogColor")
		{
			Landscape* landscape = (Landscape*)cmd->object;

			bool fog = landscape->IsFogEnabled();
			landscape->SetFog(false);
			landscape->SetFog(fog);
		}
	}
}
