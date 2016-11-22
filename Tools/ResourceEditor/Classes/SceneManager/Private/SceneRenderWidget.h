#pragma once

#include "Base/BaseTypes.h"
#include "Base/Any.h"
#include "Base/RefPtr.h"

#include "Reflection/Reflection.h"

#include "UI/UIScreen.h"
#include "UI/UI3DView.h"

#include "TArc/DataProcessing/DataWrapper.h"
#include "TArc/DataProcessing/DataListener.h"
#include "TArc/Utils/QtConnections.h"

#include <QFrame>

namespace DAVA
{
class RenderWidget;
namespace TArc
{
class ContextAccessor;
}
}

class SceneEditor2;
class SelectableGroup;
class ScenePreviewDialog;
class SceneRenderWidget : public QFrame, private DAVA::TArc::DataListener
{
public:
    class IWidgetDelegate
    {
    public:
        virtual bool CloseSceneRequest(DAVA::uint64 id) = 0;
    };

    SceneRenderWidget(DAVA::TArc::ContextAccessor* accessor, DAVA::RenderWidget* renderWidget);
    ~SceneRenderWidget();

    void ShowPreview(const DAVA::FilePath& scenePath);
    void HidePreview();
    void SetWidgetDelegate(IWidgetDelegate* widgetDelegate);

private:
    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

    void InitDavaUI();
    void OnRenderWidgetResized(DAVA::uint32 w, DAVA::uint32 h);

    void OnCloseTab(DAVA::uint64 id);

    void MouseOverSelection(SceneEditor2* scene, const SelectableGroup* objects);

private:
    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::TArc::DataWrapper activeSceneWrapper;

    DAVA::RefPtr<DAVA::UIScreen> davaUIScreen;
    DAVA::RefPtr<DAVA::UI3DView> dava3DView;
    const int davaUIScreenID = 0;
    const int dava3DViewMargin = 3;

    DAVA::TArc::QtConnections connections;
    DAVA::RenderWidget* renderWidget = nullptr;
    ScenePreviewDialog* previewDialog = nullptr;

    IWidgetDelegate* widgetDelegate = nullptr;
};