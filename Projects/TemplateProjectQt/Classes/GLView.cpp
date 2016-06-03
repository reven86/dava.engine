#include "GLView.h"
#include "QtTools/DavaGLWidget/DavaGLWidget.h"

GLView::GLView()
    : glWidget(new DavaGLWidget())
    , layoutHint("default")
{
    glWidget = new DavaGLWidget();
}

GLView::~GLView()
{
    delete glWidget;
}

const char* GLView::id() const
{
    return "glWidget";
}

const char* GLView::title() const
{
    return "title";
}

const char* GLView::windowId() const
{
    return "";
}

const LayoutHint& GLView::hint() const
{
    return layoutHint;
};

void GLView::update()
{
    glWidget->update();
}

void GLView::focusInEvent()
{
}

void GLView::focusOutEvent()
{
}

void GLView::registerListener(IViewEventListener* listener)
{
}

void GLView::deregisterListener(IViewEventListener* listener)
{
}

QWidget* GLView::releaseView()
{
    return glWidget;
};

void GLView::retainView()
{
}

QWidget* GLView::view() const
{
    return glWidget;
};