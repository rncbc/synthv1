// synthv1widget_keybd.cpp
//
/****************************************************************************
   Copyright (C) 2012-2025, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include <QTimer>


//-------------------------------------------------------------------------
// synthv1widget_keybd - A horizontal piano keyboard widget.


// Constructor.
synthv1widget_keybd::synthv1widget_keybd ( QWidget *pParent )
	: QWidget(pParent)
{
	const QFont& font = QWidget::font();
#if QT_VERSION < QT_VERSION_CHECK(6, 2, 0)
	QWidget::setFont(QFont(font.family(), font.pointSize() - 3));
#else
	QWidget::setFont(QFont(QStringList() << font.family(), font.pointSize() - 3));
#endif
	QWidget::setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	QWidget::setMinimumSize(QSize(440, 22));
	QWidget::setMouseTracking(true);

	for (int n = 0; n < NUM_NOTES; ++n) {
		Note& note = m_notes[n];
		note.enabled = false;
		note.on = false;
	}

	m_dragCursor = DragNone;

	m_bNoteRange = false;

	m_iNoteLow   = MIN_NOTE;
	m_iNoteLowX  = 0;

	m_iNoteHigh  = MAX_NOTE;
	m_iNoteHighX = 0;

	m_iNoteOn    = -1;
	m_iTimeout   = 0;
	m_iVelocity  = (MIN_VELOCITY + MAX_VELOCITY) / 2;

	m_iNoteKey   = -1;

	resetDragState();

	// Trap for help/tool-tips and leave events.
	QWidget::installEventFilter(this);
}


// Note range predicate.
void synthv1widget_keybd::setNoteRange ( bool bNoteRange )
{
	m_bNoteRange = bNoteRange;

	QWidget::update();
}
 
 
bool synthv1widget_keybd::isNoteRange (void) const
{
	return m_bNoteRange;
}
 
 
// Note enabled predicate.
void synthv1widget_keybd::setNoteEnabled ( int iNote, bool bEnabled )
{
	if (iNote >= MIN_NOTE && MAX_NOTE >= iNote)
		m_notes[iNote].enabled = bEnabled;
}


bool synthv1widget_keybd::isNoteEnabled ( int iNote ) const
{
	if (iNote >= MIN_NOTE && MAX_NOTE >= iNote)
		return m_notes[iNote].enabled;
	else
		return false;
}


void synthv1widget_keybd::setAllNotesEnabled ( bool bEnabled )
{
	for (int n = 0; n < NUM_NOTES; ++n)
		setNoteEnabled(n, bEnabled);
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


void synthv1widget_keybd::setNoteKey ( int iNoteKey )
{
	if (iNoteKey >= MIN_NOTE && MAX_NOTE >= iNoteKey) {
		m_notes[iNoteKey].path = notePath(iNoteKey, true);
		m_iNoteKey = iNoteKey;
	} else {
		m_iNoteKey = -1;
	}

	QWidget::update();
}


int synthv1widget_keybd::noteKey (void) const
{
	return m_iNoteKey;
}


// Piano key rectangle finder.
QRect synthv1widget_keybd::noteRect ( int iNote, bool bOn ) const
{
	return notePath(iNote, bOn).boundingRect().toRect();
}


QPainterPath synthv1widget_keybd::notePath ( int iNote, bool bOn ) const
{
	QPainterPath path;

	const int w = QWidget::width();
	const int h = QWidget::height();

	const float wn = float(w - 4) / float(NUM_NOTES);
	const float wk = 12.0f * wn / 7.0f;

	int k = (iNote % 12);
	if (k >= 5) ++k;

	const int nk = (iNote / 12) * 7 + (k >> 1);
	const int x2 = int(wk * float(nk));
	const int w2 = int(wn + 0.5f);

	QPainterPath path1;
	path1.addRect(x2 + int(wk - float(w2 >> 1)), 0, w2 + 1, (h << 1) / 3);

	if (k & 1) {
		path = path1;
	} else if (bOn) {
		path.addRect(x2, 0, wk, h);
		if ((k == 0 || k == 2 || k == 6 || k == 8  || k == 10) && iNote < MAX_NOTE)
			path = path.subtracted(path1.translated(+ 0.5f, 0.0f));
		if ((k == 2 || k == 4 || k == 8 || k == 10 || k == 12) && iNote > MIN_NOTE)
			path = path.subtracted(path1.translated(+ 0.5f - wk, 0.0f));
	} else {
		path.addRect(x2, 0, (w2 << 1), h);
	}

	return path;
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
	note.path = notePath(iNote, true);

	QWidget::update(note.path.boundingRect().toRect());
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

	QWidget::update(note.path.boundingRect().toRect());
}


void synthv1widget_keybd::allNotesOff (void)
{
	for (int n = 0; n < NUM_NOTES; ++n)
		noteOff(n);
}


// Kill dangling notes, if any...
void synthv1widget_keybd::allNotesTimeout (void)
{
	if (m_iTimeout < 1)
		return;

	if (m_iNoteOn >= 0) {
		++m_iTimeout;
		QTimer::singleShot(1200, this, SLOT(allNotesTimeout())); // +3sec.
		return;
	}

	for (int n = 0; n < NUM_NOTES; ++n) {
		Note& note = m_notes[n];
		if (note.on) {
			note.on = false;
			QWidget::update(note.path.boundingRect().toRect());
			emit noteOnClicked(n, 0);
		}
	}

	m_iTimeout = 0;
}


// Piano keyboard note-on handler.
void synthv1widget_keybd::dragNoteOn ( const QPoint& pos )
{
	// Compute new key cordinates...
	const int iNote = noteAt(pos);

	if (iNote < m_iNoteLow || iNote > m_iNoteHigh || iNote == m_iNoteOn)
		return;

	// Were we pending on some sounding note?
	dragNoteOff();

	// Now for the sounding new one...
	m_iNoteOn = iNote;

//	noteOn(iNote);

	emit noteOnClicked(iNote, m_iVelocity);

	if (++m_iTimeout == 1)
		QTimer::singleShot(1200, this, SLOT(allNotesTimeout())); // +3sec.
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


// Piano keyboard note descriminator.
int synthv1widget_keybd::noteAt ( const QPoint& pos ) const
{
	const int w = QWidget::width();
	const int h = QWidget::height();

	const int yk = (h << 1) / 3;

	int iNote = (NUM_NOTES * pos.x()) / w;
	if (pos.y() >=  yk) {
		int k = (iNote % 12);
		if (k >= 5) ++k;
		if (k & 1) {
			const int xk = ((w * iNote) + (w >> 1)) / NUM_NOTES;
			if (pos.x() >= xk)
				++iNote;
			else
				--iNote;
		}
	}

	return iNote;
}


// (Re)create the complete view pixmap.
void synthv1widget_keybd::updatePixmap (void)
{
	const int w = QWidget::width();
	const int h = QWidget::height();
	if (w < 4 || h < 4)
		return;

	const QPalette& pal = QWidget::palette();
	const bool bDark = (pal.base().color().value() < 128);
	const QColor& rgbLine  = pal.mid().color();
	const QColor& rgbLight = QColor(Qt::white).darker(bDark ? 240 : 160);
	const QColor& rgbDark  = QColor(Qt::black).lighter(bDark ? 120 : 180);

	m_pixmap = QPixmap(w, h);
	m_pixmap.fill(pal.window().color());

	QPainter painter(&m_pixmap);
//	painter.initFrom(this);

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
	gradDark.setColorAt(0.4, rgbDark);
	gradDark.setColorAt(0.92, rgbDark);
	gradDark.setColorAt(0.96, rgbLight);
	gradDark.setColorAt(1.0, rgbDark);
	painter.setBrush(gradDark);

	for (n = 0; n < NUM_NOTES; ++n) {
		k = (n % 12);
		if (k >= 5) ++k;
		if (k & 1) {
			const int nk = (n / 12) * 7 + (k >> 1);
			const int x1 = int(wk * float(nk + 1) - float(w2 >> 1));
			painter.drawRect(x1, 0, w2, h3);
		}
		m_notes[n].path = notePath(n, (n == m_iNoteKey
			&& m_iNoteKey >= MIN_NOTE && MAX_NOTE >= m_iNoteKey));
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
	QColor rgbGray = pal.mid().color();
	rgbGray.setAlpha(120);

	// Are we enabled still?
	if (!QWidget::isEnabled()) {
		painter.fillRect(rect, rgbGray);
		return;
	}

	// Are we sticking in some note?
	QColor rgbOver = pal.highlight().color().darker(120);
	rgbOver.setAlpha(180);
	for (int n = 0; n < NUM_NOTES; ++n) {
		Note& note = m_notes[n];
		if (!note.enabled)
			painter.fillPath(note.path, rgbGray);
		else
		if (note.on)
			painter.fillPath(note.path, rgbOver);
	}

	// Keyboard range lines...
	if (m_bNoteRange) {
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

	// Current highlighted note.
	if (m_iNoteKey >= MIN_NOTE && MAX_NOTE >= m_iNoteKey) {
		rgbOver = pal.highlight().color().lighter();
		rgbOver.setAlpha(120);
		painter.fillPath(m_notes[m_iNoteKey].path, rgbOver);
	}
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
		// Fall thru...
	default:
		break;
	}

//	QWidget::mousePressEvent(pMouseEvent);
}


void synthv1widget_keybd::mouseMoveEvent ( QMouseEvent *pMouseEvent )
{
	const QPoint& pos = pMouseEvent->pos();

	switch (m_dragState) {
	case DragNone:
		if (m_bNoteRange) {
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
		}
		break;
	case DragNoteLow:
		if (m_bNoteRange) {
			const int iNoteLow = safeNoteLow(noteAt(pos));
			m_iNoteLowX = noteRect(iNoteLow).left();
			QWidget::update();
			QToolTip::showText(
				QCursor::pos(),
				tr("Low: %1 (%2)")
					.arg(noteName(iNoteLow)).arg(iNoteLow), this);
		}
		break;
	case DragNoteHigh:
		if (m_bNoteRange) {
			const int iNoteHigh = safeNoteHigh(noteAt(pos));
			m_iNoteHighX = noteRect(iNoteHigh).right();
			QWidget::update();
			QToolTip::showText(
				QCursor::pos(),
				tr("High: %1 (%2)")
					.arg(noteName(iNoteHigh)).arg(iNoteHigh), this);
		}
		break;
	case DragNoteRange:
		if (m_bNoteRange) {
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
		}
		break;
	case DragStart:
		if (m_bNoteRange) {
			if ((m_posDrag - pos).manhattanLength()
				> QApplication::startDragDistance()) {
				// Start dragging alright...
				if (m_dragCursor != DragNone)
					m_dragState = m_dragCursor;
				else
				if (pMouseEvent->modifiers()
					& (Qt::ShiftModifier | Qt::ControlModifier)) {
					// Rubber-band starting...
					m_dragState = m_dragCursor = DragNoteRange;
					QWidget::setCursor(QCursor(Qt::SizeHorCursor));
				}
			}
		}
		// Are we still hovering the keyboard?
		if (m_dragState == DragStart) {
			dragNoteOn(pos);
			noteToolTip(pos);
		}
		// Fall thru...
	default:
		break;
	}

//	QWidget::mouseMoveEvent(pMouseEvent);
}


void synthv1widget_keybd::mouseReleaseEvent ( QMouseEvent *pMouseEvent )
{
	const QPoint& pos = pMouseEvent->pos();

	switch (m_dragState) {
	case DragNoteLow:
		if (m_bNoteRange) {
			setNoteLow(noteAt(pos));
			emit noteRangeChanged();
		}
		break;
	case DragNoteHigh:
		if (m_bNoteRange) {
			setNoteHigh(noteAt(pos));
			emit noteRangeChanged();
		}
		break;
	case DragNoteRange:
		if (m_bNoteRange) {
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
		}
		break;
		// Fall thru...
	default:
		break;
	}

	// Were we stuck on some keyboard note?
	resetDragState();

//	QWidget::mouseReleaseEvent(pMouseEvent);
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
	const int iNote = noteAt(pos);

	if (iNote < MIN_NOTE || iNote > MAX_NOTE)
		return;

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
