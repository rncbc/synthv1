// synthv1widget_knob.cpp
//
/****************************************************************************
   Copyright (C) 2012-2017, rncbc aka Rui Nuno Capela. All rights reserved.

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2
   of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, write to the Free Software Foundation, Inc.,
   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*****************************************************************************/

#include "synthv1widget_knob.h"

#include <QLabel>
#include <QSpinBox>
#include <QComboBox>

#include <QGridLayout>

#include <QMouseEvent>

#include <math.h>


// Integer value round.
inline int iroundf(float x) { return int(x < 0.0f ? x - 0.5f : x + 0.5f); }


//-------------------------------------------------------------------------
// synthv1widget_dial - A better QDial widget.

synthv1widget_dial::DialMode
synthv1widget_dial::g_dialMode = synthv1widget_dial::DefaultMode;

// Set knob dial mode behavior.
void synthv1widget_dial::setDialMode ( DialMode dialMode )
	{ g_dialMode = dialMode; }

synthv1widget_dial::DialMode synthv1widget_dial::dialMode (void)
	{ return g_dialMode; }


// Constructor.
synthv1widget_dial::synthv1widget_dial ( QWidget *pParent )
	: QDial(pParent), m_bMousePressed(false), m_fLastDragValue(0.0f)
{
}


// Mouse angle determination.
float synthv1widget_dial::mouseAngle ( const QPoint& pos )
{
	const float dx = pos.x() - (width() >> 1);
	const float dy = (height() >> 1) - pos.y();
	return 180.0f * ::atan2f(dx, dy) / float(M_PI);
}


// Alternate mouse behavior event handlers.
void synthv1widget_dial::mousePressEvent ( QMouseEvent *pMouseEvent )
{
	if (g_dialMode == DefaultMode) {
		QDial::mousePressEvent(pMouseEvent);
	} else if (pMouseEvent->button() == Qt::LeftButton) {
		m_bMousePressed = true;
		m_posMouse = pMouseEvent->pos();
		m_fLastDragValue = float(value());
		emit sliderPressed();
	}
}


void synthv1widget_dial::mouseMoveEvent ( QMouseEvent *pMouseEvent )
{
	if (g_dialMode == DefaultMode) {
		QDial::mouseMoveEvent(pMouseEvent);
		return;
	}

	if (!m_bMousePressed)
		return;

	const QPoint& pos = pMouseEvent->pos();
	const int dx = pos.x() - m_posMouse.x();
	const int dy = pos.y() - m_posMouse.y();
	float fAngleDelta =  mouseAngle(pos) - mouseAngle(m_posMouse);
	int iNewValue = value();

	switch (g_dialMode)	{
	case LinearMode:
		iNewValue = int(m_fLastDragValue) + dx - dy;
		break;
	case AngularMode:
	default:
		// Forget about the drag origin to be robust on full rotations
		if (fAngleDelta > +180.0f) fAngleDelta -= 360.0f;
		else
		if (fAngleDelta < -180.0f) fAngleDelta += 360.0f;
		m_fLastDragValue += float(maximum() - minimum()) * fAngleDelta / 270.0f;
		if (m_fLastDragValue > float(maximum()))
			m_fLastDragValue = float(maximum());
		else
		if (m_fLastDragValue < float(minimum()))
			m_fLastDragValue = float(minimum());
		m_posMouse = pos;
		iNewValue = int(m_fLastDragValue + 0.5f);
		break;
	}

	setValue(iNewValue);
	update();

	emit sliderMoved(value());
}


void synthv1widget_dial::mouseReleaseEvent ( QMouseEvent *pMouseEvent )
{
	if (g_dialMode == DefaultMode
		&& pMouseEvent->button() != Qt::MidButton) {
		QDial::mouseReleaseEvent(pMouseEvent);
	} else if (m_bMousePressed) {
		m_bMousePressed = false;
	}
}


//-------------------------------------------------------------------------
// synthv1widget_knob - Custom composite widget.
//

// Constructor.
synthv1widget_knob::synthv1widget_knob ( QWidget *pParent ) : QWidget(pParent)
{
	const QFont& font = QWidget::font();
	const QFont font2(font.family(), font.pointSize() - 2);
	QWidget::setFont(font2);

	m_pLabel = new QLabel();
	m_pLabel->setAlignment(Qt::AlignCenter);

	m_fValue = 0.0f;
	m_fScale = 100.0f;

	resetDefaultValue();

	m_pDial = new synthv1widget_dial();
	m_pDial->setNotchesVisible(true);
	m_pDial->setMaximumSize(QSize(48, 42));

	QGridLayout *pGridLayout = new QGridLayout();
	pGridLayout->setMargin(0);
	pGridLayout->setSpacing(0);
	pGridLayout->addWidget(m_pLabel, 0, 0, 1, 3);
	pGridLayout->addWidget(m_pDial,  1, 0, 1, 3);
	QWidget::setLayout(pGridLayout);

	QWidget::setMaximumSize(QSize(52, 72));

	QObject::connect(m_pDial,
		SIGNAL(valueChanged(int)),
		SLOT(dialValueChanged(int)));
}


void synthv1widget_knob::setText ( const QString& sText )
{
	m_pLabel->setText(sText);
}


QString synthv1widget_knob::text (void) const
{
	return m_pLabel->text();
}


void synthv1widget_knob::setValue ( float fValue, bool bDefault )
{
	QPalette pal;

	if (bDefault) {
		m_fDefaultValue = fValue;
		m_iDefaultValue++;
	}
	else
	if (QWidget::isEnabled()
		&& ::fabsf(fValue - m_fDefaultValue) > 0.0001f) {
		pal.setColor(QPalette::Base,
			(pal.window().color().value() < 0x7f
				? QColor(Qt::darkYellow).darker()
				: QColor(Qt::yellow).lighter()));
	}

	QWidget::setPalette(pal);

	if (::fabsf(fValue - m_fValue) > 0.0001f) {
		const bool bDialBlock = m_pDial->blockSignals(true);
		m_pDial->setValue(scaleFromValue(fValue));
		m_fValue = fValue;
		m_pDial->blockSignals(bDialBlock);
		emit valueChanged(m_fValue);
	}
}


float synthv1widget_knob::value (void) const
{
	return m_fValue;
}


QString synthv1widget_knob::valueText (void) const
{
	return QString::number(value());
}


void synthv1widget_knob::setMaximum ( float fMaximum )
{
	m_pDial->setMaximum(scaleFromValue(fMaximum));
}


float synthv1widget_knob::maximum (void) const
{
	return valueFromScale(m_pDial->maximum());
}


void synthv1widget_knob::setMinimum ( float fMinimum )
{
	m_pDial->setMinimum(scaleFromValue(fMinimum));
}


float synthv1widget_knob::minimum (void) const
{
	return valueFromScale(m_pDial->minimum());
}


void synthv1widget_knob::resetDefaultValue (void)
{
	m_fDefaultValue = 0.0f;
	m_iDefaultValue = 0;
}


bool synthv1widget_knob::isDefaultValue (void) const
{
	return (m_iDefaultValue > 0);
}


void synthv1widget_knob::setDefaultValue ( float fDefaultValue )
{
	m_fDefaultValue = fDefaultValue;
	m_iDefaultValue++;
}


float synthv1widget_knob::defaultValue (void) const
{
	return m_fDefaultValue;
}


void synthv1widget_knob::setSingleStep ( float fSingleStep )
{
	m_pDial->setSingleStep(scaleFromValue(fSingleStep));
}


float synthv1widget_knob::singleStep (void) const
{
	return valueFromScale(m_pDial->singleStep());
}


// Mouse behavior event handler.
void synthv1widget_knob::mousePressEvent ( QMouseEvent *pMouseEvent )
{
	if (pMouseEvent->button() == Qt::MidButton) {
		if (m_iDefaultValue < 1) {
			m_fDefaultValue = 0.5f * (maximum() + minimum());
			m_iDefaultValue++;
		}
		setValue(m_fDefaultValue);
	}

	QWidget::mousePressEvent(pMouseEvent);
}


// Internal widget slots.
void synthv1widget_knob::dialValueChanged ( int iDialValue )
{
	setValue(valueFromScale(iDialValue));
}



// Scale multiplier (default=100).
void synthv1widget_knob::setScale ( float fScale )
{
	m_fScale = fScale;

	m_pDial->setNotchTarget(valueFromScale(33.3f));
}


float synthv1widget_knob::scale (void) const
{
	return m_fScale;
}


// Scale/value converters.
float synthv1widget_knob::scaleFromValue ( float fValue ) const
{
	return (m_fScale * fValue);
}


float synthv1widget_knob::valueFromScale ( float fScale ) const
{
	return (fScale / m_fScale);
}


//-------------------------------------------------------------------------
// synthv1widget_spin - Custom knob/spin-box widget.
//

// Constructor.
synthv1widget_spin::synthv1widget_spin ( QWidget *pParent )
	: synthv1widget_knob(pParent)
{
	m_pSpinBox = new QDoubleSpinBox();
	m_pSpinBox->setAccelerated(true);
	m_pSpinBox->setAlignment(Qt::AlignCenter);

	const QFontMetrics fm(synthv1widget_knob::font());
	m_pSpinBox->setMaximumHeight(fm.height() + 6);

	QGridLayout *pGridLayout
		= static_cast<QGridLayout *> (QWidget::layout());
	pGridLayout->addWidget(m_pSpinBox, 2, 1, 1, 1);

	setMinimum(0.0f);
	setMaximum(1.0f);

	setDecimals(1);

	QObject::connect(m_pSpinBox,
		SIGNAL(valueChanged(double)),
		SLOT(spinBoxValueChanged(double)));
}


// Virtual accessors.
void synthv1widget_spin::setValue ( float fValue, bool bDefault )
{
	const bool bSpinBlock = m_pSpinBox->blockSignals(true);
	synthv1widget_knob::setValue(fValue, bDefault);
	m_pSpinBox->setValue(scaleFromValue(fValue));
	m_pSpinBox->blockSignals(bSpinBlock);
}


void synthv1widget_spin::setMaximum ( float fMaximum )
{
	m_pSpinBox->setMaximum(scaleFromValue(fMaximum));
	synthv1widget_knob::setMaximum(fMaximum);
}


void synthv1widget_spin::setMinimum ( float fMinimum )
{
	m_pSpinBox->setMinimum(scaleFromValue(fMinimum));
	synthv1widget_knob::setMinimum(fMinimum);
}


void synthv1widget_spin::setSingleStep ( float fSingleStep )
{
	m_pSpinBox->setSingleStep(fSingleStep);
	synthv1widget_knob::setSingleStep(fSingleStep);
}


QString synthv1widget_spin::valueText (void) const
{
	return QString::number(m_pSpinBox->value(), 'f', 1);
}


float synthv1widget_spin::value (void) const
{
	return valueFromScale(m_pSpinBox->value());
}


// Internal widget slots.
void synthv1widget_spin::spinBoxValueChanged ( double spinValue )
{
	synthv1widget_knob::setValue(valueFromScale(float(spinValue)));
}


// Special value text (minimum)
void synthv1widget_spin::setSpecialValueText ( const QString& sText )
{
	m_pSpinBox->setSpecialValueText(sText);
}


QString synthv1widget_spin::specialValueText (void) const
{
	return m_pSpinBox->specialValueText();
}


bool synthv1widget_spin::isSpecialValue (void) const
{
	return (m_pSpinBox->minimum() >= m_pSpinBox->value());
}


// Decimal digits allowed.
void synthv1widget_spin::setDecimals ( int iDecimals )
{
	m_pSpinBox->setDecimals(iDecimals);

	setSingleStep(::powf(10.0f, - float(iDecimals)));
}

int synthv1widget_spin::decimals (void) const
{
	return m_pSpinBox->decimals();
}


//-------------------------------------------------------------------------
// synthv1widget_combo - Custom knob/combo-box widget.
//

// Constructor.
synthv1widget_combo::synthv1widget_combo ( QWidget *pParent )
	: synthv1widget_knob(pParent)
{
	m_pComboBox = new QComboBox();

	const QFontMetrics fm(synthv1widget_knob::font());
	m_pComboBox->setMaximumHeight(fm.height() + 6);

	QGridLayout *pGridLayout
		= static_cast<QGridLayout *> (QWidget::layout());
	pGridLayout->addWidget(m_pComboBox, 2, 0, 1, 3);

	QObject::connect(m_pComboBox,
		SIGNAL(activated(int)),
		SLOT(comboBoxValueChanged(int)));
}


// Virtual accessors.
void synthv1widget_combo::setValue ( float fValue, bool bDefault )
{
	const bool bComboBlock = m_pComboBox->blockSignals(true);
	synthv1widget_knob::setValue(fValue, bDefault);
	m_pComboBox->setCurrentIndex(iroundf(fValue));
	m_pComboBox->blockSignals(bComboBlock);
}


QString synthv1widget_combo::valueText (void) const
{
	return m_pComboBox->currentText();
}


float synthv1widget_combo::value (void) const
{
	return float(m_pComboBox->currentIndex());
}


// Special combo-box mode accessors.
void synthv1widget_combo::insertItems ( int iIndex, const QStringList& items )
{
	m_pComboBox->insertItems(iIndex, items);

	setMinimum(0.0f);

	const int iItemCount = m_pComboBox->count();
	if (iItemCount > 0) {
		setMaximum(float(iItemCount - 1));
		setSingleStep(5.0f / float(iItemCount));
	} else {
		setMaximum(1.0f);
		setSingleStep(1.0f);
	}
}


void synthv1widget_combo::clear (void)
{
	m_pComboBox->clear();

	setMinimum(0.0f);
	setMaximum(1.0f);

	setSingleStep(1.0f);
}


// Internal widget slots.
void synthv1widget_combo::comboBoxValueChanged ( int iComboValue )
{
	synthv1widget_knob::setValue(float(iComboValue));
}


// Reimplemented mouse-wheel stepping.
void synthv1widget_combo::wheelEvent ( QWheelEvent *pWheelEvent )
{
	const int delta
		= (pWheelEvent->delta() / 120);
	if (delta) {
		float fValue = value() + float(delta);
		if (fValue < minimum())
			fValue = minimum();
		else
		if (fValue > maximum())
			fValue = maximum();
		setValue(fValue);
	}
}


// end of synthv1widget_knob.cpp
