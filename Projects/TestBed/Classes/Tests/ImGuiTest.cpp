#include "Tests/ImGuiTest.h"
#include "Debug/Private/ImGui.h"

ImGuiTest::ImGuiTest(TestBed& app)
    : BaseScreen(app, "ImGuiTest")
{
    backColor = DAVA::Color(.44f, .44f, .6f, 1.f);
}

void ImGuiTest::LoadResources()
{
    GetBackground()->SetDrawType(DAVA::UIControlBackground::DRAW_FILL);
    GetBackground()->SetColor(backColor);
}

void ImGuiTest::Update(DAVA::float32 timeElapsed)
{
    DVASSERT(ImGui::IsInitialized());

    // 1. Show a simple window
    // Tip: if we don't call ImGui::Begin()/ImGui::End() the widgets appears in a window automatically called "Debug"
    {
        static float f = 0.0f;
        ImGui::Text("Hello, world!");
        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);
        if (ImGui::ColorEdit3("clear color", backColor.color))
            GetBackground()->SetColor(backColor);
        if (ImGui::Button("Test Window"))
            showTestWindow ^= 1;
        if (ImGui::Button("Another Window"))
            showAnotherWindow ^= 1;
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
    }

    // 2. Show another simple window, this time using an explicit Begin/End pair
    if (showAnotherWindow)
    {
        ImGui::SetNextWindowSize(ImVec2(200, 100), ImGuiSetCond_FirstUseEver);
        ImGui::Begin("Another Window", &showAnotherWindow);
        ImGui::Text("Hello");
        ImGui::End();
    }

    // 3. Show the ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
    if (showTestWindow)
    {
        ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
        ImGui::ShowTestWindow(&showTestWindow);
    }
}
