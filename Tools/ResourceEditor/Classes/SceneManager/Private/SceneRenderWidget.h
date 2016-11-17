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

class SceneRenderWidget : public QFrame, private DAVA::TArc::DataListener
{
public:
    class IWidgetDelegate
    {
    public:
        virtual void CloseSceneRequest(DAVA::uint64 id) = 0;
    };

    SceneRenderWidget(DAVA::TArc::ContextAccessor* accessor, DAVA::RenderWidget* renderWidget);

    void SetWidgetDelegate(IWidgetDelegate* widgetDelegate);

private:
    void OnDataChanged(const DAVA::TArc::DataWrapper& wrapper, const DAVA::Vector<DAVA::Any>& fields) override;

    void InitDavaUI();
    void OnRenderWidgetResized(DAVA::uint32 w, DAVA::uint32 h);

    void OnCloseTab(DAVA::uint64 id);

private:
    DAVA::TArc::ContextAccessor* accessor = nullptr;
    DAVA::TArc::DataWrapper activeSceneWrapper;

    DAVA::RefPtr<DAVA::UIScreen> davaUIScreen;
    DAVA::RefPtr<DAVA::UI3DView> dava3DView;
    const int davaUIScreenID = 0;
    const int dava3DViewMargin = 3;

    QtConnections connections;

    IWidgetDelegate* widgetDelegate = nullptr;
};