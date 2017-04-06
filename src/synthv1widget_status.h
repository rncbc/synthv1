// synthv1widget_status.h
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

#ifndef __synthv1widget_status_h
#define __synthv1widget_status_h

#include <QStatusBar>


// Forward declarations.
class QLabel;
class QPixmap;


//-------------------------------------------------------------------------
// synthv1widget_status - Custom status-bar widget.

class synthv1widget_status : public QStatusBar
{
	Q_OBJECT

public:

	// Constructor.
	synthv1widget_status(QWidget *pParent = 0);

	// Destructor.
	~synthv1widget_status();

	// Permanent widgets accessors.
	void midiInLed(bool bMidiInLed);
	void modified(bool bModified);

private:

	// Permanent widgets.
	QPixmap *m_midiInLed[2];

	QLabel *m_pMidiInLedLabel;
	QLabel *m_pModifiedLabel;
};


#endif  // __synthv1widget_status_h

// end of synthv1widget_status.h
