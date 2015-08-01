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


#include "ModificationWidget.h"
#include "Commands2/TransformCommand.h"
#include "Math/MathHelpers.h"

#include <QKeyEvent>
#include <QLineEdit>
#include <QStylePainter>
#include <QHBoxLayout>
#include <QStyleOptionSpinBox>


ModificationWidget::ModificationWidget(QWidget* parent)
	: QWidget(parent)
	, curScene(nullptr)
	, groupMode(false)
	, pivotMode(PivotAbsolute)
	, modifMode(ST_MODIF_OFF)
{
	auto horizontalLayout = new QHBoxLayout(this);
	horizontalLayout->setSpacing(2);
	horizontalLayout->setContentsMargins(2, 1, 2, 1);

	xLabel = new QLabel("X:", this);
	xLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	horizontalLayout->addWidget(xLabel);

	xAxisModify = new DAVAFloat32SpinBox(this);
	xAxisModify->setMinimumSize(QSize(70, 0));
	horizontalLayout->addWidget(xAxisModify);

	yLabel = new QLabel("Y:", this);
	yLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	horizontalLayout->addWidget(yLabel);

	yAxisModify = new DAVAFloat32SpinBox(this);
	yAxisModify->setMinimumSize(QSize(70, 0));
	horizontalLayout->addWidget(yAxisModify);

	zLabel = new QLabel("Z:", this);
	zLabel->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);
	horizontalLayout->addWidget(zLabel);

	zAxisModify = new DAVAFloat32SpinBox(this);
	zAxisModify->setMinimumSize(QSize(70, 0));
	horizontalLayout->addWidget(zAxisModify);

    connect( xAxisModify, &DAVAFloat32SpinBox::valueEdited, this, &ModificationWidget::OnXChanged );
    connect( yAxisModify, &DAVAFloat32SpinBox::valueEdited, this, &ModificationWidget::OnYChanged );
    connect( zAxisModify, &DAVAFloat32SpinBox::valueEdited, this, &ModificationWidget::OnZChanged );

    connect( SceneSignals::Instance(), &SceneSignals::SelectionChanged, this, &ModificationWidget::OnSceneSelectionChanged );
    connect( SceneSignals::Instance(), &SceneSignals::Activated, this, &ModificationWidget::OnSceneActivated );
    connect( SceneSignals::Instance(), &SceneSignals::Deactivated, this, &ModificationWidget::OnSceneDeactivated );
    connect( SceneSignals::Instance(), &SceneSignals::CommandExecuted, this, &ModificationWidget::OnSceneCommand );
}

ModificationWidget::~ModificationWidget()
{}

void ModificationWidget::SetPivotMode(PivotMode mode)
{
	pivotMode = mode;
	ReloadValues();
}

void ModificationWidget::SetModifMode(ST_ModifMode mode)
{
	modifMode = mode;
	ReloadValues();
}

void ModificationWidget::ReloadValues()
{
    xAxisModify->clear();
    yAxisModify->clear();
    zAxisModify->clear();

	if(modifMode == ST_MODIF_SCALE)
	{
		xLabel->setText("Scale:");

		yLabel->setVisible(false);
		zLabel->setVisible(false);
		yAxisModify->setVisible(false);
		zAxisModify->setVisible(false);
	}
	else
	{
		xLabel->setText("X:");

		yLabel->setVisible(true);
		zLabel->setVisible(true);
		yAxisModify->setVisible(true);
		zAxisModify->setVisible(true);
	}

	if (nullptr != curScene)
	{
		auto selection = curScene->selectionSystem->GetSelection();
		if(selection.Size() > 0 && (modifMode == ST_MODIF_MOVE || modifMode == ST_MODIF_ROTATE || modifMode == ST_MODIF_SCALE))
		{
			xAxisModify->setEnabled(true);
			yAxisModify->setEnabled(true);
			zAxisModify->setEnabled(true);

			xAxisModify->showButtons(true);
			yAxisModify->showButtons(true);
			zAxisModify->showButtons(true);

			if(selection.Size() > 1)
			{
				groupMode = true;

				if(pivotMode == PivotRelative)
				{
					xAxisModify->setValue(0);
					yAxisModify->setValue(0);
					zAxisModify->setValue(0);
				}
				else
				{
					xAxisModify->showButtons(false);
					yAxisModify->showButtons(false);
					zAxisModify->showButtons(false);

					xAxisModify->clear();
					yAxisModify->clear();
					zAxisModify->clear();
				}
			}
			else
			{
				groupMode = false;

				if(pivotMode == PivotRelative)
				{
					xAxisModify->setValue(0);
					yAxisModify->setValue(0);
					zAxisModify->setValue(0);
				}
				else
				{
					DAVA::Entity *singleEntity = selection.GetEntity(0);
					if(NULL != singleEntity)
					{

						DAVA::float32 x = 0;
						DAVA::float32 y = 0;
						DAVA::float32 z = 0;

						DAVA::Matrix4 localMatrix = singleEntity->GetLocalTransform();
						switch (modifMode)
						{
						case ST_MODIF_MOVE:
							{
								DAVA::Vector3 translation = localMatrix.GetTranslationVector();
								x = translation.x;
								y = translation.y;
								z = translation.z;
							}
							break;
						case ST_MODIF_ROTATE:
							{
								DAVA::Vector3 pos, scale, rotate;
								if(localMatrix.Decomposition(pos, scale, rotate))
								{
									x = DAVA::RadToDeg(rotate.x);
									y = DAVA::RadToDeg(rotate.y);
									z = DAVA::RadToDeg(rotate.z);
								}
							}
							break;
						case ST_MODIF_SCALE:
							{
								DAVA::Vector3 pos, scale, rotate;
								if(localMatrix.Decomposition(pos, scale, rotate))
								{
									x = scale.x;
									y = scale.y;
									z = scale.z;
								}
							}
							break;
						default:
							break;
						}

						xAxisModify->setValue(x);
						yAxisModify->setValue(y);
						zAxisModify->setValue(z);
					}
				}
			}
		}
		else
		{
			xAxisModify->showButtons(true);
			yAxisModify->showButtons(true);
			zAxisModify->showButtons(true);

			xAxisModify->setEnabled(false);
			yAxisModify->setEnabled(false);
			zAxisModify->setEnabled(false);

			xAxisModify->clear();
			yAxisModify->clear();
			zAxisModify->clear();
		}
	}

    OnSnapToLandscapeChanged();
}

void ModificationWidget::ApplyValues(ST_Axis axis)
{
	switch (modifMode)
	{
	case ST_MODIF_MOVE:
		ApplyMoveValues(axis);
		break;
	case ST_MODIF_ROTATE:
		ApplyRotateValues(axis);
		break;
	case ST_MODIF_SCALE:
		ApplyScaleValues(axis);
		break;
	default:
		break;
	}

	ReloadValues();
}

void ModificationWidget::ApplyMoveValues(ST_Axis axis)
{
	DAVA::float32 x = xAxisModify->value();
	DAVA::float32 y = yAxisModify->value();
	DAVA::float32 z = zAxisModify->value();

	if(NULL != curScene)
	{
		EntityGroup selection = curScene->selectionSystem->GetSelection();
        const auto isSnappedToLandscape = curScene->modifSystem->GetLandscapeSnap();

		if(selection.Size() > 1)
		{
			curScene->BeginBatch("Multiple transform");
		}

		for (size_t i = 0; i < selection.Size(); ++i)
		{
			DAVA::Entity *entity = selection.GetEntity(i);
			DAVA::Matrix4 origMatrix = entity->GetLocalTransform();
			DAVA::Vector3 origPos = origMatrix.GetTranslationVector();
			DAVA::Vector3 newPos = origPos;

			if(pivotMode == PivotAbsolute)
			{
				switch (axis)
				{
				case ST_AXIS_X:
					newPos.x = x;
					break;
				case ST_AXIS_Y:
					newPos.y = y;
					break;
				case ST_AXIS_Z:
                    if ( !isSnappedToLandscape )
                    {
                        newPos.z = z;
                    }
					break;
				default:
					break;
				}
			}
			else
			{
				switch (axis)
				{
				case ST_AXIS_X:
					newPos.x += x;
					break;
				case ST_AXIS_Y:
					newPos.y += y;
					break;
				case ST_AXIS_Z:
                    if ( !isSnappedToLandscape )
                    {
                        newPos.z += z;
                    }
					break;
				default:
					break;
				}
			}

			DAVA::Matrix4 newMatrix = origMatrix;
			newMatrix.SetTranslationVector(newPos);

			curScene->Exec(new TransformCommand(entity,	origMatrix, newMatrix));
		}

		if(selection.Size() > 1)
		{
			curScene->EndBatch();
		}
	}
}

void ModificationWidget::ApplyRotateValues(ST_Axis axis)
{
	DAVA::float32 x = DAVA::DegToRad(xAxisModify->value());
	DAVA::float32 y = DAVA::DegToRad(yAxisModify->value());
	DAVA::float32 z = DAVA::DegToRad(zAxisModify->value());

	if(NULL != curScene)
	{
		EntityGroup selection = curScene->selectionSystem->GetSelection();

		if(selection.Size() > 1)
		{
			curScene->BeginBatch("Multiple transform");
		}

		for (size_t i = 0; i < selection.Size(); ++i)
		{
			DAVA::Entity *entity = selection.GetEntity(i);
			DAVA::Matrix4 origMatrix = entity->GetLocalTransform();

			DAVA::Vector3 pos, scale, rotate;
			if(origMatrix.Decomposition(pos, scale, rotate))
			{
				DAVA::Matrix4 newMatrix;
				DAVA::Matrix4 rotationMatrix;
				DAVA::Matrix4 transformMatrix;

				DAVA::Matrix4 moveToZeroPos;
				DAVA::Matrix4 moveFromZeroPos;

				moveToZeroPos.CreateTranslation(-origMatrix.GetTranslationVector());
				moveFromZeroPos.CreateTranslation(origMatrix.GetTranslationVector());

				if(pivotMode == PivotAbsolute)
				{
					switch (axis)
					{
					case ST_AXIS_X:
						rotationMatrix.CreateRotation(DAVA::Vector3(1, 0, 0), x - rotate.x);
						break;
					case ST_AXIS_Y:
						rotationMatrix.CreateRotation(DAVA::Vector3(0, 1, 0), y - rotate.y);
						break;
					case ST_AXIS_Z:
						rotationMatrix.CreateRotation(DAVA::Vector3(0, 0, 1), z - rotate.z);
						break;
					default:
						break;
					}
				}
				else
				{
					switch (axis)
					{
					case ST_AXIS_X:
						rotationMatrix.CreateRotation(DAVA::Vector3(1, 0, 0), x);
						break;
					case ST_AXIS_Y:
						rotationMatrix.CreateRotation(DAVA::Vector3(0, 1, 0), y);
						break;
					case ST_AXIS_Z:
						rotationMatrix.CreateRotation(DAVA::Vector3(0, 0, 1), z);
						break;
					default:
						break;
					}
				}

				newMatrix = origMatrix * moveToZeroPos * rotationMatrix * moveFromZeroPos;
				newMatrix.SetTranslationVector(origMatrix.GetTranslationVector());

				curScene->Exec(new TransformCommand(entity,	origMatrix, newMatrix));
			}
		}

		if(selection.Size() > 1)
		{
			curScene->EndBatch();
		}
	}
}

void ModificationWidget::ApplyScaleValues(ST_Axis axis)
{
	DAVA::float32 scaleValue = 1.0f;

	switch (axis)
	{
	case ST_AXIS_X:
		scaleValue = xAxisModify->value();
		break;
	case ST_AXIS_Y:
		scaleValue = yAxisModify->value();
		break;
	case ST_AXIS_Z:
		scaleValue = zAxisModify->value();
		break;
	default:
		break;
	}

	if(NULL != curScene)
	{
		EntityGroup selection = curScene->selectionSystem->GetSelection();

		if(selection.Size() > 1)
		{
			curScene->BeginBatch("Multiple transform");
		}

		for (size_t i = 0; i < selection.Size(); ++i)
		{
			DAVA::Entity *entity = selection.GetEntity(i);
			DAVA::Matrix4 origMatrix = entity->GetLocalTransform();

			DAVA::Vector3 pos, scale, rotate;
			if(origMatrix.Decomposition(pos, scale, rotate))
			{
				DAVA::Matrix4 newMatrix;
				DAVA::Matrix4 scaleMatrix;
				DAVA::Matrix4 transformMatrix;

				DAVA::Matrix4 moveToZeroPos;
				DAVA::Matrix4 moveFromZeroPos;

				moveToZeroPos.CreateTranslation(-origMatrix.GetTranslationVector());
				moveFromZeroPos.CreateTranslation(origMatrix.GetTranslationVector());

				DAVA::float32 newEntityScale;
				if(pivotMode == PivotAbsolute)
				{
					if(0 != scale.x)
					{
						newEntityScale = scaleValue / scale.x;
					}
					else
					{
						newEntityScale = 0;
					}
				}
				else
				{
					newEntityScale = scaleValue;
				}

				scaleMatrix.CreateScale(DAVA::Vector3(newEntityScale, newEntityScale, newEntityScale));
				newMatrix = origMatrix * moveToZeroPos * scaleMatrix * moveFromZeroPos;
				newMatrix.SetTranslationVector(origMatrix.GetTranslationVector());

				curScene->Exec(new TransformCommand(entity,	origMatrix, newMatrix));
			}
		}

		if(selection.Size() > 1)
		{
			curScene->EndBatch();
		}
	}
}

void ModificationWidget::OnSceneCommand(SceneEditor2 *scene, const Command2* command, bool redo)
{
	if(curScene == scene)
	{
		ReloadValues();
	}
}

void ModificationWidget::OnXChanged()
{
	ApplyValues(ST_AXIS_X);
}

void ModificationWidget::OnYChanged()
{
	ApplyValues(ST_AXIS_Y);
}

void ModificationWidget::OnZChanged()
{
	ApplyValues(ST_AXIS_Z);
}

void ModificationWidget::OnSnapToLandscapeChanged()
{
    if ( curScene == nullptr )
        return;

    auto selection = curScene->selectionSystem->GetSelection();
    if ( selection.Size() == 0 )
        return;

    const auto isSnappedToLandscape = curScene->modifSystem->GetLandscapeSnap();
    const auto isMoveMode = ( modifMode == ST_MODIF_MOVE );
    zAxisModify->setReadOnly( isSnappedToLandscape && isMoveMode );
}

void ModificationWidget::OnSceneActivated(SceneEditor2 *scene)
{
	curScene = scene;
	ReloadValues();
}

void ModificationWidget::OnSceneDeactivated(SceneEditor2 *scene)
{
	curScene = NULL;
	ReloadValues();
}

void ModificationWidget::OnSceneSelectionChanged(SceneEditor2 *scene, const EntityGroup *selected, const EntityGroup *deselected)
{
	if(curScene == scene)
	{
		ReloadValues();
	}
}

// =======================================================================================================
// ModificationSpin
// =======================================================================================================

DAVAFloat32SpinBox::DAVAFloat32SpinBox(QWidget *parent /* = 0 */)
	: QAbstractSpinBox(parent)
	, originalValue(0)
	, precision(3)
	, hasButtons(true)
	, cleared(true)
{
	QLineEdit *le = lineEdit();

	QRegExp rx("^-?\\d+([\\.,]\\d+){0,1}$");
	QValidator *vd = new QRegExpValidator(rx, this);

	le->setValidator(vd);

	connect(this, SIGNAL(editingFinished()), this, SLOT(textEditingFinished()));
}

DAVAFloat32SpinBox::~DAVAFloat32SpinBox()
{ }

DAVA::float32 DAVAFloat32SpinBox::value() const
{
	return originalValue;
}

void DAVAFloat32SpinBox::showButtons(bool show)
{
	hasButtons = show;
}

void DAVAFloat32SpinBox::clear()
{
	QAbstractSpinBox::clear();
	cleared = true;
}

void DAVAFloat32SpinBox::setValue(DAVA::float32 val)
{
	QLineEdit *le = lineEdit();

	if(originalValue != val || cleared)
	{
		cleared = false;

		originalString = QString::number((double) val, 'f', precision);
		le->setText(originalString);

		if(originalValue != val)
		{
			originalValue = val;
			emit valueChanged();
		}
	}
}

void DAVAFloat32SpinBox::stepBy(int steps)
{
	textEditingFinished();
	setValue(originalValue + (DAVA::float32) steps);
	selectAll();

	emit valueEdited();
}

void DAVAFloat32SpinBox::textEditingFinished()
{
	// was modified
	if(lineEdit()->isUndoAvailable())
	{
		QString newString = lineEdit()->text();
		newString.replace(QChar(','), QChar('.'));

		// current text is different from the originalText
		if(newString != originalString)
		{
			bool convertedNew = false;
			bool convertedOrig = false;

			double newValue = newString.toDouble(&convertedNew);
			double origValue = originalString.toDouble(&convertedOrig);

			// current double value is different from the original
			if(convertedNew && convertedOrig && newValue != origValue)
			{
				setValue(static_cast<DAVA::float32>(newValue));

				emit valueEdited();
			}
		}
	}
}

void DAVAFloat32SpinBox::keyPressEvent(QKeyEvent *event)
{
	switch (event->key()) 
	{
	case Qt::Key_Enter:
	case Qt::Key_Return:
		selectAll();
		event->ignore();
		emit editingFinished();
		return;

	default:
		QAbstractSpinBox::keyPressEvent(event);
	}
}

void DAVAFloat32SpinBox::paintEvent(QPaintEvent *event)
{
	QStyleOptionSpinBox opt;
	initStyleOption(&opt);

	// draw buttons disabled if they are
	opt.stepEnabled = stepEnabled();

	QStylePainter p(this);
	p.drawComplexControl(QStyle::CC_SpinBox, opt);
}

QAbstractSpinBox::StepEnabled DAVAFloat32SpinBox::stepEnabled() const
{
	if(hasButtons)
	{
		return (QAbstractSpinBox::StepDownEnabled | QAbstractSpinBox::StepUpEnabled);
	}

	return QAbstractSpinBox::StepNone;
}
