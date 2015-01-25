// synthv1widget_filt.h
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

#ifndef __synthv1widget_filt_h
#define __synthv1widget_filt_h

#include <QFrame>


//----------------------------------------------------------------------------
// synthv1widget_filt -- Custom widget

class synthv1widget_filt : public QFrame
{
	Q_OBJECT

public:

	// Constructor.
	synthv1widget_filt(QWidget *pParent = 0, Qt::WindowFlags wflags = 0);
	// Destructor.
	~synthv1widget_filt();

	// Parameter getters.
	float cutoff() const;
	float reso() const;
	float type() const;
	float slope() const;

public slots:

	// Parameter setters.
	void setCutoff(float fCutoff);
	void setReso(float fReso);
	void setType(float fType);
	void setSlope(float fSlope);

signals:

	// Parameter change signals.
	void cutoffChanged(float fCutoff);
	void resoChanged(float fReso);

protected:

	// Draw canvas.
	void paintEvent(QPaintEvent *);

	// Drag/move curve.
	void dragCurve(const QPoint& pos);

	// Mouse interaction.
	void mousePressEvent(QMouseEvent *pMouseEvent);
	void mouseMoveEvent(QMouseEvent *pMouseEvent);
	void mouseReleaseEvent(QMouseEvent *pMouseEvent);
	void wheelEvent(QWheelEvent *pWheelEvent);

private:

	// Instance state.
	float m_fCutoff;
	float m_fReso;
	float m_fType;
	float m_fSlope;

	// Drag state.
	bool m_bDragging;
	QPoint m_posDrag;
};

#endif	// __synthv1widget_filt_h


// end of synthv1widget_filt.h
