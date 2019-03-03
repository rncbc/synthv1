// synthv1widget_keybd.h
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

#ifndef __synthv1widget_keybd_h
#define __synthv1widget_keybd_h

#include <QWidget>
#include <QPixmap>


//-------------------------------------------------------------------------
// synthv1widget_keybd - A horizontal piano keyboard widget.

class synthv1widget_keybd : public QWidget
{
	Q_OBJECT

public:

	// Constructor.
	synthv1widget_keybd(QWidget *pParent = 0);

	// Note range predicate.
	void setNoteRange(bool bNoteRange);
	bool isNoteRange() const;

	// Default note-on velocity.
	void setVelocity(int iVelocity);
	int velocity() const;

	// Keyboard note range getters.
	int noteLow() const;
	int noteHigh() const;

public slots:

	// Keyboard note range setters.
	void setNoteLow(int iNoteLow);
	void setNoteHigh(int iNoteHigh);

	// Keyboard note/key actions.
	void noteOn(int iNote);
	void noteOff(int iNote);

	void allNotesOff();

signals:

	// Piano keyboard note-on/off signal.
	void noteOnClicked(int iNote, int iVelocity);

	// Keyboard note range changed signal.
	void noteRangeChanged();

protected slots:

	// Kill dangling notes, if any...
	void allNotesTimeout();

protected:

	// Keyboard note range sanitizers.
	int safeNoteLow(int iNoteLow) const;
	int safeNoteHigh(int iNoteHigh) const;

	// Piano key rectangle finder.
	QRect noteRect(int iNote, bool bOn = false) const;

	// Piano keyboard note-on/off handlers.
	void dragNoteOn(const QPoint& pos);
	void dragNoteOff();

	// (Re)create the complete view pixmap.
	void updatePixmap();

	// Paint event handler.
	void paintEvent(QPaintEvent *pPaintEvent);

	// Resize event handler.
	void resizeEvent(QResizeEvent *pResizeEvent);

	// Alternate mouse behavior event handlers.
	void mousePressEvent(QMouseEvent *pMouseEvent);
	void mouseMoveEvent(QMouseEvent *pMouseEvent);
	void mouseReleaseEvent(QMouseEvent *pMouseEvent);

	// Keyboard event handler.
	void keyPressEvent(QKeyEvent *pKeyEvent);

	// Trap for help/tool-tip events.
	bool eventFilter(QObject *pObject, QEvent *pEvent);

	// Present a tooltip for a note.
	void noteToolTip(const QPoint& pos) const;

	// Default note name map accessor.
	QString noteName(int iNote) const;

	// Reset drag/select state.
	void resetDragState();

protected:

	// Constants
	static const int NUM_NOTES = 128;

	static const int MIN_NOTE  = 0;
	static const int MAX_NOTE  = 127;

	static const int MIN_VELOCITY = 1;
	static const int MAX_VELOCITY = 127;

	// Local double-buffering pixmap.
	QPixmap m_pixmap;

	// Current notes being keyed on.
	struct Note
	{
		bool  on;
		QRect rect;

	} m_notes[NUM_NOTES];

	// Keyboard note range state.
	enum DragState {
		DragNone = 0, DragStart,
		DragNoteRange, DragNoteLow, DragNoteHigh
	} m_dragState, m_dragCursor;

	QPoint m_posDrag;

	// Piano keyboard note range.
	bool m_bNoteRange;

	int m_iNoteLow;
	int m_iNoteLowX;

	int m_iNoteHigh;
	int m_iNoteHighX;

	// Current note being keyed on.
	int m_iNoteOn;

	// Current note-on timeout.
	int m_iTimeout;

	// Default note-on velocity.
	int m_iVelocity;
};


#endif  // __synthv1widget_keybd_h

// end of synthv1widget_keybd.h
