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


#ifndef __UIEditor__UIJoypadMetadata__
#define __UIEditor__UIJoypadMetadata__

#include "UIControlMetadata.h"
#include "UI/UIJoypad.h"

namespace DAVA {

// Metadata class for DAVA UIJoypad control.
class UIJoypadMetadata : public UIControlMetadata
{
    Q_OBJECT

    Q_PROPERTY(QString StickSprite READ GetStickSprite WRITE SetStickSprite);
    Q_PROPERTY(int StickSpriteFrame READ GetStickSpriteFrame WRITE SetStickSpriteFrame);
    Q_PROPERTY(int DeadArea READ GetDeadArea WRITE SetDeadArea);
    Q_PROPERTY(float DigitalSense READ GetDigitalSense WRITE SetDigitalSense);

public:
    UIJoypadMetadata(QObject* parent = 0);

protected:
    virtual bool GetInitialInputEnabled() const {return true;};

    virtual QString GetUIControlClassName() const { return "UIJoypad"; };

    // Helper to access active UIJoypad.
    UIJoypad* GetActiveJoypad() const;

    // Getters/setters.
    QString GetStickSprite() const;
    void SetStickSprite(const QString& value);

    int GetStickSpriteFrame() const;
    void SetStickSpriteFrame(int value);

    int GetDeadArea() const;
    void SetDeadArea(int value);
    
    float GetDigitalSense() const;
    void SetDigitalSense(float value);
};

};

#endif /* defined(__UIEditor__UIJoypadMetadata__) */
