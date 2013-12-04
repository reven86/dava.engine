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


#ifndef SurfaceTester_Test_h
#define SurfaceTester_Test_h

#include "DAVAEngine.h"
#include "LandscapeTestData.h"

using namespace DAVA;

class Test: public DAVA::UIScreen
{
protected:
	~Test(){}
public:
	Test(const FilePath & fullName);
	
	virtual void LoadResources();
	virtual void UnloadResources();
	virtual void WillAppear();
	virtual void WillDisappear();
	
	virtual void Update(float32 timeElapsed);
	virtual void Draw(const UIGeometricData &geometricData);
	
	virtual void Input(DAVA::UIEvent * touch);
	
	int32 GetScreenId() const {return screenId;};
	
	bool IsFinished() const {return isFinished;};

	Texture* GetLandscapeTexture();

	const FilePath & GetFilePath() const;

	const LandscapeTestData& GetLandscapeTestData() const {return testData;};
private:
	Test();
	
	static int32 globalScreenId;
	int32 screenId;
	
	int32 skipFrames;

	FilePath fullName;
	
	LandscapeTestData testData;
    Vector3 curCameraPosition;
    uint32 nextRectNum;

    float32 curCameraAngle;
	float32 curSectorTime;
	int32 curSectorFrames;
	int32 curSectorIndex;
    LinearAnimation<float32>* camRotateAnimation;

    Vector<DAVA::Rect> rectSequence;
	FpsStatItem fpsStatItem;

	bool isFinished;
	
    Vector3 GetRealPoint(const Vector2& point);
    
	void PreparePath();
    void PrepareCameraPosition();
    void PrepareFpsStat();
    bool MoveToNextPoint();
    void SaveFpsStat();
	void ZeroCurFpsStat();
	void OnSectorCameraAnimationEnded(BaseObject* caller, void* userData, void* callerData);

	inline UI3DView* GetSceneView();
	inline Scene* GetScene();
	inline Camera* GetCamera();
	inline Landscape* GetLandscape();
};

#endif
