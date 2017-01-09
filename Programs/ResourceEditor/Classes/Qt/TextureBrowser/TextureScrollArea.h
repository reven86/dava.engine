#ifndef __TEXTURE_SCROLL_AREA_H__
#define __TEXTURE_SCROLL_AREA_H__

#include <QGraphicsView>

#include "Base/BaseTypes.h"

class QImage;
class QLabel;

class TextureScrollArea : public QGraphicsView
{
    Q_OBJECT

public:
    enum TextureColorChannels
    {
        ChannelNo = 0,

        ChannelR = 0x1,
        ChannelG = 0x2,
        ChannelB = 0x4,
        ChannelA = 0x8,

        ChannelAll = 0xFFFFFFFF
    };

    TextureScrollArea(QWidget* parent = 0);
    ~TextureScrollArea();

    void setImage(const QImage& image);
    void setImage(const QList<QImage>& images, int flags = 0x000000FF); //this method sets cubemap faces
    QImage getImage();
    void setColorChannel(int mask);

    QColor getPixelColor(QPoint pos);
    float getTextureZoom();

    void borderShow(bool show);
    void bgmaskShow(bool show);
    void waitbarShow(bool show);

    void warningSetText(const QString& text);
    void warningShow(bool show);

    void resetTexturePosZoom();

    QSize getContentSize();

public slots:
    void setTexturePos(const QPoint& pos);
    void setTextureZoom(const float& zoom);

signals:
    void texturePosChanged(const QPoint& pos);
    void textureZoomChanged(const float& zoom);

    void mouseOverPixel(const QPoint& pos);
    void mouseWheel(int delta);

protected:
    virtual void drawBackground(QPainter* painter, const QRectF& rect);
    virtual void scrollContentsBy(int dx, int dy);
    virtual void wheelEvent(QWheelEvent* e);

    virtual void mouseMoveEvent(QMouseEvent* event);
    virtual void mousePressEvent(QMouseEvent* event);
    virtual void mouseReleaseEvent(QMouseEvent* event);

private:
    int textureColorMask;

    bool mouseInMoveState;
    QPoint mousePressPos;
    QPoint mousePressScrollPos;

    QGraphicsRectItem* textureBorder;
    QGraphicsProxyWidget* waitBar;

    QImage currentTextureImage;

    DAVA::Vector<QImage> currentCompositeImages;
    int compositeImagesFlags;
    QPixmap cubeDrawPixmap;

    QGraphicsScene* textureScene;
    QGraphicsPixmapItem* texturePixmap;
    float zoomFactor;

    bool tiledBgDoDraw;
    QPixmap tiledBgPixmap;

    bool noImageVisible;
    QLabel* noImageLabel;
    QGraphicsProxyWidget* noImageProxy;

    QLabel* warningLabel;
    QGraphicsProxyWidget* warningProxy;

    void sutupCustomTiledBg();

    void applyTextureImageToScenePixmap();

    void applyCurrentImageToScenePixmap();
    void applyCurrentCompositeImagesToScenePixmap();

    void applyTextureImageBorder();
    void applyCompositeImageBorder();
    void applyCurrentImageBorder();

    void adjustWidgetsPos();

    void prepareImageWithColormask(QImage& srcImage, QImage& dstImage);

    bool isCompositeImage() const;
};

#endif // __TEXTURE_SCROLL_AREA_H__
