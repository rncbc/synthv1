// synthv1widget_knob.h
//
/****************************************************************************
   Copyright (C) 2012-2015, rncbc aka Rui Nuno Capela. All rights reserved.

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
#include <QDial>


// Forward declarations.
class QLabel;
class QDoubleSpinBox;
class QComboBox;


//-------------------------------------------------------------------------
// synthv1widget_dial - A better QDial widget.

class synthv1widget_dial : public QDial
{
	Q_OBJECT

public:

	// Constructor.
	synthv1widget_dial(QWidget *pParent = 0);

	// Dial mode behavior:
	// DefaultMode - default (old) QDial behavior.
	// LinearMode  - proportionally to distance in one ortogonal axis.
	// AngularMode - angularly relative to widget center.
	enum DialMode { DefaultMode = 0, LinearMode, AngularMode };

	// Set knob dial mode behavior.
	static void setDialMode(DialMode dialMode);
	static DialMode dialMode();

protected:

	// Mouse angle determination.
	float mouseAngle(const QPoint& pos);

	// Alternate mouse behavior event handlers.
	void mousePressEvent(QMouseEvent *pMouseEvent);
	void mouseMoveEvent(QMouseEvent *pMouseEvent);
	void mouseReleaseEvent(QMouseEvent *pMouseEvent);

private:

	// Alternate mouse behavior tracking.
	bool   m_bMousePressed;
	QPoint m_posMouse;

	// Just for more precission on the movement
	float m_fLastDragValue;

	// Knob dial mode behavior.
	static DialMode g_dialMode;
};


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

	virtual void setMaximum(float fMaximum);
	float maximum() const;

	virtual void setMinimum(float fMinimum);
	float minimum() const;

	virtual void setSingleStep(float fSingleStep);
	float singleStep() const;

	void resetDefaultValue();
	void setDefaultValue(float fDefaultValue);
	float defaultValue() const;

	virtual QString valueText() const;
	virtual float value() const;

	void setScale(float fScale);
	float scale() const;

public slots:

	virtual void setValue(float fValue, bool bDefault = false);

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
	float scaleFromValue(float fValue) const;
	float valueFromScale(float fScale) const;

private:

	// Widget members.
	QLabel *m_pLabel;

	synthv1widget_dial *m_pDial;

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

	// Virtual accessors.
	void setMaximum(float fMaximum);
	void setMinimum(float fMinimum);

	void setSingleStep(float fSingleStep);

	QString valueText() const;
	float value() const;

	// Specialized accessors.
	void setSpecialValueText(const QString& sText);
	QString specialValueText() const;

	bool isSpecialValue() const;

	void setDecimals(int iDecimals);
	int decimals() const;

public slots:

	// Virtual accessor.
	void setValue(float fValue, bool bDefault = false);

protected slots:

	// Change slot.
	void spinBoxValueChanged(double);

private:

	// Widget members.
	QDoubleSpinBox *m_pSpinBox;
};


//-------------------------------------------------------------------------
// synthv1widget_combo - Custom knob/combo-box widget.

class synthv1widget_combo : public synthv1widget_knob
{
	Q_OBJECT

public:

	// Constructor.
	synthv1widget_combo(QWidget *pParent = 0);

	// Virtual accessors.
	QString valueText() const;
	float value() const;

	// Specialized accessors.
	void insertItems(int iIndex, const QStringList& items);
	void clear();

public slots:

	// Virtual accessor.
	void setValue(float fValue, bool bDefault = false);

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
