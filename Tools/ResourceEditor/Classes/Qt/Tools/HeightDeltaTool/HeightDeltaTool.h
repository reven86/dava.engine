#ifndef __HEIGHTMAPPATH_TOOL_H__
#define __HEIGHTMAPPATH_TOOL_H__

#include "DAVAEngine.h"

#include <QWidget>
#include <QScopedPointer>


namespace Ui { class HeightDeltaTool; }


class HeightDeltaTool
    : public QWidget
{
    Q_OBJECT

public:
    explicit HeightDeltaTool(QWidget *p = NULL);
    ~HeightDeltaTool();

    void SetDefaultDir(const QString& path);
    void SetOutputTemplate(const QString& prefix, const QString& suffix);

private slots:
    void OnBrowse();
    void OnRun();
    void OnValueChanged();

private:
    double GetThresholdInMeters(double unitSize);

    QScopedPointer<Ui::HeightDeltaTool> ui;
    QString defaultDir;
    QString outTemplate;
    QString inPath;
    QString outPath;
    QString outName;
};


#endif // __HEIGHTMAPPATH_TOOL_H__
