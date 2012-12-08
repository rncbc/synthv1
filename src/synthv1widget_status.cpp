// synthv1widget_status.cpp
//
/****************************************************************************
   Copyright (C) 2012, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "synthv1widget_config.h"

#include <QLabel>


//-------------------------------------------------------------------------
// synthv1widget_status - Custom status-bar widget.
//

// Constructor.
synthv1widget_status::synthv1widget_status ( QWidget *pParent )
	: QStatusBar (pParent)
{
	const QFontMetrics fm(QStatusBar::font());
	m_pModifiedLabel = new QLabel();
	m_pModifiedLabel->setAlignment(Qt::AlignHCenter);
	m_pModifiedLabel->setMinimumSize(QSize(fm.width("MOD") + 4, fm.height()));
	m_pModifiedLabel->setToolTip(tr("Modification status"));
	m_pModifiedLabel->setAutoFillBackground(true);
	QStatusBar::addPermanentWidget(m_pModifiedLabel);
}


// Permanent widgets accessors.
void synthv1widget_status::setModified ( bool bModified )
{
	if (bModified)
		m_pModifiedLabel->setText(tr("MOD"));
	else
		m_pModifiedLabel->clear();
}


bool synthv1widget_status::isModified (void) const
{
	return !m_pModifiedLabel->text().isEmpty();
}


// end of synthv1widget_status.cpp
