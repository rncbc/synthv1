// synthv1_tuning.h
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

#ifndef __synthv1_tuning_h
#define __synthv1_tuning_h

#include <QString>
#include <QVector>

//-------------------------------------------------------------------------
// TuningMap
//
// -- borrowed, stirred and refactored from amsynth --
//    https://github.com/amsynth/amsynth
//    Copyright (C) 2001-2012 Nick Dowell
//

/*
 * A TuningMap consists of two parts.
 *
 * The "key map" maps from MIDI note numbers to logical note numbers
 * for the scale. This is often the identity mapping, but if your
 * scale has, for example, 11 notes in it, you'll want to skip one
 * per octave so the scale lines up with the pattern of keys on a
 * standard keyboard.
 *
 * The "scale" maps from those logical note numbers to actual pitches.
 * In terms of member variables, "scale" and "scaleDesc" belong to the
 * scale, and everything else belongs to the mapping.
 *
 * For more information, refer to http://www.huygens-fokker.org/scala/
 */

class synthv1_tuning
{
public:

	// Default reference note and pitch (A4 @440hz)
	synthv1_tuning(float refPitch = 440.0f, int refNote = 69);

	// Default 12-tone equal temperament, wstern standard mapping
	void reset(float refPitch, int refNote);

	// Reference note and pitch accessors.
	float refPitch() const { return m_refPitch; }
	int   refNote()  const { return m_refNote;  }

	// Load custom Scala key map file (.kbm)
	bool loadKeyMapFile (const QString& filename);

	// Load custom Scala scale file (.scl)
	bool loadScaleFile (const QString& filename);

	const QString& keyMapFile() const { return m_keyMapFile; }

	const QString& scaleFile() const { return m_scaleFile;  }
	const QString& scaleDesc() const { return m_scaleDesc;  }

	// The main pitch/frequency (Hz) getter
	float noteToPitch(int note) const;

protected:

	float parseScaleLine(const QString& line) const;

	void updateBasePitch();

private:

	// Instance member variables.
	QString m_keyMapFile;

	QString m_scaleFile;
	QString m_scaleDesc;

	QVector<float> m_scale;

	float m_refPitch;
	int   m_refNote;
	int   m_zeroNote;
	int   m_mapRepeatInc;
	float m_basePitch;

	QVector<int> m_mapping;
};


#endif	// __synthv1_tuning_h

// end of synthv1_tuning.h
