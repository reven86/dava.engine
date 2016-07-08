#ifndef __QT_IMAGE_AREA_H__
#define __QT_IMAGE_AREA_H__

#include <QLabel>
#include "DAVAEngine.h"

class QMimeData;

class ImageArea : public QLabel
{
    Q_OBJECT

public:
    explicit ImageArea(QWidget* parent = 0);
    ~ImageArea();
    void SetImage(const DAVA::FilePath& filePath);
    void SetImage(DAVA::Image* image);
    inline DAVA::Image* GetImage() const;
    DAVA::Vector2 GetAcceptableSize() const;
    const DAVA::FilePath& GetImagePath() const;

    void SetRequestedImageFormat(const DAVA::PixelFormat format);
    DAVA::PixelFormat GetRequestedImageFormat() const;

public slots:
    void ClearArea();
    void UpdatePreviewPicture();

    void SetAcceptableSize(const DAVA::Vector2& size);

signals:

    void changed();

private:
    void mousePressEvent(QMouseEvent* event);
    void dragEnterEvent(QDragEnterEvent* event);
    void dropEvent(QDropEvent* event);

    void ConnectSignals();
    DAVA::String GetDefaultPath() const;

    DAVA::Image* image;
    DAVA::Vector2 acceptableSize;
    DAVA::FilePath imagePath;

    DAVA::PixelFormat requestedFormat;
};

inline DAVA::Image* ImageArea::GetImage() const
{
    return image;
}

#endif /* defined(__QT_IMAGE_AREA_H__) */
