// synthv1_ui.cpp
//
/****************************************************************************
   Copyright (C) 2012-2018, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "synthv1_param.h"


//-------------------------------------------------------------------------
// synthv1_ui - decl.
//

synthv1_ui::synthv1_ui ( synthv1 *pSynth, bool bPlugin )
	: m_pSynth(pSynth), m_bPlugin(bPlugin)
{
}


synthv1 *synthv1_ui::instance (void) const
{
	return m_pSynth;
}


bool synthv1_ui::isPlugin (void) const
{
	return m_bPlugin;
}


bool synthv1_ui::loadPreset ( const QString& sFilename )
{
	return synthv1_param::loadPreset(m_pSynth, sFilename);
}

bool synthv1_ui::savePreset ( const QString& sFilename )
{
	return synthv1_param::savePreset(m_pSynth, sFilename);
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


void synthv1_ui::directNoteOn ( int note, int vel )
{
	m_pSynth->directNoteOn(note, vel);
}


void synthv1_ui::updateTuning (void)
{
	m_pSynth->updateTuning();
}


// MIDI note/octave name helper (static).
QString synthv1_ui::noteName ( int note )
{
	static const char *notes[] =
		{ "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };
	return QString("%1 %2").arg(notes[note % 12]).arg((note / 12) - 1);
}


// end of synthv1_ui.cpp
