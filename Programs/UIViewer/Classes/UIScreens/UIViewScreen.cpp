#include "UIScreens/UIViewScreen.h"

#include <Base/ScopedPtr.h>
#include <Base/ObjectFactory.h>
#include <Engine/Engine.h>
#include <FileSystem/LocalizationSystem.h>
#include <FileSystem/FileSystem.h>
#include <Render/2D/TextBlock.h>
#include <Render/2D/Systems/VirtualCoordinatesSystem.h>
#include <Sound/SoundSystem.h>
#include <UI/Layouts/UILayoutSystem.h>
#include <UI/Styles/UIStyleSheetSystem.h>
#include <UI/Update/UIUpdateComponent.h>
#include <UI/DefaultUIPackageBuilder.h>
#include <UI/UIControlSystem.h>
#include <UI/UIPackageLoader.h>
#include <UI/UIStaticText.h>
#include <UI/UIYamlLoader.h>
#include <Utils/UTF8Utils.h>

namespace UIViewScreenDetails
{
class PreviewPackageBuilder : public DAVA::DefaultUIPackageBuilder
{
public:
    PreviewPackageBuilder(DAVA::UIPackagesCache* packagesCache_ = nullptr)
        : DAVA::DefaultUIPackageBuilder(packagesCache_)
    {
    }

protected:
    DAVA::UIControl* CreateControlByName(const DAVA::String& customClassName, const DAVA::String& className) override
    {
        using namespace DAVA;

        if (ObjectFactory::Instance()->IsTypeRegistered(customClassName))
        {
            return ObjectFactory::Instance()->New<UIControl>(customClassName);
        }

        return ObjectFactory::Instance()->New<UIControl>(className);
    }

    std::unique_ptr<DAVA::DefaultUIPackageBuilder> CreateBuilder(DAVA::UIPackagesCache* packagesCache) override
    {
        return std::make_unique<PreviewPackageBuilder>(packagesCache);
    }
};

DAVA::UIControl* LoadControl(const DAVA::FilePath& yamlPath, const DAVA::String& controlName)
{
    PreviewPackageBuilder packageBuilder;
    if (DAVA::UIPackageLoader().LoadPackage(yamlPath, &packageBuilder))
    {
        DAVA::UIControl* ctrl = packageBuilder.GetPackage()->GetControl<DAVA::UIControl*>(controlName);
        return DAVA::SafeRetain(ctrl);
    }

    return nullptr;
}
}

UIViewScreen::UIViewScreen(DAVA::Window* window_)
    : BaseScreen()
    , window(window_)
{
    GetOrCreateComponent<DAVA::UIUpdateComponent>();
}

void UIViewScreen::LoadResources()
{
    BaseScreen::LoadResources();

    SetupEnvironment();
    SetupUI();
}

void UIViewScreen::UnloadResources()
{
    ClearEnvironment();
    BaseScreen::UnloadResources();
}

void UIViewScreen::SetupEnvironment()
{
    using namespace DAVA;

    //All this settings should be transfered from QuickEd

    FilePath projectPath("~doc:/UIViewer/Project/");
    FilePath::AddResourcesFolder(projectPath);

    String locale = "en";
    { //  localization
        FilePath stringsFolder("~res:/Strings/");
        LocalizationSystem* localizationSystem = GetEngineContext()->localizationSystem;
        localizationSystem->SetDirectory(stringsFolder);
        localizationSystem->SetCurrentLocale(locale);
        localizationSystem->Init();
    }

    { //  rtl
        bool isRtl = true;
        TextBlock::SetBiDiSupportEnabled(isRtl);
        window->GetUIControlSystem()->GetLayoutSystem()->SetRtl(isRtl);
    }

    { //  sound
        SoundSystem* soundSystem = GetEngineContext()->soundSystem;
        soundSystem->SetCurrentLocale(locale);
    }

    { //  Fonts
        FilePath fontsConfigsDirectory("~res:/Fonts/");
        FilePath localizedFontsPath(fontsConfigsDirectory + locale + "/fonts.yaml");
        if (GetEngineContext()->fileSystem->Exists(localizedFontsPath) == false)
        {
            localizedFontsPath = fontsConfigsDirectory + "/fonts.yaml";
        }
        UIYamlLoader::LoadFonts(localizedFontsPath);
    }

    { //  Size
        Size2f size = window->GetSize();

        VirtualCoordinatesSystem* vcs = window->GetUIControlSystem()->vcs;
        vcs->UnregisterAllAvailableResourceSizes();
        vcs->SetVirtualScreenSize(static_cast<int32>(size.dx), static_cast<int32>(size.dy));
        vcs->RegisterAvailableResourceSize(static_cast<int32>(size.dx), static_cast<int32>(size.dy), "Gfx");
        vcs->RegisterAvailableResourceSize(static_cast<int32>(size.dx * 2.0f), static_cast<int32>(size.dy * 2.0f), "Gfx2");

        window->sizeChanged.Connect(this, &UIViewScreen::OnWindowSizeChanged);
    }

    Vector<String> globalClasses;
    //    Split(CommandLineParser::GetCommandParam("-classes"), ";", globalClasses);    //we should use correct classes for real app
    for (const String& className : globalClasses)
    {
        window->GetUIControlSystem()->GetStyleSheetSystem()->AddGlobalClass(FastName(className));
    }
    window->GetUIControlSystem()->GetStyleSheetSystem()->AddGlobalClass(FastName(locale));
}

void UIViewScreen::ClearEnvironment()
{
    using namespace DAVA;

    FilePath projectPath("~doc:/UIViewer/Project/");
    FilePath::RemoveResourcesFolder(projectPath);
}

void UIViewScreen::OnWindowSizeChanged(DAVA::Window* window, DAVA::Size2f size, DAVA::Size2f surfaceSize)
{
    using namespace DAVA;

    VirtualCoordinatesSystem* vcs = window->GetUIControlSystem()->vcs;
    vcs->UnregisterAllAvailableResourceSizes();
    vcs->SetVirtualScreenSize(static_cast<int32>(size.dx), static_cast<int32>(size.dy));
    vcs->RegisterAvailableResourceSize(static_cast<int32>(size.dx), static_cast<int32>(size.dy), "Gfx");
    vcs->RegisterAvailableResourceSize(static_cast<int32>(size.dx * 2.0f), static_cast<int32>(size.dy * 2.0f), "Gfx2");
}

void UIViewScreen::SetupUI()
{
    using namespace DAVA;

    FilePath placeHolderYaml = "~res:/UI/Screens/Battle/BattleLoadingScreen.yaml";
    String placeHolderRootControl = "BattleLoadingScreen";
    String placeHolderControl = "MapNameLeftBg";

    FilePath testedYaml = "~res:/UI/Screens/Battle/ChatButton.yaml";
    String testedControlName = "ChatButton";

    ScopedPtr<UIControl> placeHolderRoot(UIViewScreenDetails::LoadControl(placeHolderYaml, placeHolderRootControl));
    if (placeHolderRoot)
    {
        AddControl(placeHolderRoot);
        //        UIControl* placeholder = placeHolderRoot->FindByPath(placeHolderControl);     // we should use this method in case of using path
        UIControl* placeholder = placeHolderRoot->FindByName(placeHolderControl, true); // we should use this method in case of using name
        if (placeholder != nullptr)
        {
            ScopedPtr<UIControl> testedControl(UIViewScreenDetails::LoadControl(testedYaml, testedControlName));
            if (testedControl)
            {
                placeholder->AddControl(testedControl);
            }
            else
            {
                PrintError(Format("Cannot find %s in %s", testedControlName.c_str(), testedYaml.GetStringValue().c_str()));
            }
        }
        else
        {
            PrintError(Format("Cannot find control %s in %s", placeHolderControl.c_str(), placeHolderYaml.GetStringValue().c_str()));
        }
    }
    else
    {
        PrintError(Format("Cannot find %s in %s", placeHolderRootControl.c_str(), placeHolderYaml.GetStringValue().c_str()));
    }
}

void UIViewScreen::PrintError(const DAVA::String& errorMessage)
{
    using namespace DAVA;

    RemoveAllControls();

    ScopedPtr<UIStaticText> errorText(new UIStaticText(GetRect()));
    errorText->SetFont(font);
    errorText->SetTextColorInheritType(UIControlBackground::COLOR_IGNORE_PARENT);
    errorText->SetTextColor(Color(1.f, 0.f, 0.f, 1.f));
    errorText->SetText(UTF8Utils::EncodeToWideString(errorMessage));
    AddControl(errorText);
}
