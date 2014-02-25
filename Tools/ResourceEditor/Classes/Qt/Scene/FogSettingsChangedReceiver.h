#ifndef __RESOURCEEDITORQT__FOGSETTINGSCHANGEDRECEIVER__
#define __RESOURCEEDITORQT__FOGSETTINGSCHANGEDRECEIVER__

#include <QObject>
#include "Base/Singleton.h"

class SceneEditor2;
class Command2;

class FogSettingsChangedReceiver: public QObject, public DAVA::Singleton<FogSettingsChangedReceiver>
{
	Q_OBJECT

public:
	FogSettingsChangedReceiver();
	~FogSettingsChangedReceiver();

private slots:
	void SceneCommandExecuted(SceneEditor2 *scene, const Command2* command, bool redo);
};

#endif /* defined(__RESOURCEEDITORQT__FOGSETTINGSCHANGEDRECEIVER__) */
