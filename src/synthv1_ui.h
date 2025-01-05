// synthv1_ui.h
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

#ifndef __synthv1_ui_h
#define __synthv1_ui_h

#include "synthv1.h"

#include <QString>


//-------------------------------------------------------------------------
// synthv1_ui - decl.
//

class synthv1_ui
{
public:

	synthv1_ui(synthv1 *pSynth, bool bPlugin);

	synthv1 *instance() const;

	bool isPlugin() const;

	bool newPreset();

	bool loadPreset(const QString& sFilename);
	bool savePreset(const QString& sFilename);

	void setParamValue(synthv1::ParamIndex index, float fValue);
	float paramValue(synthv1::ParamIndex index) const;

	synthv1_controls *controls() const;
	synthv1_programs *programs() const;

	void reset();

	void updatePreset(bool bDirty);

	void updateParam(synthv1::ParamIndex index);

	void midiInEnabled(bool bEnabled);
	uint32_t midiInCount();

	void directNoteOn(int note, int vel);

	void setTuningEnabled(bool enabled);
	bool isTuningEnabled() const;

	void setTuningRefPitch(float refPitch);
	float tuningRefPitch() const;

	void setTuningRefNote(int refNote);
	int tuningRefNote() const;

	void setTuningScaleFile(const char *pszScaleFile);
	const char *tuningScaleFile() const;

	void setTuningKeyMapFile(const char *pszKeyMapFile);
	const char *tuningKeyMapFile() const;

	void resetTuning();

	// MIDI note/octave name helper.
	static QString noteName(int note);

private:

	synthv1 *m_pSynth;

	bool m_bPlugin;
};


#endif// __synthv1_ui_h

// end of synthv1_ui.h
