// synthv1widget_env.cpp
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

#include "synthv1widget_env.h"

#include <QPainter>
#include <QMouseEvent>

#include <math.h>


// Safe value capping.
inline float safe_value ( float x )
{
	return (x < 0.0f ? 0.0f : (x > 1.0f ? 1.0f : x));
}


//----------------------------------------------------------------------------
// synthv1widget_env -- Custom widget

// Constructor.
synthv1widget_env::synthv1widget_env (
	QWidget *pParent, Qt::WindowFlags wflags )
	: QFrame(pParent, wflags),
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
		update();
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
		update();
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
		update();
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
		update();
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

	const QRect& rect = QWidget::rect();
	const int h  = rect.height();
	const int w  = rect.width();

	const int w4 = (w - 12) >> 2;

	const int x1 = int(m_fAttack  * float(w4)) + 6;
	const int x2 = int(m_fDecay   * float(w4)) + x1;
	const int x3 = x2 + w4;
	const int x4 = int(m_fRelease * float(w4)) + x3;

	const int y3 = h - int(m_fSustain * float(h - 12)) - 6;

	m_poly.putPoints(0, 7,
		0,  h,
		6,  h - 6,
		x1, 6,
		x2, y3,
		x3, y3,
		x4, h - 6,
		x4, h);

	QPainterPath path;
	path.addPolygon(m_poly);

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

	painter.setBrush(pal.mid().color());
	painter.drawRect(nodeRect(1));
	painter.setBrush(rgbLite);
	painter.drawRect(nodeRect(2));
	painter.drawRect(nodeRect(3));
	painter.drawRect(nodeRect(4));
	painter.drawRect(nodeRect(5));

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
	if (nodeRect(5).contains(pos))
		return 5; // Release

	if (nodeRect(4).contains(pos))
		return 4; // Sustain

	if (nodeRect(3).contains(pos))
		return 3; // Decay

	if (nodeRect(2).contains(pos))
		return 2; // Attack

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
		case 2: // Attack
			x = int(attack() * float(w4));
			setAttack(float(x + dx) / float(w4));
			break;
		case 3: // Decay/Sustain
			x = int(decay() * float(w4));
			setDecay(float(x + dx) / float(w4));
			// Fall thru...
		case 4: // Sustain
			y = int(sustain() * float(h - 12));
			setSustain(float(y - dy) / float(h - 12));
			break;
		case 5: // Release
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
		if (iDragNode >= 0) {
			switch (iDragNode) {
			case 2: // Attack
			case 5: // Release
				setCursor(Qt::SizeHorCursor);
				break;
			case 3: // Decay/Sustain
				setCursor(Qt::SizeAllCursor);
				break;
			case 4: // Sustain
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
	if (m_iDragNode >= 0)
		dragNode(pos);
	else if (nodeIndex(pos) >= 0)
		setCursor(Qt::PointingHandCursor);
	else
		unsetCursor();
}


void synthv1widget_env::mouseReleaseEvent ( QMouseEvent *pMouseEvent )
{
	QFrame::mouseReleaseEvent(pMouseEvent);

	if (m_iDragNode >= 0) {
		dragNode(pMouseEvent->pos());
		m_iDragNode = -1;
		unsetCursor();
	}
}


// end of synthv1widget_env.cpp
