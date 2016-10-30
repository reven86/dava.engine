#ifndef __FILE_DIALOG_H__
#define __FILE_DIALOG_H__

#include <QFileDialog>

class FileDialog : public QFileDialog
{
public:
    static QString getOpenFileName(QWidget* parent = 0,
                                   const QString& caption = QString(),
                                   const QString& dir = QString(),
                                   const QString& filter = QString(),
                                   QString* selectedFilter = 0,
                                   Options options = 0);

    static QUrl getOpenFileUrl(QWidget* parent = 0,
                               const QString& caption = QString(),
                               const QUrl& dir = QUrl(),
                               const QString& filter = QString(),
                               QString* selectedFilter = 0,
                               Options options = 0,
                               const QStringList& supportedSchemes = QStringList());

    static QString getSaveFileName(QWidget* parent = 0,
                                   const QString& caption = QString(),
                                   const QString& dir = QString(),
                                   const QString& filter = QString(),
                                   QString* selectedFilter = 0,
                                   Options options = 0);

    static QUrl getSaveFileUrl(QWidget* parent = 0,
                               const QString& caption = QString(),
                               const QUrl& dir = QUrl(),
                               const QString& filter = QString(),
                               QString* selectedFilter = 0,
                               Options options = 0,
                               const QStringList& supportedSchemes = QStringList());

    static QString getExistingDirectory(QWidget* parent = 0,
                                        const QString& caption = QString(),
                                        const QString& dir = QString(),
                                        Options options = ShowDirsOnly);

    static QUrl getExistingDirectoryUrl(QWidget* parent = 0,
                                        const QString& caption = QString(),
                                        const QUrl& dir = QUrl(),
                                        Options options = ShowDirsOnly,
                                        const QStringList& supportedSchemes = QStringList());

    static QStringList getOpenFileNames(QWidget* parent = 0,
                                        const QString& caption = QString(),
                                        const QString& dir = QString(),
                                        const QString& filter = QString(),
                                        QString* selectedFilter = 0,
                                        Options options = 0);

    static QList<QUrl> getOpenFileUrls(QWidget* parent = 0,
                                       const QString& caption = QString(),
                                       const QUrl& dir = QUrl(),
                                       const QString& filter = QString(),
                                       QString* selectedFilter = 0,
                                       Options options = 0,
                                       const QStringList& supportedSchemes = QStringList());
};

#endif // __FILE_DIALOG_H__
