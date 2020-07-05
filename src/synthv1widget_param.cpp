// synthv1widget_param.cpp
//
/****************************************************************************
   Copyright (C) 2012-2020, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "synthv1widget_param.h"

#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QRadioButton>
#include <QCheckBox>

#include <QGridLayout>
#include <QMouseEvent>

#include <math.h>


// Integer value round.
inline int iroundf(float x) { return int(x < 0.0f ? x - 0.5f : x + 0.5f); }


//-------------------------------------------------------------------------
// synthv1widget_param_style - Custom widget style.
//

#include <QProxyStyle>
#include <QPainter>
#include <QIcon>


class synthv1widget_param_style : public QProxyStyle
{
public:

	// Constructor.
	synthv1widget_param_style() : QProxyStyle()
	{
		m_icon.addPixmap(
			QPixmap(":/images/ledOff.png"), QIcon::Normal, QIcon::Off);
		m_icon.addPixmap(
			QPixmap(":/images/ledOn.png"), QIcon::Normal, QIcon::On);
	}


	// Hints override.
	int styleHint(StyleHint hint, const QStyleOption *option,
		const QWidget *widget, QStyleHintReturn *retdata) const
	{
		if (hint == QStyle::SH_UnderlineShortcut)
			return 0;
		else
			return QProxyStyle::styleHint(hint, option, widget, retdata);
	}

	// Paint job.
	void drawPrimitive(PrimitiveElement element,
		const QStyleOption *option,
		QPainter *painter, const QWidget *widget) const
	{
		if (element == PE_IndicatorRadioButton ||
			element == PE_IndicatorCheckBox) {
			const QRect& rect = option->rect;
			if (option->state & State_Enabled) {
				if (option->state & State_On)
					m_icon.paint(painter, rect,
						Qt::AlignCenter, QIcon::Normal, QIcon::On);
				else
			//	if (option->state & State_Off)
					m_icon.paint(painter, rect,
						Qt::AlignCenter, QIcon::Normal, QIcon::Off);
			} else {
				m_icon.paint(painter, rect,
					Qt::AlignCenter, QIcon::Disabled, QIcon::Off);
			}
		}
		else
		QProxyStyle::drawPrimitive(element, option, painter, widget);
	}

	// Spiced up text margins
	void drawItemText(QPainter *painter, const QRect& rectangle,
		int alignment, const QPalette& palette, bool enabled,
		const QString& text, QPalette::ColorRole textRole) const
	{
		QRect rect = rectangle;
		rect.setLeft(rect.left() - 4);
		rect.setRight(rect.right() + 4);
		QProxyStyle::drawItemText(painter, rect,
			alignment, palette, enabled, text, textRole);
	}

	static void addRef ()
		{ if (++g_iRefCount == 1) g_pStyle = new synthv1widget_param_style(); }

	static void releaseRef ()
		{ if (--g_iRefCount == 0) { delete g_pStyle; g_pStyle = nullptr; } }

	static synthv1widget_param_style *getRef ()
		{ return g_pStyle; }

private:

	QIcon m_icon;

	static synthv1widget_param_style *g_pStyle;
	static unsigned int g_iRefCount;
};


synthv1widget_param_style *synthv1widget_param_style::g_pStyle = nullptr;
unsigned int synthv1widget_param_style::g_iRefCount = 0;


//-------------------------------------------------------------------------
// synthv1widget_param - Custom composite widget.
//

// Constructor.
synthv1widget_param::synthv1widget_param ( QWidget *pParent ) : QWidget(pParent)
{
	const QFont& font = QWidget::font();
	const QFont font2(font.family(), font.pointSize() - 2);
	QWidget::setFont(font2);

	m_fValue = 0.0f;

	m_fMinimum = 0.0f;
	m_fMaximum = 1.0f;

	m_fScale = 1.0f;

	resetDefaultValue();

	QWidget::setMaximumSize(QSize(52, 72));

	QGridLayout *pGridLayout = new QGridLayout();
	pGridLayout->setContentsMargins(0, 0, 0, 0);
	pGridLayout->setSpacing(0);
	QWidget::setLayout(pGridLayout);
}


// Accessors.
void synthv1widget_param::setText ( const QString& sText )
{
	setValue(sText.toFloat());
}


QString synthv1widget_param::text (void) const
{
	return QString::number(value());
}


void synthv1widget_param::setValue ( float fValue )
{
	QPalette pal;

	if (m_iDefaultValue == 0) {
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
		m_fValue = fValue;
		emit valueChanged(m_fValue);
	}
}


float synthv1widget_param::value (void) const
{
	return m_fValue;
}


QString synthv1widget_param::valueText (void) const
{
	return QString::number(value());
}


void synthv1widget_param::setMaximum ( float fMaximum )
{
	m_fMaximum = fMaximum;
}

float synthv1widget_param::maximum (void) const
{
	return m_fMaximum;
}


void synthv1widget_param::setMinimum ( float fMinimum )
{
	m_fMinimum = fMinimum;
}

float synthv1widget_param::minimum (void) const
{
	return m_fMinimum;
}


void synthv1widget_param::resetDefaultValue (void)
{
	m_fDefaultValue = 0.0f;
	m_iDefaultValue = 0;
}


bool synthv1widget_param::isDefaultValue (void) const
{
	return (m_iDefaultValue > 0);
}


void synthv1widget_param::setDefaultValue ( float fDefaultValue )
{
	m_fDefaultValue = fDefaultValue;
	m_iDefaultValue++;
}


float synthv1widget_param::defaultValue (void) const
{
	return m_fDefaultValue;
}


// Mouse behavior event handler.
void synthv1widget_param::mousePressEvent ( QMouseEvent *pMouseEvent )
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


// Scale multiplier accessors.
void synthv1widget_param::setScale ( float fScale )
{
	m_fScale = fScale;
}


float synthv1widget_param::scale (void) const
{
	return m_fScale;
}


// Scale/value converters.
float synthv1widget_param::scaleFromValue ( float fValue ) const
{
	return (m_fScale * fValue);
}


float synthv1widget_param::valueFromScale ( float fScale ) const
{
	return (fScale / m_fScale);
}


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
// synthv1widget_knob - Custom knob/dial widget.
//

// Constructor.
synthv1widget_knob::synthv1widget_knob ( QWidget *pParent ) : synthv1widget_param(pParent)
{
	m_pLabel = new QLabel();
	m_pLabel->setAlignment(Qt::AlignCenter);

	m_pDial = new synthv1widget_dial();
	m_pDial->setNotchesVisible(true);
	m_pDial->setMaximumSize(QSize(48, 48));

	QGridLayout *pGridLayout
		= static_cast<QGridLayout *> (synthv1widget_param::layout());
	pGridLayout->addWidget(m_pLabel, 0, 0, 1, 3);
	pGridLayout->addWidget(m_pDial,  1, 0, 1, 3);
	pGridLayout->setAlignment(m_pDial, Qt::AlignVCenter | Qt::AlignHCenter);

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


void synthv1widget_knob::setValue ( float fValue )
{
	const bool bDialBlock = m_pDial->blockSignals(true);
	synthv1widget_param::setValue(fValue);
	m_pDial->setValue(scaleFromValue(fValue));
	m_pDial->blockSignals(bDialBlock);
}


void synthv1widget_knob::setMaximum ( float fMaximum )
{
	synthv1widget_param::setMaximum(fMaximum);
	m_pDial->setMaximum(scaleFromValue(fMaximum));
}


void synthv1widget_knob::setMinimum ( float fMinimum )
{
	synthv1widget_param::setMinimum(fMinimum);
	m_pDial->setMinimum(scaleFromValue(fMinimum));
}


// Scale-step accessors.
void synthv1widget_knob::setSingleStep ( float fSingleStep )
{
	m_pDial->setSingleStep(scaleFromValue(fSingleStep));
}


float synthv1widget_knob::singleStep (void) const
{
	return valueFromScale(m_pDial->singleStep());
}


// Dial change slot.
void synthv1widget_knob::dialValueChanged ( int iDialValue )
{
	setValue(valueFromScale(iDialValue));
}


//-------------------------------------------------------------------------
// synthv1widget_edit - A better QDoubleSpinBox widget.

synthv1widget_edit::EditMode
synthv1widget_edit::g_editMode = synthv1widget_edit::DefaultMode;

// Set spin-box edit mode behavior.
void synthv1widget_edit::setEditMode ( EditMode editMode )
	{ g_editMode = editMode; }

synthv1widget_edit::EditMode synthv1widget_edit::editMode (void)
	{ return g_editMode; }


// Constructor.
synthv1widget_edit::synthv1widget_edit ( QWidget *pParent )
	: QDoubleSpinBox(pParent), m_iTextChanged(0)
{
	QObject::connect(QDoubleSpinBox::lineEdit(),
		SIGNAL(textChanged(const QString&)),
		SLOT(lineEditTextChanged(const QString&)));
	QObject::connect(this,
		SIGNAL(editingFinished()),
		SLOT(spinBoxEditingFinished()));
	QObject::connect(this,
		SIGNAL(valueChanged(double)),
		SLOT(spinBoxValueChanged(double)));
}


// Alternate value change behavior handlers.
void synthv1widget_edit::lineEditTextChanged ( const QString& )
{
	if (g_editMode == DeferredMode)
		++m_iTextChanged;
}


void synthv1widget_edit::spinBoxEditingFinished (void)
{
	if (g_editMode == DeferredMode) {
		m_iTextChanged = 0;
		emit valueChangedEx(QDoubleSpinBox::value());
	}
}


void synthv1widget_edit::spinBoxValueChanged ( double spinValue )
{
	if (g_editMode != DeferredMode || m_iTextChanged == 0)
		emit valueChangedEx(spinValue);
}


// Inherited/override methods.
QValidator::State synthv1widget_edit::validate ( QString& sText, int& iPos ) const
{
	const QValidator::State state
		= QDoubleSpinBox::validate(sText, iPos);

	if (state == QValidator::Acceptable
		&& g_editMode == DeferredMode
		&& m_iTextChanged == 0)
		return QValidator::Intermediate;

	return state;
}


//-------------------------------------------------------------------------
// synthv1widget_spin - Custom knob/spin-box widget.
//

// Constructor.
synthv1widget_spin::synthv1widget_spin ( QWidget *pParent )
	: synthv1widget_knob(pParent)
{
	m_pSpinBox = new synthv1widget_edit();
	m_pSpinBox->setAccelerated(true);
	m_pSpinBox->setAlignment(Qt::AlignCenter);

	const QFontMetrics fm(synthv1widget_knob::font());
	m_pSpinBox->setMaximumHeight(fm.height() + 6);

	QGridLayout *pGridLayout
		= static_cast<QGridLayout *> (synthv1widget_knob::layout());
	pGridLayout->addWidget(m_pSpinBox, 2, 1, 1, 1);

	setScale(100.0f);

	setMinimum(0.0f);
	setMaximum(1.0f);

	setDecimals(1);

	QObject::connect(m_pSpinBox,
		SIGNAL(valueChangedEx(double)),
		SLOT(spinBoxValueChanged(double)));
}


// Virtual accessors.
void synthv1widget_spin::setValue ( float fValue )
{
	const bool bSpinBlock = m_pSpinBox->blockSignals(true);
	synthv1widget_knob::setValue(fValue);
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


QString synthv1widget_spin::valueText (void) const
{
	return QString::number(m_pSpinBox->value(), 'f', 1);
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
	m_pSpinBox->setSingleStep(::powf(10.0f, - float(iDecimals)));

	setSingleStep(0.1f);
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
		= static_cast<QGridLayout *> (synthv1widget_knob::layout());
	pGridLayout->addWidget(m_pComboBox, 2, 0, 1, 3);

//	setScale(1.0f);

	QObject::connect(m_pComboBox,
		SIGNAL(activated(int)),
		SLOT(comboBoxValueChanged(int)));
}


// Virtual accessors.
void synthv1widget_combo::setValue ( float fValue )
{
	const bool bComboBlock = m_pComboBox->blockSignals(true);
	synthv1widget_knob::setValue(fValue);
	m_pComboBox->setCurrentIndex(iroundf(fValue));
	m_pComboBox->blockSignals(bComboBlock);
}


QString synthv1widget_combo::valueText (void) const
{
	return m_pComboBox->currentText();
}


// Special combo-box mode accessors.
void synthv1widget_combo::insertItems ( int iIndex, const QStringList& items )
{
	m_pComboBox->insertItems(iIndex, items);

	setMinimum(0.0f);

	const int iItemCount = m_pComboBox->count();
	if (iItemCount > 0)
		setMaximum(float(iItemCount - 1));
	else
		setMaximum(1.0f);

	setSingleStep(1.0f);
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
		= (pWheelEvent->angleDelta().y() / 120);
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


//-------------------------------------------------------------------------
// synthv1widget_radio - Custom radio/button widget.
//

// Constructor.
synthv1widget_radio::synthv1widget_radio ( QWidget *pParent )
	: synthv1widget_param(pParent), m_group(this)
{
	synthv1widget_param_style::addRef();
#if 0
	synthv1widget_param::setStyleSheet(
	//	"QRadioButton::indicator { width: 16px; height: 16px; }"
		"QRadioButton::indicator::unchecked { image: url(:/images/ledOff.png); }"
		"QRadioButton::indicator::checked   { image: url(:/images/ledOn.png);  }"
	);
#endif

	QObject::connect(&m_group,
	#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
		SIGNAL(idClicked(int)),
	#else
		SIGNAL(buttonClicked(int)),
	#endif
		SLOT(radioGroupValueChanged(int)));
}


// Destructor.
synthv1widget_radio::~synthv1widget_radio (void)
{
	synthv1widget_param_style::releaseRef();
}


// Virtual accessors.
void synthv1widget_radio::setValue ( float fValue )
{
	const int iRadioValue = iroundf(fValue);
	QRadioButton *pRadioButton
		= static_cast<QRadioButton *> (m_group.button(iRadioValue));
	if (pRadioButton) {
		const bool bRadioBlock = pRadioButton->blockSignals(true);
		synthv1widget_param::setValue(float(iRadioValue));
		pRadioButton->setChecked(true);
		pRadioButton->blockSignals(bRadioBlock);
	}
}


QString synthv1widget_radio::valueText (void) const
{
	QString sValueText;
	const int iRadioValue = iroundf(value());
	QRadioButton *pRadioButton
		= static_cast<QRadioButton *> (m_group.button(iRadioValue));
	if (pRadioButton)
		sValueText = pRadioButton->text();
	return sValueText;
}


// Special combo-box mode accessors.
void synthv1widget_radio::insertItems ( int iIndex, const QStringList& items )
{
	const QFont& font = synthv1widget_param::font();
	const QFont font1(font.family(), font.pointSize() - 1);

	QGridLayout *pGridLayout
		= static_cast<QGridLayout *> (synthv1widget_param::layout());
	const QString sToolTipMask(synthv1widget_param::toolTip() + ": %1");
	QStringListIterator iter(items);
	while (iter.hasNext()) {
		const QString& sValueText = iter.next();
		QRadioButton *pRadioButton = new QRadioButton(sValueText);
		pRadioButton->setStyle(synthv1widget_param_style::getRef());
		pRadioButton->setFont(font1);
		pRadioButton->setToolTip(sToolTipMask.arg(sValueText));
		pGridLayout->addWidget(pRadioButton, iIndex, 0);
		m_group.addButton(pRadioButton, iIndex);
		++iIndex;
	}

	setMinimum(0.0f);

	const QList<QAbstractButton *> list = m_group.buttons();
	const int iRadioCount = list.count();
	if (iRadioCount > 0)
		setMaximum(float(iRadioCount - 1));
	else
		setMaximum(1.0f);
}


void synthv1widget_radio::clear (void)
{
	const QList<QAbstractButton *> list = m_group.buttons();
	QListIterator<QAbstractButton *> iter(list);
	while (iter.hasNext()) {
		QRadioButton *pRadioButton
			= static_cast<QRadioButton *> (iter.next());
		if (pRadioButton)
			m_group.removeButton(pRadioButton);
	}

	setMinimum(0.0f);
	setMaximum(1.0f);
}


void synthv1widget_radio::radioGroupValueChanged ( int iRadioValue )
{
	synthv1widget_param::setValue(float(iRadioValue));
}


//-------------------------------------------------------------------------
// synthv1widget_check - Custom check-box widget.
//

// Constructor.
synthv1widget_check::synthv1widget_check ( QWidget *pParent )
	: synthv1widget_param(pParent)
{
	synthv1widget_param_style::addRef();
#if 0
	synthv1widget_param::setStyleSheet(
	//	"QCheckBox::indicator { width: 16px; height: 16px; }"
		"QCheckBox::indicator::unchecked { image: url(:/images/ledOff.png); }"
		"QCheckBox::indicator::checked   { image: url(:/images/ledOn.png);  }"
	);
#endif
	m_pCheckBox = new QCheckBox();
	m_pCheckBox->setStyle(synthv1widget_param_style::getRef());

	m_alignment = Qt::AlignHCenter | Qt::AlignVCenter;

	QGridLayout *pGridLayout
		= static_cast<QGridLayout *> (synthv1widget_param::layout());
	pGridLayout->addWidget(m_pCheckBox, 0, 0);
	pGridLayout->setAlignment(m_pCheckBox, m_alignment);

	synthv1widget_param::setMaximumSize(QSize(72, 72));

	QObject::connect(m_pCheckBox,
		SIGNAL(toggled(bool)),
		SLOT(checkBoxValueChanged(bool)));
}


// Destructor.
synthv1widget_check::~synthv1widget_check (void)
{
	synthv1widget_param_style::releaseRef();
}


// Accessors.
void synthv1widget_check::setText ( const QString& sText )
{
	m_pCheckBox->setText(sText);
}


QString synthv1widget_check::text (void) const
{
	return m_pCheckBox->text();
}


void synthv1widget_check::setAlignment ( Qt::Alignment alignment )
{
	m_alignment = alignment;

	QLayout *pLayout = synthv1widget_param::layout();
	if (pLayout)
		pLayout->setAlignment(m_pCheckBox, m_alignment);
}


Qt::Alignment synthv1widget_check::alignment (void) const
{
	return m_alignment;
}


// Virtual accessors.
void synthv1widget_check::setValue ( float fValue )
{
	const bool bCheckValue = (fValue > 0.5f * (maximum() + minimum()));
	const bool bCheckBlock = m_pCheckBox->blockSignals(true);
	synthv1widget_param::setValue(bCheckValue ? maximum() : minimum());
	m_pCheckBox->setChecked(bCheckValue);
	m_pCheckBox->blockSignals(bCheckBlock);
}


void synthv1widget_check::checkBoxValueChanged ( bool bCheckValue )
{
	synthv1widget_param::setValue(bCheckValue ? maximum() : minimum());
}


//-------------------------------------------------------------------------
// synthv1widget_group - Custom checkable group-box widget.
//

// Constructor.
synthv1widget_group::synthv1widget_group ( QWidget *pParent )
	: QGroupBox(pParent)
{
	synthv1widget_param_style::addRef();
#if 0
	QGroupBox::setStyleSheet(
	//	"QGroupBox::indicator { width: 16px; height: 16px; }"
		"QGroupBox::indicator::unchecked { image: url(:/images/ledOff.png); }"
		"QGroupBox::indicator::checked   { image: url(:/images/ledOn.png);  }"
		);
#endif
	QGroupBox::setStyle(synthv1widget_param_style::getRef());

	m_pParam = new synthv1widget_param(this);
	m_pParam->setToolTip(QGroupBox::toolTip());
	m_pParam->setValue(0.5f); // HACK: half-way on.

	QObject::connect(m_pParam,
		 SIGNAL(valueChanged(float)),
		 SLOT(paramValueChanged(float)));

	QObject::connect(this,
		 SIGNAL(toggled(bool)),
		 SLOT(groupBoxValueChanged(bool)));
}


// Destructor.
synthv1widget_group::~synthv1widget_group (void)
{
	synthv1widget_param_style::releaseRef();

	delete m_pParam;
}


// Accessors.
void synthv1widget_group::setToolTip ( const QString& sToolTip )
{
	m_pParam->setToolTip(sToolTip);
}


QString synthv1widget_group::toolTip (void) const
{
	return m_pParam->toolTip();
}


synthv1widget_param *synthv1widget_group::param (void) const
{
	return m_pParam;
}


// Virtual accessors.
void synthv1widget_group::paramValueChanged ( float fValue )
{
	const float fMaximum = m_pParam->maximum();
	const float fMinimum = m_pParam->minimum();

	const bool bGroupValue = (fValue > 0.5f * (fMaximum + fMinimum));
	const bool bGroupBlock = QGroupBox::blockSignals(true);
	QGroupBox::setChecked(bGroupValue);
	QGroupBox::blockSignals(bGroupBlock);
}


void synthv1widget_group::groupBoxValueChanged ( bool bGroupValue )
{
	const float fMaximum = m_pParam->maximum();
	const float fMinimum = m_pParam->minimum();

	m_pParam->setValue(bGroupValue ? fMaximum : fMinimum);
}


// end of synthv1widget_param.cpp
