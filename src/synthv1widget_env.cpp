// synthv1widget_env.cpp
//
/****************************************************************************
   Copyright (C) 2012-2021, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "synthv1widget_env.h"

#include <QPainter>
#include <QPainterPath>

#include <QLinearGradient>

#include <QMouseEvent>

#include <cmath>


// Safe value capping.
inline float safe_value ( float x )
{
	return (x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x));
}


//----------------------------------------------------------------------------
// synthv1widget_env -- Custom widget

// Constructor.
synthv1widget_env::synthv1widget_env ( QWidget *pParent )
	: QFrame(pParent),
		m_fAttack(0.0f), m_fDecay(0.0f), m_fSustain(0.0f), m_fRelease(0.0f),
		m_poly(7), m_iDragNode(-1)
{
	setMouseTracking(true);
	setMinimumSize(QSize(120, 72));

	QFrame::setFrameShape(QFrame::Panel);
	QFrame::setFrameShadow(QFrame::Sunken);
}


// Destructor.
synthv1widget_env::~synthv1widget_env (void)
{
}


// Parameter accessors.
void synthv1widget_env::setAttack ( float fAttack )
{
	if (::fabsf(m_fAttack - fAttack) > 0.001f) {
		m_fAttack = safe_value(fAttack);
		updatePolygon();
		emit attackChanged(attack());
	}
}

float synthv1widget_env::attack (void) const
{
	return m_fAttack;
}


void synthv1widget_env::setDecay ( float fDecay )
{
	if (::fabsf(m_fDecay - fDecay) > 0.001f) {
		m_fDecay = safe_value(fDecay);
		updatePolygon();
		emit decayChanged(decay());
	}
}

float synthv1widget_env::decay (void) const
{
	return m_fDecay;
}


void synthv1widget_env::setSustain ( float fSustain )
{
	if (::fabsf(m_fSustain - fSustain) > 0.001f) {
		m_fSustain = safe_value(fSustain);
		updatePolygon();
		emit sustainChanged(sustain());
	}
}

float synthv1widget_env::sustain (void) const
{
	return m_fSustain;
}


void synthv1widget_env::setRelease ( float fRelease )
{
	if (::fabsf(m_fRelease - fRelease) > 0.001f) {
		m_fRelease = safe_value(fRelease);
		updatePolygon();
		emit releaseChanged(release());
	}
}

float synthv1widget_env::release (void) const
{
	return m_fRelease;
}


// Draw curve.
void synthv1widget_env::paintEvent ( QPaintEvent *pPaintEvent )
{
	QPainter painter(this);

	const QRect& rect = QFrame::rect();
	const int h = rect.height();
	const int w = rect.width();

	QPainterPath path;
//	path.addPolygon(m_poly);
	QPoint p1, p2, p3;
	path.moveTo(m_poly.at(0));
	p1 = m_poly.at(Idle);
	path.lineTo(p1);
	p2 = p1;
	p2.setY(h >> 1);
	p3 = m_poly.at(Attack);
	path.cubicTo(p1, p2, p3);
	p1 = p2 = p3;
	p3 = m_poly.at(Decay);
	p2.setY((p3.y() >> 1) + 1);
	path.cubicTo(p1, p2, p3);
	p1 = m_poly.at(Sustain);
	path.lineTo(p1);
	p2 = p1;
	p2.setY(p1.y() + ((h - p1.y()) >> 1) - 1);
	p3 = m_poly.at(Release);
	path.cubicTo(p1, p2, p3);
	path.lineTo(m_poly.at(End));
	path.lineTo(m_poly.at(0));

	const QPalette& pal = palette();
	const bool bDark = (pal.window().color().value() < 0x7f);
	const QColor& rgbLite = (isEnabled() ? Qt::yellow : pal.mid().color());
	const QColor& rgbDark = pal.window().color().darker();

	painter.fillRect(rect, rgbDark);

	QColor rgbLite1(rgbLite);
	QColor rgbDrop1(Qt::black);
	rgbLite1.setAlpha(bDark ? 80 : 180);
	rgbDrop1.setAlpha(80);

	QLinearGradient grad(0, 0, w << 1, h << 1);
	grad.setColorAt(0.0f, rgbLite1);
	grad.setColorAt(1.0f, rgbDrop1);

	painter.setRenderHint(QPainter::Antialiasing, true);

//	painter.setPen(bDark ? Qt::gray : Qt::darkGray);
	painter.setPen(QPen(rgbLite1, 2));
	painter.setBrush(grad);
	painter.drawPath(path);

	painter.setPen(rgbDrop1);
	painter.setBrush(rgbDrop1.lighter());
	painter.drawRect(nodeRect(Idle));
	painter.setPen(rgbLite1.lighter());
	painter.setBrush(rgbLite1);
	painter.drawRect(nodeRect(Attack));
	painter.drawRect(nodeRect(Decay));
	painter.drawRect(nodeRect(Sustain));
	painter.drawRect(nodeRect(Release));

#ifdef CONFIG_DEBUG_0
	painter.drawText(QFrame::rect(),
		Qt::AlignTop|Qt::AlignHCenter,
		tr("A(%1) D(%2) S(%3) R(%4)")
		.arg(int(100.0f * attack()))
		.arg(int(100.0f * decay()))
		.arg(int(100.0f * sustain()))
		.arg(int(100.0f * release())));
#endif

	painter.setRenderHint(QPainter::Antialiasing, false);

	painter.end();

	QFrame::paintEvent(pPaintEvent);
}


// Draw rectangular point.
QRect synthv1widget_env::nodeRect ( int iNode ) const
{
	const QPoint& pos = m_poly.at(iNode);
	return QRect(pos.x() - 4, pos.y() - 4, 8, 8); 
}


int synthv1widget_env::nodeIndex ( const QPoint& pos ) const
{
	if (nodeRect(Release).contains(pos))
		return Release;
	if (nodeRect(Sustain).contains(pos))
		return Sustain;
	if (nodeRect(Decay).contains(pos))
		return Decay;
	if (nodeRect(Attack).contains(pos))
		return Attack;

	return -1;
}


void synthv1widget_env::dragNode ( const QPoint& pos )
{
	const int h  = height();
	const int w  = width();

	const int w4 = (w - 12) >> 2;

	const int dx = (pos.x() - m_posDrag.x());
	const int dy = (pos.y() - m_posDrag.y());

	if (dx || dy) {
		int x, y;
		switch (m_iDragNode) {
		case Attack:
			x = int(attack() * float(w4));
			setAttack(float(x + dx) / float(w4));
			break;
		case Decay:
			x = int(decay() * float(w4));
			setDecay(float(x + dx) / float(w4));
			// Fall thru...
		case Sustain:
			y = int(sustain() * float(h - 12));
			setSustain(float(y - dy) / float(h - 12));
			break;
		case Release:
			x = int(release() * float(w4));
			setRelease(float(x + dx) / float(w4));
			break;
		}
		m_posDrag = m_poly.at(m_iDragNode);
	//	m_posDrag = pos;
	}
}


// Mouse interaction.
void synthv1widget_env::mousePressEvent ( QMouseEvent *pMouseEvent )
{
	if (pMouseEvent->button() == Qt::LeftButton) {
		const QPoint& pos = pMouseEvent->pos();
		const int iDragNode = nodeIndex(pos);
		if (iDragNode > Idle) {
			switch (iDragNode) {
			case Attack:
			case Release:
				setCursor(Qt::SizeHorCursor);
				break;
			case Decay:
				setCursor(Qt::SizeAllCursor);
				break;
			case Sustain:
				setCursor(Qt::SizeVerCursor);
				break;
			default:
				break;
			}
			m_iDragNode = iDragNode;
			m_posDrag = pos;
		}
	}

	QFrame::mousePressEvent(pMouseEvent);
}


void synthv1widget_env::mouseMoveEvent ( QMouseEvent *pMouseEvent )
{
	const QPoint& pos = pMouseEvent->pos();
	if (m_iDragNode > Idle)
		dragNode(pos);
	else if (nodeIndex(pos) > Idle)
		setCursor(Qt::PointingHandCursor);
	else
		unsetCursor();
}


void synthv1widget_env::mouseReleaseEvent ( QMouseEvent *pMouseEvent )
{
	QFrame::mouseReleaseEvent(pMouseEvent);

	if (m_iDragNode > Idle) {
		dragNode(pMouseEvent->pos());
		m_iDragNode = -1;
		unsetCursor();
	}
}


// Resize canvas.
void synthv1widget_env::resizeEvent ( QResizeEvent *pResizeEvent )
{
	QFrame::resizeEvent(pResizeEvent);

	updatePolygon();
}


// Update the drawing polygon.
void synthv1widget_env::updatePolygon (void)
{
	const QRect& rect = QFrame::rect();
	const int h  = rect.height();
	const int w  = rect.width();

	const int w4 = (w - 10) >> 2;

	const int x1 = int(m_fAttack  * float(w4)) + 5;
	const int x2 = int(m_fDecay   * float(w4)) + x1;
	const int x3 = x2 + w4;
	const int x4 = int(m_fRelease * float(w4)) + x3;

	const int y3 = h - int(m_fSustain * float(h - 10)) - 5;

	m_poly.putPoints(0, 7,
		5,  h,
		5,  h - 5, // Idle
		x1, 5,     // Attack
		x2, y3,    // Decay
		x3, y3,    // Sustain
		x4, h - 5, // Release
		x4, h);

	QFrame::update();
}


// end of synthv1widget_env.cpp
