//
//  SmartDLC.h
//  Framework
//
//  Created by Aleksei Kanash on 2/4/15.
//
//

#ifndef __SMARTDLC_h_
#define __SMARTDLC_h_

#include "Base/BaseTypes.h"
#include "Base/Singleton.h"
#include "FileSystem/FilePath.h"

namespace DAVA
{

class SmartDLC : public Singleton<SmartDLC>
{
private:
    enum State
    {
        START = 0,
        RESUME,
        PAUSE,
        END,
    };

public:
    SmartDLC();
    ~SmartDLC() override;
    
    void Enable();
    void Disable();
    bool IsEnabled() {return isEnabled;}

private:
    void UpdateState(State nextState);

private:
    bool isEnabled;
    String downloadUrl;
    FilePath savePath;
    State currentState;
    uint32 currentDownloadID;
};

}
#endif
