#pragma once

#include "UI/UI3DView.h"

class BaseTest;
class CustomUI3DView : public DAVA::UI3DView
{
public:
    CustomUI3DView(BaseTest* baseTest, const DAVA::Rect& rect = DAVA::Rect());

protected:
    void OnVisible() override;

private:
    void Update(DAVA::float32 timeElapsed);

    BaseTest* baseTest = nullptr;
};