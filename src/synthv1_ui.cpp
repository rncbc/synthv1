// synthv1_ui.cpp
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

#include "synthv1_ui.h"


//-------------------------------------------------------------------------
// synthv1_ui - decl.
//

synthv1_ui::synthv1_ui ( synthv1 *pSynth ) : m_pSynth(pSynth)
{
}


synthv1 *synthv1_ui::instance (void) const
{
	return m_pSynth;
}


void synthv1_ui::setParamValue ( synthv1::ParamIndex index, float fValue )
{
	m_pSynth->setParamValue(index, fValue);
}

float synthv1_ui::paramValue ( synthv1::ParamIndex index ) const
{
	return m_pSynth->paramValue(index);
}


synthv1_controls *synthv1_ui::controls (void) const
{
	return m_pSynth->controls();
}


synthv1_programs *synthv1_ui::programs (void) const
{
	return m_pSynth->programs();
}


void synthv1_ui::reset (void)
{
	return m_pSynth->reset();
}


void synthv1_ui::updatePreset ( bool bDirty )
{
	m_pSynth->updatePreset(bDirty);
}


void synthv1_ui::midiInEnabled ( bool bEnabled )
{
	m_pSynth->midiInEnabled(bEnabled);
}


uint32_t synthv1_ui::midiInCount (void)
{
	return m_pSynth->midiInCount();
}


// end of synthv1_ui.cpp
