// synthv1widget_filt.cpp
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

#include "synthv1widget_filt.h"

#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>

#include <math.h>


// Safe value capping.
inline float safe_value ( float x )
{
	return (x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x));
}


//----------------------------------------------------------------------------
// synthv1widget_filt -- Custom widget

// Constructor.
synthv1widget_filt::synthv1widget_filt (
	QWidget *pParent, Qt::WindowFlags wflags )
	: QFrame(pParent, wflags),
		m_fCutoff(0.0f), m_fReso(0.0f), m_fType(0.0f), m_fSlope(0.0f),
		m_bDragging(false)
{
//	setMouseTracking(true);
	setMinimumSize(QSize(180, 60));

	QFrame::setFrameShape(QFrame::Panel);
	QFrame::setFrameShadow(QFrame::Sunken);
}


// Destructor.
synthv1widget_filt::~synthv1widget_filt (void)
{
}


// Parameter accessors.
void synthv1widget_filt::setCutoff ( float fCutoff )
{
	if (::fabs(m_fCutoff - fCutoff) > 0.001f) {
		m_fCutoff = safe_value(fCutoff);
		update();
		emit cutoffChanged(cutoff());
	}
}

float synthv1widget_filt::cutoff (void) const
{
	return m_fCutoff;
}


void synthv1widget_filt::setReso ( float fReso )
{
	if (::fabs(m_fReso - fReso) > 0.001f) {
		m_fReso = safe_value(fReso);
		update();
		emit resoChanged(reso());
	}
}

float synthv1widget_filt::reso (void) const
{
	return m_fReso;
}


void synthv1widget_filt::setType ( float fType )
{
	if (::fabs(m_fType - fType) > 0.001f) {
		m_fType = fType;
		update();
	}
}

float synthv1widget_filt::type (void) const
{
	return m_fType;
}


void synthv1widget_filt::setSlope ( float fSlope )
{
	if (::fabs(m_fSlope - fSlope) > 0.001f) {
		m_fSlope = fSlope;
		update();
	}
}

float synthv1widget_filt::slope (void) const
{
	return m_fSlope;
}


// Draw curve.
void synthv1widget_filt::paintEvent ( QPaintEvent *pPaintEvent )
{
	QPainter painter(this);

	const QRect& rect = QWidget::rect();
	const int h  = rect.height();
	const int w  = rect.width();

	const int h2 = h >> 1;
	const int h4 = h >> 2;
	const int w4 = w >> 2;
	const int w8 = w >> 3;

	const int iSlope = int(m_fSlope);
	const int ws = w8 - (w8 >> 1) * (iSlope == 1 ? 1 : 0);

	int x = w8 + int(m_fCutoff * float(w - w4));
	int y = h2 - int(m_fReso * float(h + h4));

	QPolygon poly(6);
	QPainterPath path;

	const int iType = (iSlope == 3 ? 4 : int(m_fType));
	// Low, Notch
	if (iType == 0 || iType == 3) {
		if (iType == 3) x -= w8;
		poly.putPoints(0, 6,
			0, h2,
			x - w8, h2,
			x, h2,
			x, y,
			x + ws, h,
			0, h);
		path.moveTo(poly.at(0));
		path.lineTo(poly.at(1));
		path.cubicTo(poly.at(2), poly.at(3), poly.at(4));
		path.lineTo(poly.at(5));
		if (iType == 3) x += w8;
	}
	// Band
	if (iType == 1) {
		const int y2 = (y + h4) >> 1;
		poly.putPoints(0, 6,
			0, h,
			x - w8 - ws, h,
			x - ws, y2,
			x + ws, y2,
			x + w8 + ws, h,
			0, h);
		path.moveTo(poly.at(0));
		path.lineTo(poly.at(1));
		path.cubicTo(poly.at(2), poly.at(3), poly.at(4));
		path.lineTo(poly.at(5));
	}
	// High, Notch
	if (iType == 2 || iType == 3) {
		if (iType == 3) { x += w8; y = h2; }
		poly.putPoints(0, 6,
			x - ws, h,
			x, y,
			x, h2,
			x + w8, h2,
			w, h2,
			w, h);
		path.moveTo(poly.at(0));
		path.cubicTo(poly.at(1), poly.at(2), poly.at(3));
		path.lineTo(poly.at(4));
		path.lineTo(poly.at(5));
		if (iType == 3) x -= w8;
	}
	// Formant
	if (iType == 4) {
		const int x2 = (x - w4) >> 1;
		const int y2 = (y - h4) >> 1;
		poly.putPoints(0, 6,
			0, h2,
			x2, h2,
			x, h2,
			x, y2,
			x + ws, h,
			0, h);
		path.moveTo(poly.at(0));
		path.lineTo(poly.at(1));
		path.cubicTo(poly.at(2), poly.at(3), poly.at(4));
		path.lineTo(poly.at(5));
	}

	const QPalette& pal = palette();
	const bool bDark = (pal.window().color().value() < 0x7f);
	const QColor& rgbLite = (isEnabled()
		? (bDark ? Qt::darkYellow : Qt::yellow) : pal.mid().color());
    const QColor& rgbDark = pal.window().color().darker(180);

	painter.fillRect(rect, rgbDark);

	painter.setPen(bDark ? Qt::gray : Qt::darkGray);

	QLinearGradient grad(0, 0, w << 1, h << 1);
	grad.setColorAt(0.0f, rgbLite);
	grad.setColorAt(1.0f, Qt::black);

	painter.setRenderHint(QPainter::Antialiasing, true);

	painter.setBrush(grad);
	painter.drawPath(path);

#ifdef CONFIG_DEBUG_0
	painter.drawText(QFrame::rect(),
		Qt::AlignTop|Qt::AlignHCenter,
		tr("Cutoff(%1) Reso(%2)")
		.arg(int(100.0f * cutoff()))
		.arg(int(100.0f * reso())));
#endif

	painter.setRenderHint(QPainter::Antialiasing, false);

	painter.end();

	QFrame::paintEvent(pPaintEvent);
}


// Drag/move curve.
void synthv1widget_filt::dragCurve ( const QPoint& pos )
{
	const int h  = height();
	const int w  = width();

	const int dx = (pos.x() - m_posDrag.x());
	const int dy = (pos.y() - m_posDrag.y());

	if (dx || dy) {
		const int h2 = (h >> 1);
		const int x = int(cutoff() * float(w));
		const int y = int(reso() * float(h2));
		setCutoff(float(x + dx) / float(w));
		setReso(float(y - dy) / float(h2));
		m_posDrag = pos;
	}
}


// Mouse interaction.
void synthv1widget_filt::mousePressEvent ( QMouseEvent *pMouseEvent )
{
	if (pMouseEvent->button() == Qt::LeftButton)
		m_posDrag = pMouseEvent->pos();

	QFrame::mousePressEvent(pMouseEvent);
}


void synthv1widget_filt::mouseMoveEvent ( QMouseEvent *pMouseEvent )
{
	const QPoint& pos = pMouseEvent->pos();
	if (m_bDragging) {
		dragCurve(pos);
	} else { // if ((pos - m_posDrag).manhattanLength() > 4) {
		setCursor(Qt::SizeAllCursor);
		m_bDragging = true;
	}
}


void synthv1widget_filt::mouseReleaseEvent ( QMouseEvent *pMouseEvent )
{
	QFrame::mouseReleaseEvent(pMouseEvent);

	if (m_bDragging) {
		dragCurve(pMouseEvent->pos());
		m_bDragging = false;
		unsetCursor();
	}
}


void synthv1widget_filt::wheelEvent ( QWheelEvent *pWheelEvent )
{
	const int delta = (pWheelEvent->delta() / 60);

	if (pWheelEvent->modifiers() & (Qt::ShiftModifier | Qt::ControlModifier)) {
		const int h2 = (height() >> 1);
		const int y = int(reso() * float(h2));
		setReso(float(y + delta) / float(h2));
	} else {
		const int w2 = (width() >> 1);
		const int x = int(cutoff() * float(w2));
		setCutoff(float(x + delta) / float(w2));
	}
}


// end of synthv1widget_filt.cpp
