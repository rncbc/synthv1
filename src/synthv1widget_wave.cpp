// synthv1widget_wave.cpp
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

#include "synthv1widget_wave.h"

#include "synthv1_wave.h"

#include <QPainter>
#include <QPainterPath>
#include <QMouseEvent>
#include <QWheelEvent>

#include <math.h>


// Safe value capping.
inline float safe_value ( float x )
{
	return (x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x));
}


//----------------------------------------------------------------------------
// synthv1widget_wave -- Custom widget

// Constructor.
synthv1widget_wave::synthv1widget_wave ( QWidget *pParent )
	: QFrame(pParent),
		m_bDragging(false), m_iDragShape(0)
{
	m_pWave = new synthv1_wave_lf(128);

	setFixedSize(QSize(60, 60));

	QFrame::setFrameShape(QFrame::Panel);
	QFrame::setFrameShadow(QFrame::Sunken);
}


// Destructor.
synthv1widget_wave::~synthv1widget_wave (void)
{
	delete m_pWave;
}


// Parameter accessors.
void synthv1widget_wave::setWaveShape ( float fWaveShape )
{
	int iWaveShape = int(fWaveShape);
	if (iWaveShape != int(m_pWave->shape())) {
		if (iWaveShape < int(synthv1_wave::Pulse))
			iWaveShape = int(synthv1_wave::Noise);
		else
		if (iWaveShape > int(synthv1_wave::Noise))
			iWaveShape = int(synthv1_wave::Pulse);
		m_pWave->reset(synthv1_wave::Shape(iWaveShape), m_pWave->width());
		update();
		emit waveShapeChanged(waveShape());
	}
}

float synthv1widget_wave::waveShape (void) const
{
	return float(m_pWave->shape());
}


void synthv1widget_wave::setWaveWidth ( float fWaveWidth )
{
	if (::fabsf(fWaveWidth - m_pWave->width()) > 0.001f) {
		m_pWave->reset(m_pWave->shape(), safe_value(fWaveWidth));
		update();
		emit waveWidthChanged(waveWidth());
	}
}

float synthv1widget_wave::waveWidth (void) const
{
	return m_pWave->width();
}


// Draw curve.
void synthv1widget_wave::paintEvent ( QPaintEvent *pPaintEvent )
{
	QPainter painter(this);

	const QRect& rect = QWidget::rect();
	const int h  = rect.height();
	const int w  = rect.width();

	const int h2 = (h >> 1);
	const int w2 = (w >> 1);

	QPainterPath path;
	path.moveTo(0, h2);
	for (int x = 1; x < w; ++x)
		path.lineTo(x, h2 - int(m_pWave->value(float(x) / float(w)) * float(h2 - 2)));
	path.lineTo(w, h2);

	const QPalette& pal = palette();
	const bool bDark = (pal.window().color().value() < 0x7f);
	const QColor& rgbLite = (isEnabled()
		? (bDark ? Qt::darkYellow : Qt::yellow) : pal.mid().color());
    const QColor& rgbDark = pal.window().color().darker(180);

	painter.fillRect(rect, rgbDark);

	painter.setPen(bDark ? pal.mid().color() : pal.midlight().color());
	painter.drawLine(w2, 0, w2, h);
	painter.drawLine(0, h2, w, h2);

	painter.setRenderHint(QPainter::Antialiasing, true);

	painter.setPen(QPen(rgbLite.darker(), 2));
	path.translate(+1, +1);
	painter.drawPath(path);
	painter.setPen(QPen(rgbLite, 2));
	path.translate(-1, -1);
	painter.drawPath(path);

	painter.setRenderHint(QPainter::Antialiasing, false);

	painter.end();

	QFrame::paintEvent(pPaintEvent);
}


// Drag/move curve.
void synthv1widget_wave::dragCurve ( const QPoint& pos )
{
	const int h  = height();
	const int w  = width();

	const int dx = (pos.x() - m_posDrag.x());
	const int dy = (pos.y() - m_posDrag.y());

	if (dx || dy) {
		const int x = int(waveWidth() * float(w));
		setWaveWidth(float(x + dx) / float(w));
		const int h2 = (h >> 1);
		m_iDragShape += dy;
		if (m_iDragShape > +h2) {
			setWaveShape(waveShape() - 1);
			m_iDragShape = 0;
		}
		else
		if (m_iDragShape < -h2) {
			setWaveShape(waveShape() + 1);
			m_iDragShape = 0;
		}
		m_posDrag = pos;
	}
}


// Mouse interaction.
void synthv1widget_wave::mousePressEvent ( QMouseEvent *pMouseEvent )
{
	if (pMouseEvent->button() == Qt::LeftButton)
		m_posDrag = pMouseEvent->pos();

	QFrame::mousePressEvent(pMouseEvent);
}


void synthv1widget_wave::mouseMoveEvent ( QMouseEvent *pMouseEvent )
{
	const QPoint& pos = pMouseEvent->pos();
	if (m_bDragging) {
		dragCurve(pos);
	} else if ((pos - m_posDrag).manhattanLength() > 4) {
		setCursor(Qt::SizeAllCursor);
		m_bDragging = true;
		m_iDragShape = 0;
	}
}


void synthv1widget_wave::mouseReleaseEvent ( QMouseEvent *pMouseEvent )
{
	QFrame::mouseReleaseEvent(pMouseEvent);

	if (m_bDragging) {
		dragCurve(pMouseEvent->pos());
		m_bDragging = false;
		unsetCursor();
	}
}


void synthv1widget_wave::mouseDoubleClickEvent ( QMouseEvent *pMouseEvent )
{
	QFrame::mouseDoubleClickEvent(pMouseEvent);

	if (!m_bDragging)
		setWaveShape(waveShape() + 1);
}


void synthv1widget_wave::wheelEvent ( QWheelEvent *pWheelEvent )
{
	const int delta = (pWheelEvent->angleDelta().y() / 60);

	if (pWheelEvent->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) {
		setWaveShape(waveShape() + (delta < 0 ? -1 : +1));
	} else {
		const float w2 = float(width() >> 1);
		const int x = int(waveWidth() * w2);
		setWaveWidth(float(x + delta) / w2);
	}
}


// end of synthv1widget_wave.cpp
