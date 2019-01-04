// synthv1widget_keybd.cpp
//
/****************************************************************************
   Copyright (C) 2012-2019, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "synthv1widget_keybd.h"

#include "synthv1_ui.h"

#include <QPainter>
#include <QToolTip>

#include <QApplication>

#include <QPaintEvent>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QKeyEvent>

#include <math.h>


//-------------------------------------------------------------------------
// synthv1widget_keybd - A horizontal piano keyboard widget.


// Constructor.
synthv1widget_keybd::synthv1widget_keybd ( QWidget *pParent )
	: QWidget(pParent)
{
	const QFont& font = QWidget::font();
	QWidget::setFont(QFont(font.family(), font.pointSize() - 3));
	QWidget::setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	QWidget::setMinimumSize(QSize(440, 22));
	QWidget::setMouseTracking(true);

	for (int n = 0; n < NUM_NOTES; ++n)
		m_notes[n].on = false;

	m_dragCursor = DragNone;

	m_iNoteLow   = MIN_NOTE;
	m_iNoteLowX  = 0;
	m_iNoteHigh  = MAX_NOTE;
	m_iNoteHighX = 0;

	m_iNoteOn    = -1;

	m_iVelocity  = (MIN_VELOCITY + MAX_VELOCITY) / 2;

	resetDragState();

	// Trap for help/tool-tips and leave events.
	QWidget::installEventFilter(this);
}


// Default note-on velocity.
void synthv1widget_keybd::setVelocity ( int iVelocity )
{
	if (iVelocity < MIN_VELOCITY)
		iVelocity = MIN_VELOCITY;
	if (iVelocity > MAX_VELOCITY)
		iVelocity = MAX_VELOCITY;

	m_iVelocity = iVelocity;
}


int synthv1widget_keybd::velocity (void) const
{
	return m_iVelocity;
}


// Keyboard note range sanitizers.
int synthv1widget_keybd::safeNoteLow ( int iNoteLow ) const
{
	if (iNoteLow < MIN_NOTE)
		iNoteLow = MIN_NOTE;
	if (iNoteLow > m_iNoteHigh)
		iNoteLow = m_iNoteHigh;

	return iNoteLow;
}


int synthv1widget_keybd::safeNoteHigh ( int iNoteHigh ) const
{
	if (iNoteHigh > MAX_NOTE)
		iNoteHigh = MAX_NOTE;
	if (iNoteHigh < m_iNoteLow)
		iNoteHigh = m_iNoteLow;

	return iNoteHigh;
}


// Keyboard note range setters/getters.
void synthv1widget_keybd::setNoteLow ( int iNoteLow )
{
	m_iNoteLow  = safeNoteLow(iNoteLow);
	m_iNoteLowX = noteRect(m_iNoteLow).left();

	QWidget::update();
}


int synthv1widget_keybd::noteLow (void) const
{
	return m_iNoteLow;
}


void synthv1widget_keybd::setNoteHigh ( int iNoteHigh )
{
	m_iNoteHigh  = safeNoteHigh(iNoteHigh);
	m_iNoteHighX = noteRect(m_iNoteHigh).right();

	QWidget::update();
}


int synthv1widget_keybd::noteHigh (void) const
{
	return m_iNoteHigh;
}


// Piano key rectangle finder.
QRect synthv1widget_keybd::noteRect ( int iNote ) const
{
	const int w = QWidget::width();
	const int h = QWidget::height();

	const float wn = float(w - 4) / float(NUM_NOTES);
	const float wk = 12.0f * wn / 7.0f;
	const int w2 = int(wn + 0.5f);

	int k = (iNote % 12);
	if (k >= 5) ++k;

	const int nk = (iNote / 12) * 7 + (k >> 1);
	int x2 = int(wk * float(nk));
	int h2 = h;
	if (k & 1) {
		x2 += int(wk - float(w2 >> 1));
		h2  = (h << 1) / 3;
	} else {
		x2 += (w2 >> 1);
	}

	return QRect(x2, 0, w2, h2);
}


// Piano keyboard note/key actions.
void synthv1widget_keybd::noteOn ( int iNote )
{
	if (iNote < m_iNoteLow || iNote > m_iNoteHigh)
		return;

	// If it ain't changed we won't change it ;)
	Note& note = m_notes[iNote];
	if (note.on)
		return;

	// Now for the sounding new one...
	note.on = true;
	note.rect = noteRect(iNote);

	QWidget::update(note.rect);
}


void synthv1widget_keybd::noteOff ( int iNote )
{
	if (iNote < m_iNoteLow || iNote > m_iNoteHigh)
		return;

	// Turn off old note...
	Note& note = m_notes[iNote];
	if (!note.on)
		return;

	// Now for the sounding new one...
	note.on = false;

	QWidget::update(note.rect);
}


void synthv1widget_keybd::allNotesOff (void)
{
	for (int n = 0; n < NUM_NOTES; ++n)
		noteOff(n);
}


// Piano keyboard note-on handler.
void synthv1widget_keybd::dragNoteOn ( const QPoint& pos )
{
	// Compute new key cordinates...
	const int iNote = (NUM_NOTES * pos.x() / QWidget::width());

	if (iNote < m_iNoteLow || iNote > m_iNoteHigh || iNote == m_iNoteOn)
		return;

	// Were we pending on some sounding note?
	dragNoteOff();

	// Now for the sounding new one...
	m_iNoteOn = iNote;

//	noteOn(iNote);

	emit noteOnClicked(iNote, m_iVelocity);
}


// Piano keyboard note-off handler.
void synthv1widget_keybd::dragNoteOff (void)
{
	if (m_iNoteOn < 0)
		return;

	// Turn off old note...
	const int iNote = m_iNoteOn;

	m_iNoteOn = -1;

//	noteOff(iNote);

	emit noteOnClicked(iNote, 0);
}


// (Re)create the complete view pixmap.
void synthv1widget_keybd::updatePixmap (void)
{
	const int w = QWidget::width();
	const int h = QWidget::height();
	if (w < 4 || h < 4)
		return;

	const QPalette& pal = QWidget::palette();

	const QColor& rgbLine   = pal.mid().color();
	const QColor& rgbLight  = pal.light().color();
	const QColor& rgbShadow = pal.shadow().color();

	m_pixmap = QPixmap(w, h);
	m_pixmap.fill(pal.window().color());

	QPainter painter(&m_pixmap);
	painter.initFrom(this);

	const float wn = float(w - 4) / float(NUM_NOTES);
	const float wk = 12.0f * wn / 7.0f;
	const int w2 = int(wn + 0.5f);
	const int h3 = (h << 1) / 3;

	QLinearGradient gradLight(0, 0, 0, h);
	gradLight.setColorAt(0.0, rgbLight);
	gradLight.setColorAt(0.1, rgbLight.lighter());
	painter.fillRect(0, 0, w, h, gradLight);
	painter.setPen(rgbLine);

	int n, k;

	for (n = 0; n < NUM_NOTES; ++n) {
		k = (n % 12);
		if (k >= 5) ++k;
		if ((k & 1) == 0) {
			const int nk = (n / 12) * 7 + (k >> 1);
			const int x1 = int(wk * float(nk));
			painter.drawLine(x1, 0, x1, h);
			if (k == 0 && w2 > 10)
				painter.drawText(x1 + 4, h - 4, noteName(n));
		}
	}

	QLinearGradient gradDark(0, 0, 0, h3);
	gradDark.setColorAt(0.0, rgbLight);
	gradDark.setColorAt(0.4, rgbShadow);
	gradDark.setColorAt(0.96, rgbShadow);
	gradDark.setColorAt(0.98, rgbLight);
	gradDark.setColorAt(1.0, rgbShadow);
	painter.setBrush(gradDark);

	for (n = 0; n < NUM_NOTES; ++n) {
		k = (n % 12);
		if (k >= 5) ++k;
		if (k & 1) {
			const int nk = (n / 12) * 7 + (k >> 1);
			const int x1 = int(wk * float(nk + 1) - float(w2 >> 1));
			painter.drawRect(x1, 0, w2, h3);
		}
	}

	m_iNoteLowX  = noteRect(m_iNoteLow).left();
	m_iNoteHighX = noteRect(m_iNoteHigh).right();
}


// Paint event handler.
void synthv1widget_keybd::paintEvent ( QPaintEvent *pPaintEvent )
{
	QPainter painter(this);
	
	// Render the pixmap region...
	const QRect& rect = pPaintEvent->rect();
	painter.drawPixmap(rect, m_pixmap, rect);

	const QPalette& pal = QWidget::palette();
	QColor rgbOver;

	// Are we sticking in some note?
	rgbOver = pal.highlight().color().dark(120);
	rgbOver.setAlpha(180);
	for (int n = 0; n < NUM_NOTES; ++n) {
		Note& note = m_notes[n];
		if (note.on)
			painter.fillRect(note.rect, rgbOver);
	}

	// Keyboard range lines...
	const int w  = QWidget::width();
	const int h  = QWidget::height();
	const int x1 = m_iNoteLowX;
	const int x2 = m_iNoteHighX;

	rgbOver = pal.dark().color().darker();
	rgbOver.setAlpha(120);
	if (x1 > 0)
		painter.fillRect(0, 0, x1, h, rgbOver);
	if (x2 < w)
		painter.fillRect(x2, 0, w, h, rgbOver);
}


// Resize event handler.
void synthv1widget_keybd::resizeEvent ( QResizeEvent *pResizeEvent )
{
	updatePixmap();

	return QWidget::resizeEvent(pResizeEvent);
}


// Alternate mouse behavior event handlers.
void synthv1widget_keybd::mousePressEvent ( QMouseEvent *pMouseEvent )
{
	const QPoint& pos = pMouseEvent->pos();
	switch (pMouseEvent->button()) {
	case Qt::LeftButton:
		if (m_dragCursor == DragNone) {
			// Are we keying in some keyboard?
			if ((pMouseEvent->modifiers()
				& (Qt::ShiftModifier | Qt::ControlModifier)) == 0) {
				dragNoteOn(pos);
				noteToolTip(pos);
			}
			// Maybe we'll start something...
			m_dragState = DragStart;
			m_posDrag = pos;
		} else {
			m_dragState = m_dragCursor;
		}
		break;
	default:
		break;
	}

	QWidget::mousePressEvent(pMouseEvent);
}


void synthv1widget_keybd::mouseMoveEvent ( QMouseEvent *pMouseEvent )
{
	const QPoint& pos = pMouseEvent->pos();
	switch (m_dragState) {
	case DragNone: {
		// Are we already moving/dragging something?
		const int dx = 4;
		const int x1 = m_iNoteLowX;
		const int x2 = m_iNoteHighX;
		if (::abs(x2 - pos.x()) < dx) {
			m_dragCursor = DragNoteHigh;
			QWidget::setCursor(QCursor(Qt::SizeHorCursor));
			QToolTip::showText(
				QWidget::mapToGlobal(pos),
				tr("High: %1 (%2)")
					.arg(noteName(m_iNoteHigh)).arg(m_iNoteHigh), this);
		}
		else
		if (::abs(x1 - pos.x()) < dx) {
			m_dragCursor = DragNoteLow;
			QWidget::setCursor(QCursor(Qt::SizeHorCursor));
			QToolTip::showText(
				QWidget::mapToGlobal(pos),
				tr("Low: %1 (%2)")
					.arg(noteName(m_iNoteLow)).arg(m_iNoteLow), this);

		}
		else
		if (m_dragCursor != DragNone) {
			m_dragCursor  = DragNone;
			QWidget::unsetCursor();
		}
		break;
	}
	case DragNoteLow: {
		const int w = QWidget::width();
		if (w > 0) {
			const int iNoteLow = safeNoteLow((NUM_NOTES * pos.x()) / w);
			m_iNoteLowX = noteRect(iNoteLow).left();
			QWidget::update();
			QToolTip::showText(
				QCursor::pos(),
				tr("Low: %1 (%2)")
					.arg(noteName(iNoteLow)).arg(iNoteLow), this);
		}
		break;
	}
	case DragNoteHigh: {
		const int w = QWidget::width();
		if (w > 0) {
			const int iNoteHigh = safeNoteHigh((NUM_NOTES * pos.x()) / w);
			m_iNoteHighX = noteRect(iNoteHigh).right();
			QWidget::update();
			QToolTip::showText(
				QCursor::pos(),
				tr("High: %1 (%2)")
					.arg(noteName(iNoteHigh)).arg(iNoteHigh), this);
		}
		break;
	}
	case DragNoteRange: {
		const int w = QWidget::width();
		if (w > 0) {
			// Rubber-band offset selection...
			const QRect& rect = QRect(m_posDrag, pos).normalized();
			int iNoteLow  = (NUM_NOTES * rect.left())  / w;
			int iNoteHigh = (NUM_NOTES * rect.right()) / w;
			if (iNoteLow  < MIN_NOTE)
				iNoteLow  = MIN_NOTE;
			if (iNoteLow  > iNoteHigh)
				iNoteLow  = iNoteHigh;
			if (iNoteHigh > MAX_NOTE)
				iNoteHigh = MAX_NOTE;
			if (iNoteHigh < iNoteLow)
				iNoteHigh = iNoteLow;
			m_iNoteLowX   = noteRect(iNoteLow).left();
			m_iNoteHighX  = noteRect(iNoteHigh).right();
			QWidget::update();
			QToolTip::showText(
				QWidget::mapToGlobal(pos),
				tr("Low: %1 (%2) High: %3 (%4)")
					.arg(noteName(iNoteLow)).arg(iNoteLow)
					.arg(noteName(iNoteHigh)).arg(iNoteHigh), this);
		}
		break;
	}
	case DragStart: {
		const bool bModifiers = (pMouseEvent->modifiers()
			& (Qt::ShiftModifier | Qt::ControlModifier));
		if ((m_posDrag - pos).manhattanLength()
			> QApplication::startDragDistance()) {
			// Start dragging alright...
			if (m_dragCursor != DragNone)
				m_dragState = m_dragCursor;
			else
			if (bModifiers) {
				// Rubber-band starting...
				m_dragState = m_dragCursor = DragNoteRange;
				QWidget::setCursor(QCursor(Qt::SizeHorCursor));
			}
		}
		// Are we hovering in some keyboard?
		if (m_dragCursor == DragNone && !bModifiers) {
			dragNoteOn(pos);
			noteToolTip(pos);
		}
		break;
	}}

	QWidget::mouseMoveEvent(pMouseEvent);
}


void synthv1widget_keybd::mouseReleaseEvent ( QMouseEvent *pMouseEvent )
{
	const QPoint& pos = pMouseEvent->pos();
	switch (m_dragState) {
	case DragNoteLow: {
		const int w = QWidget::width();
		if (w > 0) {
			setNoteLow((NUM_NOTES * pos.x()) / w);
			emit noteRangeChanged();
		}
		break;
	}
	case DragNoteHigh: {
		const int w = QWidget::width();
		if (w > 0) {
			setNoteHigh((NUM_NOTES * pos.x()) / w);
			emit noteRangeChanged();
		}
		break;
	}
	case DragNoteRange: {
		const int w = QWidget::width();
		if (w > 0) {
			const QRect& rect = QRect(m_posDrag, pos).normalized();
			int iNoteLow  = (NUM_NOTES * rect.left())  / w;
			int iNoteHigh = (NUM_NOTES * rect.right()) / w;
			if (iNoteLow  < MIN_NOTE)
				iNoteLow  = MIN_NOTE;
			if (iNoteHigh > MAX_NOTE)
				iNoteHigh = MAX_NOTE;
			if (iNoteLow  > iNoteHigh)
				iNoteLow  = iNoteHigh;
			if (iNoteHigh < iNoteLow)
				iNoteHigh = iNoteLow;
			m_iNoteLow    = iNoteLow;
			m_iNoteLowX   = noteRect(iNoteLow).left();
			m_iNoteHigh   = iNoteHigh;
			m_iNoteHighX  = noteRect(iNoteHigh).right();
			QWidget::update();
			emit noteRangeChanged();
		}
		break;
	}
	default:
		break;
	}

	// Were we stuck on some keyboard note?
	resetDragState();

	QWidget::mouseReleaseEvent(pMouseEvent);
}


// Keyboard event handler.
void synthv1widget_keybd::keyPressEvent ( QKeyEvent *pKeyEvent )
{
	switch (pKeyEvent->key()) {
	case Qt::Key_Escape:
		resetDragState();
		QWidget::update();
		break;
	default:
		QWidget::keyPressEvent(pKeyEvent);
		break;
	}
}


	// Trap for help/tool-tip events.
bool synthv1widget_keybd::eventFilter ( QObject *pObject, QEvent *pEvent )
{
	if (static_cast<QWidget *> (pObject) == this) {
		if (pEvent->type() == QEvent::ToolTip) {
			QHelpEvent *pHelpEvent = static_cast<QHelpEvent *> (pEvent);
			if (pHelpEvent && m_dragCursor == DragNone) {
				noteToolTip(pHelpEvent->pos());
				return true;
			}
		}
		else
		if (pEvent->type() == QEvent::Leave) {
			dragNoteOff();
			return true;
		}
	}

	// Not handled here.
	return QWidget::eventFilter(pObject, pEvent);
}


// Present a tooltip for a note.
void synthv1widget_keybd::noteToolTip ( const QPoint& pos ) const
{
	const int iNote = (NUM_NOTES * pos.x() / QWidget::width());
	QToolTip::showText(QWidget::mapToGlobal(pos),
		QString("%1 (%2)").arg(noteName(iNote)).arg(iNote));
}


// Default note name map accessor.
QString synthv1widget_keybd::noteName ( int iNote ) const
{
	return synthv1_ui::noteName(iNote);
}


// Reset drag/select state.
void synthv1widget_keybd::resetDragState (void)
{
	dragNoteOff();

	if (m_dragCursor != DragNone)
		QWidget::unsetCursor();

	m_dragState = m_dragCursor = DragNone;
}


// end of synthv1widget_keybd.cpp
