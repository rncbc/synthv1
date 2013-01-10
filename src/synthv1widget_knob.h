// synthv1widget_knob.h
//
/****************************************************************************
   Copyright (C) 2012-2013, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __synthv1widget_knob_h
#define __synthv1widget_knob_h

#include <QWidget>

// Forward declarations.
class QLabel;
class QDial;
class QSpinBox;
class QComboBox;


//-------------------------------------------------------------------------
// synthv1widget_knob - Custom composite widget.

class synthv1widget_knob : public QWidget
{
	Q_OBJECT

public:

	// Constructor.
	synthv1widget_knob(QWidget *pParent = 0);

	// Accessors.
	void setText(const QString& sText);
	QString text() const;

	float value() const;

	virtual void setMaximum(float fMaximum);
	float maximum() const;

	virtual void setMinimum(float fMinimum);
	float minimum() const;

	void setSingleStep(float fSingleStep);
	float singleStep() const;

	void resetDefaultValue();
	void setDefaultValue(float fDefaultValue);
	float defaultValue() const;

	virtual QString valueText() const;

	void setScale(float fScale);
	float scale() const;

public slots:

	virtual void setValue(float fValue);

signals:

	// Change signal.
	void valueChanged(float);

protected slots:

	// Change slot.
	void dialValueChanged(int);

protected:

	// Mouse behavior event handler.
	void mousePressEvent(QMouseEvent *pMouseEvent);

	// Scale/value converters.
	int scaleFromValue(float fValue) const;
	float valueFromScale(int iScale) const;

private:

	// Widget members.
	QLabel *m_pLabel;
	QDial  *m_pDial;

	// Default value.
	float m_fDefaultValue;
	int   m_iDefaultValue;

	// Scale multiplier (default=100).
	float m_fScale;
};


//-------------------------------------------------------------------------
// synthv1widget_spin - Custom knob/spin-box widget.

class synthv1widget_spin : public synthv1widget_knob
{
	Q_OBJECT

public:

	// Constructor.
	synthv1widget_spin(QWidget *pParent = 0);

	void setMaximum(float fMaximum);
	void setMinimum(float fMinimum);

	void setSpecialValueText(const QString& sText);
	QString specialValueText() const;

public slots:

	// Virtual accessors.
	void setValue(float fValue);

protected slots:

	// Change slot.
	void spinBoxValueChanged(int);

private:

	// Widget members.
	QSpinBox *m_pSpinBox;
};


//-------------------------------------------------------------------------
// synthv1widget_combo - Custom knob/combo-box widget.

class synthv1widget_combo : public synthv1widget_knob
{
	Q_OBJECT

public:

	// Constructor.
	synthv1widget_combo(QWidget *pParent = 0);

	// Accessors.
	void insertItems(int iIndex, const QStringList& items);
	void clear();

	QString valueText() const;

public slots:

	// Virtual accessor.
	void setValue(float fValue);

protected slots:

	// Change slot.
	void comboBoxValueChanged(int);

protected:

	// Reimplemented mouse-wheel stepping.
	void wheelEvent(QWheelEvent *pWheelEvent);

private:

	// Widget members.
	QComboBox *m_pComboBox;
};


#endif  // __synthv1widget_knob_h

// end of synthv1widget_knob.h
