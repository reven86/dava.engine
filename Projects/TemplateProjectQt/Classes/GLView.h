#pragma once

#include <core_ui_framework/layout_hint.hpp>
#include <core_qt_common/i_qt_view.hpp>

class DavaGLWidget;

class GLView : public wgt::IQtView
{
public:
    GLView();
    ~GLView() override;

    const char* id() const override;
    const char* title() const override;
    const char* windowId() const override;
    const wgt::LayoutHint& hint() const override;
    void update() override;

    void focusInEvent() override;
    void focusOutEvent() override;

    virtual void registerListener(wgt::IViewEventListener* listener) override;
    virtual void deregisterListener(wgt::IViewEventListener* listener) override;

    QWidget* releaseView() override;
    void retainView() override;
    QWidget* view() const override;
    DavaGLWidget* glWidget;
    wgt::LayoutHint layoutHint;
};