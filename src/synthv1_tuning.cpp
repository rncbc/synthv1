// synthv1_tuning.cpp
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

#include "synthv1_tuning.h"

#include <QTextStream>
#include <QFile>

#include <math.h>


// Default ctor.
synthv1_tuning::synthv1_tuning (void)
{
	reset();
}


// Default is 12-tone equal temperament, wstern standard mapping.
void synthv1_tuning::reset ( float refPitch, int refNote )
{
	m_refPitch = refPitch;
	m_refNote  = refNote;
	m_zeroNote = 0;

	m_scale.clear();

	for (int i = 0; i < 12; ++i)
		m_scale.push_back(::powf(2.0f, (i + 1) / 12.0f));

	m_mapRepeatInc = 1;

	m_mapping.clear();
	m_mapping.push_back(0);

	updateBasePitch();
}


void synthv1_tuning::updateBasePitch	(void)
{
	// Clever, huh?
	m_basePitch = 1.0f;
	m_basePitch = m_refPitch / noteToPitch(m_refNote);
}


// Load custom Scala key-map file (.kbm)
bool synthv1_tuning::loadKeyMapFile ( const QString& keyMapFile )
{
	QFile file(keyMapFile);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	QTextStream fs(&file);
	int   mapSize      = -1;
	int   firstNote    = -1;
	int   lastNote     = -1;
	int   zeroNote     = -1;
	int   refNote      = -1;
	float refPitch     = 0.0f;
	int   mapRepeatInc = -1;
	QVector<int> mapping;

	while (!fs.atEnd()) {
		const QString& line
			= fs.readLine().simplified();
		// Skip all-whitespace lines...
		if (line.isEmpty())
			continue;
		// Skip comment lines...
		if (line.at(0) == '!')
			continue;
		bool ok = false;
		const QString& val
			= line.section(' ', 0, 0);
		// An active range should be defined on this line...
		if (line.at(0) == '<') {
			// No overlap is checked for;
			// it wouldn't hurt anything if ranges overlapped.
			const int min = line.section(' ', 1, 1).toInt(&ok);
			if (!ok || min < 0)
				return false;
			ok = false;
			const int max = line.section(' ', 2, 2).toInt(&ok);
			if (!ok || max < min || max > 127)
				return false;
		}
		else
		if (mapSize < 0) {
			mapSize = val.toInt(&ok);
			if (!ok || mapSize < 0)
				return false;
		}
		else
		if (firstNote < 0) {
			firstNote = val.toInt(&ok);
			if (!ok || firstNote < 0 || firstNote > 127)
				return false;
		}
		else
		if (lastNote < 0) {
			lastNote = val.toInt(&ok);
			if (!ok || lastNote < 0 || lastNote > 127)
				return false;
		}
		else
		if (zeroNote < 0) {
			zeroNote = val.toInt(&ok);
			if (!ok || zeroNote < 0 || zeroNote > 127)
				return false;
		}
		else
		if (refNote < 0) {
			refNote = val.toInt(&ok);
			if (!ok || refNote < 0 || refNote > 127)
				return false;
		}
		else
		if (refPitch <= 0.0f) {
			refPitch = val.toFloat(&ok);
			if (!ok || refPitch < 0.001f)
				return false;
		}
		else
		if (mapRepeatInc < 0) {
			mapRepeatInc = val.toInt(&ok);
			if (!ok || mapRepeatInc < 0)
				return false;
		}
		else
		if (line.at(0).toLower() == 'x') {
			mapping.push_back(-1); // unmapped key
		}
		else {
			const int mapEntry = val.toInt(&ok);
			if (!ok || mapEntry < 0)
				return false;
			mapping.push_back(mapEntry);
		}
	}

	// Didn't get far enough?
	if (mapRepeatInc < 0)
		return false;

	// Special case for "automatic" linear mapping
	if (mapSize == 0) {
		if (!mapping.empty())
			return false;
		m_keyMapFile = keyMapFile;
		m_zeroNote = zeroNote;
		m_refNote  = refNote;
		m_refPitch = refPitch;
		m_mapRepeatInc = 1;
		m_mapping.clear();
		m_mapping.push_back(0);
		updateBasePitch();
		return true;
	}

	// Some of the kbm files included with Scala have
	// extra x's at the end for no good reason
	//if (mapping.size() > mapSize)
	//	return false;

	mapping.resize(mapSize);

	// Check to make sure reference pitch is actually mapped
	int refIndex = (refNote - zeroNote) % mapSize;
	if (refIndex < 0)
		refIndex += mapSize;
	if (mapping.at(refIndex) < 0)
		return false;

	m_keyMapFile = keyMapFile;
	m_zeroNote = zeroNote;
	m_refNote  = refNote;
	m_refPitch = refPitch;

	if (mapRepeatInc == 0)
		m_mapRepeatInc = mapSize;
	else
		m_mapRepeatInc = mapRepeatInc;

	m_mapping = mapping;

	updateBasePitch();
	return true;
}


// Load custom Scala scale file (.scl)
bool synthv1_tuning::loadScaleFile ( const QString& scaleFile )
{
	QFile file(scaleFile);
	if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
		return false;

	QTextStream fs(&file);
	QString scaleDesc;
	int scaleSize = -1;
	QVector<float> scale;

	while (!fs.atEnd()) {
		const QString& line
			= fs.readLine().simplified();
		// Skip all-whitespace lines after description...
		if (line.isEmpty() && !scaleDesc.isEmpty())
			continue;
		// Skip comment lines
		if (line.at(0) == '!')
			continue;
		if (scaleDesc.isEmpty())
			scaleDesc = line;
		else
		if (scaleSize < 0) {
			bool ok = false;
			scaleSize = line.section(' ', 0, 0).toInt(&ok);
			if (!ok || scaleSize < 0)
				return false;
		}
		else scale.push_back(parseScaleLine(line));
	}

	if (scaleDesc.isEmpty() || scale.size() != scaleSize)
		return false;

	m_scaleFile = scaleFile;
	m_scaleDesc = scaleDesc;

	m_scale = scale;

	updateBasePitch();
	return true;
}


// Convert a single line of a Scala scale file to a frequency relative to 1/1.
float synthv1_tuning::parseScaleLine ( const QString& line ) const
{
	bool ok = false;

	if (line.contains('.')) {
		// Treat as cents...
		const float cents = line.section(' ', 0, 0).toFloat(&ok);
		if (!ok || cents < 0.001f)
			return 0.0f;
		else
			return ::powf(2.0f, cents / 1200.0f);
	} else {
		// Treat as ratio...
		const long n = line.section('/', 0, 0).toLong(&ok);
		if (!ok || n < 0)
			return 0.0f;
		ok = false;
		const long d = line.section('/', 1, 1).toLong(&ok);
		if (!ok || d < 0)
			return 0.0f;
		else
			return float(n) / float(d);
	}
}


// The main pitch/frequency (Hz) getter.
float synthv1_tuning::noteToPitch ( int note ) const
{
	if (note < 0 || note > 127 || m_mapping.empty())
		return 0.0f;

	const int mapSize = m_mapping.size();

	int nRepeats = (note - m_zeroNote) / mapSize;
	int mapIndex = (note - m_zeroNote) % mapSize;

	if (mapIndex < 0) {
		mapIndex += mapSize;
		--nRepeats;
	}

	if (m_mapping.at(mapIndex) < 0)
		return 0.0f; // unmapped note

	const int scaleDegree = nRepeats * m_mapRepeatInc + m_mapping.at(mapIndex);
	const int scaleSize = m_scale.size();

	int nOctaves = scaleDegree / scaleSize;
	int scaleIndex = scaleDegree % scaleSize;

	if (scaleIndex < 0) {
		scaleIndex += scaleSize;
		--nOctaves;
	}

	const float pitch
		= m_basePitch * ::powf(m_scale.at(scaleSize - 1), nOctaves);
	if (scaleIndex > 0)
		return pitch * m_scale.at(scaleIndex - 1);
	else
		return pitch;
}


// end of synthv1_tuning.cpp
