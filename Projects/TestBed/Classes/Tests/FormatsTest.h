#ifndef __FORMATSTEST_TEST_H__
#define __FORMATSTEST_TEST_H__

#include "DAVAEngine.h"
#include "Infrastructure/BaseScreen.h"

using namespace DAVA;

class FormatsTest : public BaseScreen, public UIListDelegate
{
public:
    FormatsTest();

    void LoadResources() override;
    void UnloadResources() override;

    //UIListDelegate
    float32 CellHeight(UIList* list, int32 index) override;
    int32 ElementsCount(UIList* list) override;
    UIListCell* CellAtIndex(UIList* list, int32 index) override;
    void OnCellSelected(UIList* forList, UIListCell* selectedCell) override;

private:
    UIList* formatsGrid = nullptr;
};

#endif //__FORMATSTEST_TEST_H__
