/*==================================================================================
    Copyright (c) 2008, binaryzebra
    All rights reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
    notice, this list of conditions and the following disclaimer in the
    documentation and/or other materials provided with the distribution.
    * Neither the name of the binaryzebra nor the
    names of its contributors may be used to endorse or promote products
    derived from this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE binaryzebra AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL binaryzebra BE LIABLE FOR ANY
    DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
    ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
=====================================================================================*/


#ifndef __PARTICLE_TIME_LINE_COLUMNS__H__
#define __PARTICLE_TIME_LINE_COLUMNS__H__

#include "DAVAEngine.h"

#include <QWidget>
#include "ParticleTimeLineWidget.h"

////////////////////////////////////////////////////////////////////////////////////
// A specific class to display the per-layer particles info.
const int32 EXTRA_INFO_DEFAULT_COLUMN_WIDTH = 60;
const int32 EXTRA_INFO_LEFT_PADDING = 3;

// Sizes for specific columns.
const int32 EXTRA_INFO_COUNT_COLUMN_WIDTH = 40;
const int32 EXTRA_INFO_MAX_COUNT_COLUMN_WIDTH = 52;
const int32 EXTRA_INFO_AREA_COLUMN_WIDTH = 50;

class ParticlesExtraInfoColumn : public QWidget
{
	Q_OBJECT
	
public:
	explicit ParticlesExtraInfoColumn(const ParticleTimeLineWidget* timeLineWidget,
									  QWidget *parent = 0);

	// This method is called when the list of layers is changed.
	virtual void OnLayersListChanged() {};

	// This method returns the preferred column's width.
	virtual int32 GetColumnWidth() { return EXTRA_INFO_DEFAULT_COLUMN_WIDTH; };

	// This method is called when the appropriate particle effect is started/stopped/restarted.
	virtual void Reset() {};

protected:
	virtual void paintEvent(QPaintEvent *);
	
	// These methods are to be overriden for derived classes.
	// Get an extra information to be displayed near the line.
	virtual QString GetExtraInfoForLayerLine(ParticleEffectComponent *effect, const ParticleTimeLineWidget::LINE& line) {return QString();};
	
	// Get the extra info for the header/footer.
	virtual QString GetExtraInfoHeader() {return QString(); };
	virtual QString GetExtraInfoFooter() {return QString(); };
	
	// In case some information should be accumulated during the loop,
	// these methods are called just before the loop and just after it.
	virtual void OnBeforeGetExtraInfoLoop() {};
	virtual void OnAfterGetExtraInfoLoop() {};
	
	// The timeline widget being used.
	const ParticleTimeLineWidget* timeLineWidget;
	
	// Helper methods.
	QString FormatFloat(float32 value);
};

// Base class for Cumulative Columns.
class ParticlesExtraInfoCumulativeColumn : public ParticlesExtraInfoColumn
{
	Q_OBJECT
public:
	explicit ParticlesExtraInfoCumulativeColumn(const ParticleTimeLineWidget* timeLineWidget,
												QWidget *parent = 0);
	virtual void OnLayersListChanged();

protected:
	// Add the value to the Cumulative Data.
	void UpdateCumulativeData(ParticleLayer* layer, float32 value);
	
	// Update the value in Cumulative Data if the new value is bigger than existing one.
	void UpdateCumulativeDataIfMaximum(ParticleLayer* layer, float32 value);
	
	// Cleanup all the Cumulative Data.
	void CleanupCumulativeData();

	int32 totalParticlesCount;
	float32 totalParticlesArea;
	int32 totalUpdatesCount;

	Map<ParticleLayer*, float32> cumulativeData;
};

// Particles Count information.
class ParticlesCountColumn : public ParticlesExtraInfoColumn
{
	Q_OBJECT
public:
	explicit ParticlesCountColumn(const ParticleTimeLineWidget* timeLineWidget,
								  QWidget *parent = 0);

	virtual int32 GetColumnWidth() { return EXTRA_INFO_COUNT_COLUMN_WIDTH; };

protected:
	virtual void OnBeforeGetExtraInfoLoop();
	virtual QString GetExtraInfoForLayerLine(ParticleEffectComponent *effect, const ParticleTimeLineWidget::LINE& line);
	
	virtual QString GetExtraInfoHeader();
	virtual QString GetExtraInfoFooter();
	
private:
	int32 totalParticlesCount;
};

class ParticlesAverageCountColumn : public ParticlesExtraInfoCumulativeColumn
{
	Q_OBJECT
public:
	explicit ParticlesAverageCountColumn(const ParticleTimeLineWidget* timeLineWidget,
									 QWidget *parent = 0);
	
	virtual void Reset();

protected:
	virtual QString GetExtraInfoForLayerLine(ParticleEffectComponent *effect, const ParticleTimeLineWidget::LINE& line);
	
	virtual QString GetExtraInfoHeader();
	virtual QString GetExtraInfoFooter();
};

class ParticlesMaxCountColumn : public ParticlesExtraInfoCumulativeColumn
{
	Q_OBJECT
public:
	explicit ParticlesMaxCountColumn(const ParticleTimeLineWidget* timeLineWidget,
								  QWidget *parent = 0);
	
	virtual int32 GetColumnWidth() { return EXTRA_INFO_MAX_COUNT_COLUMN_WIDTH; };
	virtual void OnLayersListChanged();
	virtual void Reset();

protected:
	virtual QString GetExtraInfoForLayerLine(ParticleEffectComponent *effect, const ParticleTimeLineWidget::LINE& line);

	virtual void OnBeforeGetExtraInfoLoop();
	virtual void OnAfterGetExtraInfoLoop();

	virtual QString GetExtraInfoHeader();
	virtual QString GetExtraInfoFooter();
	
private:
	int32 maxParticlesCount;
	int32 totalParticlesCountOnThisLoop;
};

// Particles Area information.
class ParticlesAreaColumn : public ParticlesExtraInfoColumn
{
	Q_OBJECT
public:
	explicit ParticlesAreaColumn(const ParticleTimeLineWidget* timeLineWidget,
								 QWidget *parent = 0);
	virtual int32 GetColumnWidth() { return EXTRA_INFO_AREA_COLUMN_WIDTH; };

protected:
	virtual void OnBeforeGetExtraInfoLoop();
	virtual QString GetExtraInfoForLayerLine(ParticleEffectComponent *effect, const ParticleTimeLineWidget::LINE& line);
	
	virtual QString GetExtraInfoHeader();
	virtual QString GetExtraInfoFooter();

private:
	float32 totalParticlesArea;
};

class ParticlesAverageAreaColumn : public ParticlesExtraInfoCumulativeColumn
{
	Q_OBJECT
public:
	explicit ParticlesAverageAreaColumn(const ParticleTimeLineWidget* timeLineWidget,
										 QWidget *parent = 0);
	virtual void Reset();

protected:
	virtual QString GetExtraInfoForLayerLine(ParticleEffectComponent *effect, const ParticleTimeLineWidget::LINE& line);
	
	virtual QString GetExtraInfoHeader();
	virtual QString GetExtraInfoFooter();
};

class ParticlesMaxAreaColumn : public ParticlesExtraInfoCumulativeColumn
{
	Q_OBJECT
public:
	explicit ParticlesMaxAreaColumn(const ParticleTimeLineWidget* timeLineWidget,
									QWidget *parent = 0);
	virtual void OnLayersListChanged();
	virtual void Reset();

protected:
	virtual QString GetExtraInfoForLayerLine(ParticleEffectComponent *effect, const ParticleTimeLineWidget::LINE& line);

	virtual void OnBeforeGetExtraInfoLoop();
	virtual void OnAfterGetExtraInfoLoop();

	virtual QString GetExtraInfoHeader();
	virtual QString GetExtraInfoFooter();
	
private:
	float32 maxParticlesArea;
	float32 totalParticlesAreaOnThisLoop;
};


#endif /* __PARTICLE_TIME_LINE_COLUMNS__H__ */
