#include "DAVAEngine.h"

using namespace DAVA;

class TestScreen : public UIScreen
{
protected:
    ~TestScreen()
    {
    }

public:
    virtual void LoadResources();
    virtual void UnloadResources();
    virtual void WillAppear();
    virtual void WillDisappear();

    virtual void Update(float32 timeElapsed);
    virtual void Draw(const UIGeometricData& geometricData);

    virtual void Input(UIEvent* touch);

private:
};
