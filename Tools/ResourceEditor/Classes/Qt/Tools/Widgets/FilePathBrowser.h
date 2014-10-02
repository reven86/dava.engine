#ifndef FILEPATHBROWSER_H
#define FILEPATHBROWSER_H


#include "LineEditEx.h"


class FilePathBrowser
    : public LineEditEx
{
    Q_OBJECT

signals:
    void pathChanged( const QString& path );

public:
    explicit FilePathBrowser(QWidget *parent = NULL);
    ~FilePathBrowser();

    void SetHint(const QString& hint);
    void SetDefaultFolder(const QString& path);
    void SetPath(const QString& path);
    void SetFilter(const QString& filter);

    QSize sizeHint() const;

protected:
    QString DefaultBrowsePath();

private slots:
    void OnBrowse();
    void OnReturnPressed();

private:
    void InitActions();
    void TryToAcceptPath(const QString& path);

    QSize ButtonSizeHint(const QAction *action) const;

    QString hintText;
    QString defaultFolder;
    QString path;
    QString filter;
};


#endif // FILEPATHBROWSER_H
