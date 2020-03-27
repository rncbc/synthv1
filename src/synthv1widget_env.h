// synthv1widget_env.h
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

#ifndef __synthv1widget_env_h
#define __synthv1widget_env_h

#include <QFrame>


//----------------------------------------------------------------------------
// synthv1widget_env -- Custom widget

class synthv1widget_env : public QFrame
{
	Q_OBJECT

public:

	// Constructor.
	synthv1widget_env(QWidget *pParent = nullptr);
	// Destructor.
	~synthv1widget_env();

	// Parameter getters.
	float attack() const;
	float decay() const;
	float sustain() const;
	float release() const;

public slots:

	// Parameter setters.
	void setAttack(float fAttack);
	void setDecay(float fDecay);
	void setSustain(float fSustain);
	void setRelease(float fRelease);

signals:

	// Parameter change signals.
	void attackChanged(float fAttack);
	void decayChanged(float fDecay);
	void sustainChanged(float fSustain);
	void releaseChanged(float fRelease);

protected:

	// Draw canvas.
	void paintEvent(QPaintEvent *);

	// Draw rectangular point.
	QRect nodeRect(int iNode) const;
	int nodeIndex(const QPoint& pos) const;

	void dragNode(const QPoint& pos);

	// Mouse interaction.
	void mousePressEvent(QMouseEvent *pMouseEvent);
	void mouseMoveEvent(QMouseEvent *pMouseEvent);
	void mouseReleaseEvent(QMouseEvent *pMouseEvent);

private:

	// Instance state.
	float m_fAttack;
	float m_fDecay;
	float m_fSustain;
	float m_fRelease;

	// Draw state.
	QPolygon m_poly;

	// Drag state.
	int    m_iDragNode;
	QPoint m_posDrag;
};

#endif	// __synthv1widget_env_h


// end of synthv1widget_env.h
