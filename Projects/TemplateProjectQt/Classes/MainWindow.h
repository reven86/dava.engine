#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui
{
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = 0);
    ~MainWindow();

private:
    void CreateGlWidget();

private slots:
    void OnGlWidgedResized(int width, int height, int dpr);

private:
    Ui::MainWindow* ui;
};

#endif // MAINWINDOW_H
