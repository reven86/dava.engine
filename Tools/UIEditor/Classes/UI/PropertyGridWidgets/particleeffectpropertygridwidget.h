#ifndef __PARTICLEEFFECTPROPERTYGRIDWIDGET_H__
#define __PARTICLEEFFECTPROPERTYGRIDWIDGET_H__

#include "basepropertygridwidget.h"

namespace Ui {
class ParticleEffectPropertyGridWidget;
}

class ParticleEffectPropertyGridWidget : public BasePropertyGridWidget
{
    Q_OBJECT

public:
    explicit ParticleEffectPropertyGridWidget(QWidget *parent = 0);
    ~ParticleEffectPropertyGridWidget();

    virtual void Initialize(BaseMetadata* activeMetadata);
    virtual void Cleanup();

protected slots:
    void OnSelectEffectPathButtonClicked();

protected:
    void UpdateButtons();

private:
    Ui::ParticleEffectPropertyGridWidget *ui;
};

#endif //__PARTICLEEFFECTPROPERTYGRIDWIDGET_H__

