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


#ifndef LevelPerformanceTest_ResultScreen_h
#define LevelPerformanceTest_ResultScreen_h

#include "DAVAEngine.h"
#include "LandscapeTestData.h"

enum eResultScreenState
{
	RESULT_STATE_NORMAL = 0,
	RESULT_STATE_MAKING_SCREEN_SHOT,
	RESULT_STATE_FINISHED
};

class ResultScreen: public DAVA::UIScreen
{
protected:
	~ResultScreen();
public:
	ResultScreen(const LandscapeTestData& testData, const DAVA::FilePath& filename, DAVA::Texture* landscapeTexture);
	
	virtual void LoadResources();
	virtual void UnloadResources();
	virtual void WillAppear();
	virtual void WillDisappear();
	
	virtual void Update(DAVA::float32 timeElapsed);
	virtual void Draw(const DAVA::UIGeometricData &geometricData);
	
	virtual void Input(DAVA::UIEvent * touch);

	eResultScreenState GetState() const {return state;};
	bool IsFinished() const {return isFinished;};
private:
	ResultScreen();
	ResultScreen& operator=(const ResultScreen&);
    
    void PrepareSprite();
	
	DAVA::Vector2 GetVecInRect(const DAVA::Rect & rect, DAVA::float32 angleInRad);

	void DrawStatImage(DAVA::Rect rect);
    
    void SaveResults();
    
	DAVA::Texture* texture;
	DAVA::Sprite* textureSprite;
    DAVA::Sprite* resultSprite;

	DAVA::FilePath filename;
	const LandscapeTestData& testData;
	eResultScreenState state;
	bool isFinished;
	
	DAVA::UIStaticText *fileNameText;
	DAVA::UIStaticText *statText[3];
	DAVA::UIStaticText *tapToContinue;
	DAVA::UIStaticText *screenshotText;
	
	DAVA::int32 testCount;
	DAVA::int32 testNumber;
};

#endif
