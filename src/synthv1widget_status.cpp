// synthv1widget_status.cpp
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

#include "synthv1widget_status.h"

#include <QLabel>


//-------------------------------------------------------------------------
// synthv1widget_status - Custom status-bar widget.
//

// Constructor.
synthv1widget_status::synthv1widget_status ( QWidget *pParent )
	: QStatusBar (pParent)
{
	m_midiInLed.addPixmap(
		QPixmap(":/images/ledOff.png"), QIcon::Normal, QIcon::Off);
	m_midiInLed.addPixmap(
		QPixmap(":/images/ledOn.png"), QIcon::Normal, QIcon::On);

	const QString sMidiIn(tr("MIDI In"));
	m_pMidiInLedLabel = new QLabel();
	m_pMidiInLedLabel->setAlignment(Qt::AlignHCenter);
	m_pMidiInLedLabel->setPixmap(m_midiInLed.pixmap(16, 16));
	m_pMidiInLedLabel->setToolTip(tr("%1 status").arg(sMidiIn));
	m_pMidiInLedLabel->setAutoFillBackground(true);
	QStatusBar::addWidget(m_pMidiInLedLabel);
	QStatusBar::addWidget(new QLabel(sMidiIn));

	const QFontMetrics fm(QStatusBar::font());
	m_pModifiedLabel = new QLabel();
	m_pModifiedLabel->setAlignment(Qt::AlignHCenter);
	m_pModifiedLabel->setMinimumSize(QSize(fm.width("MOD") + 4, fm.height()));
	m_pModifiedLabel->setToolTip(tr("Modification status"));
	m_pModifiedLabel->setAutoFillBackground(true);
	QStatusBar::addPermanentWidget(m_pModifiedLabel);
}


// Permanent widgets accessors.
void synthv1widget_status::midiInLed ( bool bMidiInLed )
{
	if (bMidiInLed) {
		m_pMidiInLedLabel->setPixmap(
			m_midiInLed.pixmap(16, 16, QIcon::Normal, QIcon::On));
	} else {
		m_pMidiInLedLabel->setPixmap(
			m_midiInLed.pixmap(16, 16, QIcon::Normal, QIcon::Off));
	}
}


void synthv1widget_status::modified ( bool bModified )
{
	if (bModified)
		m_pModifiedLabel->setText(tr("MOD"));
	else
		m_pModifiedLabel->clear();
}


// end of synthv1widget_status.cpp
