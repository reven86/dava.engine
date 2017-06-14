#pragma once

class QWidget;

struct Guide
{
    void Show();
    void Hide();

    QWidget* line = nullptr;
    QWidget* text = nullptr;
};
