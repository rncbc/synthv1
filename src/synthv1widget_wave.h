// synthv1widget_wave.h
//
/****************************************************************************
   Copyright (C) 2012-2014, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __synthv1widget_wave_h
#define __synthv1widget_wave_h

#include <QFrame>


// Forward decl.
class synthv1_wave_lf;


//----------------------------------------------------------------------------
// synthv1widget_wave -- Custom widget

class synthv1widget_wave : public QFrame
{
	Q_OBJECT

public:

	// Constructor.
	synthv1widget_wave(QWidget *pParent = 0, Qt::WindowFlags wflags = 0);
	// Destructor.
	~synthv1widget_wave();

	// Parameter getters.
	float waveShape() const;
	float waveWidth() const;

public slots:

	// Parameter setters.
	void setWaveShape(float fWaveShape);
	void setWaveWidth(float fWaveWidth);

signals:

	// Parameter change signals.
	void waveShapeChanged(float fWaveShape);
	void waveWidthChanged(float fWaveWidth);

protected:

	// Draw canvas.
	void paintEvent(QPaintEvent *);

	// Drag/move curve.
	void dragCurve(const QPoint& pos);

	// Mouse interaction.
	void mousePressEvent(QMouseEvent *pMouseEvent);
	void mouseMoveEvent(QMouseEvent *pMouseEvent);
	void mouseReleaseEvent(QMouseEvent *pMouseEvent);
	void mouseDoubleClickEvent(QMouseEvent *pMouseEvent);
	void wheelEvent(QWheelEvent *pWheelEvent);

private:

	// Instance state.
	synthv1_wave_lf *m_pWave;

	// Drag state.
	bool m_bDragging;
	int m_iDragShape;
	QPoint m_posDrag;
};

#endif	// __synthv1widget_wave_h


// end of synthv1widget_wave.h
