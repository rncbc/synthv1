// synthv1_param.cpp
//
/****************************************************************************
   Copyright (C) 2012-2016, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "synthv1_param.h"
#include "synthv1_config.h"

#include <QHash>

#include <QDomDocument>
#include <QTextStream>
#include <QDir>

#include <math.h>


//-------------------------------------------------------------------------
// state params description.

enum ParamType { PARAM_FLOAT = 0, PARAM_INT, PARAM_BOOL };

static
struct ParamInfo {

	const char *name;
	ParamType type;
	float def;
	float min;
	float max;

} synthv1_params[synthv1::NUM_PARAMS] = {

	// name            type,           def,    min,    max
	{ "DCO1_SHAPE1",   PARAM_INT,     1.0f,   0.0f,   4.0f }, // DCO1 Wave Shape 1
	{ "DCO1_WIDTH1",   PARAM_FLOAT,   1.0f,   0.0f,   1.0f }, // DCO1 Wave Width 1
	{ "DCO1_BANDL1",   PARAM_BOOL,    0.0f,   0.0f,   1.0f }, // DCO1 Wave Bandlimit 1
	{ "DCO1_SHAPE2",   PARAM_INT,     1.0f,   0.0f,   4.0f }, // DCO1 Wave Shape 2
	{ "DCO1_WIDTH2",   PARAM_FLOAT,   1.0f,   0.0f,   1.0f }, // DCO1 Width 2
	{ "DCO1_BANDL2",   PARAM_BOOL,    0.0f,   0.0f,   1.0f }, // DCO1 Wave Bandlimit 2
	{ "DCO1_BALANCE",  PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // DCO1 Balance
	{ "DCO1_DETUNE",   PARAM_FLOAT,   0.1f,   0.0f,   1.0f }, // DCO1 Detune
	{ "DCO1_PHASE",    PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // DCO1 Phase
	{ "DCO1_RINGMOD",  PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // DCO1 Ring Mod
	{ "DCO1_OCTAVE",   PARAM_FLOAT,   0.0f,  -4.0f,   4.0f }, // DCO1 Octave
	{ "DCO1_TUNING",   PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // DCO1 Tuning
	{ "DCO1_GLIDE",    PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // DCO1 Glide
	{ "DCO1_ENVTIME",  PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // DCO1 Env.Time
	{ "DCF1_CUTOFF",   PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // DCF1 Cutoff
	{ "DCF1_RESO",     PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // DCF1 Resonance
	{ "DCF1_TYPE",     PARAM_INT,     0.0f,   0.0f,   3.0f }, // DCF1 Type
	{ "DCF1_SLOPE",    PARAM_INT,     0.0f,   0.0f,   3.0f }, // DCF1 Slope
	{ "DCF1_ENVELOPE", PARAM_FLOAT,   1.0f,  -1.0f,   1.0f }, // DCF1 Envelope
	{ "DCF1_ATTACK",   PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // DCF1 Attack
	{ "DCF1_DECAY",    PARAM_FLOAT,   0.2f,   0.0f,   1.0f }, // DCF1 Decay
	{ "DCF1_SUSTAIN",  PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // DCF1 Sustain
	{ "DCF1_RELEASE",  PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // DCF1 Release
	{ "LFO1_SHAPE",    PARAM_INT,     1.0f,   0.0f,   4.0f }, // LFO1 Wave Shape
	{ "LFO1_WIDTH",    PARAM_FLOAT,   1.0f,   0.0f,   1.0f }, // LFO1 Wave Width
	{ "LFP1_BPM",      PARAM_FLOAT, 180.0f,   0.0f, 360.0f }, // LFO1 BPM
	{ "LFO1_RATE",     PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // LFO1 Rate
	{ "LFO1_SYNC",     PARAM_BOOL,    0.0f,   0.0f,   1.0f }, // LFO1 Sync
	{ "LFO1_SWEEP",    PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // LFO1 Sweep
	{ "LFO1_PITCH",    PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // LFO1 Pitch
	{ "LFO1_RINGMOD",  PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // LFO1 Ring Mod
	{ "LFO1_CUTOFF",   PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // LFO1 Cutoff
	{ "LFO1_RESO",     PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // LFO1 Resonance
	{ "LFO1_PANNING",  PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // LFO1 Panning
	{ "LFO1_VOLUME",   PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // LFO1 Volume
	{ "LFO1_ATTACK",   PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // LFO1 Attack
	{ "LFO1_DECAY",    PARAM_FLOAT,   0.1f,   0.0f,   1.0f }, // LFO1 Decay
	{ "LFO1_SUSTAIN",  PARAM_FLOAT,   1.0f,   0.0f,   1.0f }, // LFO1 Sustain
	{ "LFO1_RELEASE",  PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // LFO1 Release
	{ "DCA1_VOLUME",   PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // DCA1 Volume
	{ "DCA1_ATTACK",   PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // DCA1 Attack
	{ "DCA1_DECAY",    PARAM_FLOAT,   0.1f,   0.0f,   1.0f }, // DCA1 Decay
	{ "DCA1_SUSTAIN",  PARAM_FLOAT,   1.0f,   0.0f,   1.0f }, // DCA1 Sustain
	{ "DCA1_RELEASE",  PARAM_FLOAT,   0.1f,   0.0f,   1.0f }, // DCA1 Release
	{ "OUT1_WIDTH",    PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // OUT1 Stereo Width
	{ "OUT1_PANNING",  PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // OUT1 Panning
	{ "OUT1_FXSEND",   PARAM_FLOAT,   1.0f,   0.0f,   1.0f }, // OUT1 FX Send
	{ "OUT1_VOLUME",   PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // OUT1 Volume

	{ "DEF1_PITCHBEND",PARAM_FLOAT,   0.2f,   0.0f,   1.0f }, // DEF1 Pitchbend
	{ "DEF1_MODWHEEL", PARAM_FLOAT,   0.2f,   0.0f,   1.0f }, // DEF1 Modwheel
	{ "DEF1_PRESSURE", PARAM_FLOAT,   0.2f,   0.0f,   1.0f }, // DEF1 Pressure
	{ "DEF1_VELOCITY", PARAM_FLOAT,   0.2f,   0.0f,   1.0f }, // DEF1 Velocity
	{ "DEF1_CHANNEL",  PARAM_INT,     0.0f,   0.0f,  16.0f }, // DEF1 Channel
	{ "DEF1_MONO",     PARAM_BOOL,    0.0f,   0.0f,   1.0f }, // DEF1 Mono

	{ "DCO2_SHAPE1",   PARAM_INT,     1.0f,   0.0f,   4.0f }, // DCO2 Wave Shape 1
	{ "DCO2_WIDTH1",   PARAM_FLOAT,   1.0f,   0.0f,   1.0f }, // DCO2 Wave Width 1
	{ "DCO2_BANDL1",   PARAM_BOOL,    0.0f,   0.0f,   1.0f }, // DCO2 Wave Bandlimit 1
	{ "DCO2_SHAPE2",   PARAM_INT,     1.0f,   0.0f,   4.0f }, // DCO2 Wave Shape 2
	{ "DCO2_WIDTH2",   PARAM_FLOAT,   1.0f,   0.0f,   1.0f }, // DCO2 Wave Width 2
	{ "DCO2_BANDL2",   PARAM_BOOL,    0.0f,   0.0f,   1.0f }, // DCO2 Wave Bandlimit 2
	{ "DCO2_BALANCE",  PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // DCO2 Balance
	{ "DCO2_DETUNE",   PARAM_FLOAT,   0.1f,   0.0f,   1.0f }, // DCO2 Detune
	{ "DCO2_PHASE",    PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // DCO2 Phase
	{ "DCO2_RINGMOD",  PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // DCO2 Ring Mod
	{ "DCO2_OCTAVE",   PARAM_FLOAT,  -2.0f,  -4.0f,   4.0f }, // DCO2 Octave
	{ "DCO2_TUNING",   PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // DCO2 Tuning
	{ "DCO2_GLIDE",    PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // DCO2 Glide
	{ "DCO2_ENVTIME",  PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // DCO2 Env.Time
	{ "DCF2_CUTOFF",   PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // DCF2 Cutoff
	{ "DCF2_RESO",     PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // DCF2 Resonance
	{ "DCF2_TYPE",     PARAM_INT,     0.0f,   0.0f,   3.0f }, // DCF2 Type
	{ "DCF2_SLOPE",    PARAM_INT,     0.0f,   0.0f,   3.0f }, // DCF2 Slope
	{ "DCF2_ENVELOPE", PARAM_FLOAT,   1.0f,  -1.0f,   1.0f }, // DCF2 Envelope
	{ "DCF2_ATTACK",   PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // DCF2 Attack
	{ "DCF2_DECAY",    PARAM_FLOAT,   0.2f,   0.0f,   1.0f }, // DCF2 Decay
	{ "DCF2_SUSTAIN",  PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // DCF2 Sustain
	{ "DCF2_RELEASE",  PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // DCF2 Release
	{ "LFO2_SHAPE",    PARAM_INT,     1.0f,   0.0f,   4.0f }, // LFO2 Wave Shape
	{ "LFO2_WIDTH",    PARAM_FLOAT,   1.0f,   0.0f,   1.0f }, // LFO2 Wave Width
	{ "LFP2_BPM",      PARAM_FLOAT, 180.0f,   0.0f, 360.0f }, // LFO2 BPM
	{ "LFO2_RATE",     PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // LFO2 Rate
	{ "LFO2_SYNC",     PARAM_BOOL,    0.0f,   0.0f,   1.0f }, // LFO2 Sync
	{ "LFO2_SWEEP",    PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // LFO2 Sweep
	{ "LFO2_PITCH",    PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // LFO2 Pitch
	{ "LFO2_RINGMOD",  PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // LFO2 Ring Mod
	{ "LFO2_CUTOFF",   PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // LFO2 Cutoff
	{ "LFO2_RESO",     PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // LFO2 Resonance
	{ "LFO2_PANNING",  PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // LFO2 Panning
	{ "LFO2_VOLUME",   PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // LFO2 Volume
	{ "LFO2_ATTACK",   PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // LFO2 Attack
	{ "LFO2_DECAY",    PARAM_FLOAT,   0.1f,   0.0f,   1.0f }, // LFO2 Decay
	{ "LFO2_SUSTAIN",  PARAM_FLOAT,   1.0f,   0.0f,   1.0f }, // LFO2 Sustain
	{ "LFO2_RELEASE",  PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // LFO2 Release
	{ "DCA2_VOLUME",   PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // DCA2 Volume
	{ "DCA2_ATTACK",   PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // DCA2 Attack
	{ "DCA2_DECAY",    PARAM_FLOAT,   0.1f,   0.0f,   1.0f }, // DCA2 Decay
	{ "DCA2_SUSTAIN",  PARAM_FLOAT,   1.0f,   0.0f,   1.0f }, // DCA2 Sustain
	{ "DCA2_RELEASE",  PARAM_FLOAT,   0.1f,   0.0f,   1.0f }, // DCA2 Release
	{ "OUT2_WIDTH",    PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // OUT2 Stereo Width
	{ "OUT2_PANNING",  PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // OUT2 Panning
	{ "OUT2_FXSEND",   PARAM_FLOAT,   1.0f,   0.0f,   1.0f }, // OUT2 FX Send
	{ "OUT2_VOLUME",   PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // OUT2 Volume

	{ "DEF2_PITCHBEND",PARAM_FLOAT,   0.2f,   0.0f,   1.0f }, // DEF2 Pitchbend
	{ "DEF2_MODWHEEL", PARAM_FLOAT,   0.2f,   0.0f,   1.0f }, // DEF2 Modwheel
	{ "DEF2_PRESSURE", PARAM_FLOAT,   0.2f,   0.0f,   1.0f }, // DEF2 Pressure
	{ "DEF2_VELOCITY", PARAM_FLOAT,   0.2f,   0.0f,   1.0f }, // DEF2 Velocity
	{ "DEF2_CHANNEL",  PARAM_INT,     0.0f,   0.0f,  16.0f }, // DEF2 Channel
	{ "DEF2_MONO",     PARAM_BOOL,    0.0f,   0.0f,   1.0f }, // DEF2 Mono

	{ "CHO1_WET",      PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // Chorus Wet
	{ "CHO1_DELAY",    PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // Chorus Delay
	{ "CHO1_FEEDB",    PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // Chorus Feedback
	{ "CHO1_RATE",     PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // Chorus Rate
	{ "CHO1_MOD",      PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // Chorus Modulation
	{ "FLA1_WET",      PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // Flanger Wet
	{ "FLA1_DELAY",    PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // Flanger Delay
	{ "FLA1_FEEDB",    PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // Flanger Feedback
	{ "FLA1_DAFT",     PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // Flanger Daft
	{ "PHA1_WET",      PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // Phaser Wet
	{ "PHA1_RATE",     PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // Phaser Rate
	{ "PHA1_FEEDB",    PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // Phaser Feedback
	{ "PHA1_DEPTH",    PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // Phaser Depth
	{ "PHA1_DAFT",     PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // Phaser Daft
	{ "DEL1_WET",      PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // Delay Wet
	{ "DEL1_DELAY",    PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // Delay Delay
	{ "DEL1_FEEDB",    PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // Delay Feedback
	{ "DEL1_BPM",      PARAM_FLOAT, 180.0f,   0.0f, 360.0f }, // Delay BPM
	{ "REV1_WET",      PARAM_FLOAT,   0.0f,   0.0f,   1.0f }, // Reverb Wet
	{ "REV1_ROOM",     PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // Reverb Room
	{ "REV1_DAMP",     PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // Reverb Damp
	{ "REV1_FEEDB",    PARAM_FLOAT,   0.5f,   0.0f,   1.0f }, // Reverb Feedback
	{ "REV1_WIDTH",    PARAM_FLOAT,   0.0f,  -1.0f,   1.0f }, // Reverb Width
	{ "DYN1_COMPRESS", PARAM_BOOL,    0.0f,   0.0f,   1.0f }, // Dynamic Compressor
	{ "DYN1_LIMITER",  PARAM_BOOL,    1.0f,   0.0f,   1.0f }  // Dynamic Limiter
};


const char *synthv1_param::paramName ( synthv1::ParamIndex index )
{
	return synthv1_params[index].name;
}


float synthv1_param::paramDefaultValue ( synthv1::ParamIndex index )
{
	return synthv1_params[index].def;
}


float synthv1_param::paramValue ( synthv1::ParamIndex index, float fScale )
{
	const ParamInfo& param = synthv1_params[index];

	if (param.type == PARAM_BOOL)
		return (fScale > 0.5f ? 1.0f : 0.0f);

	const float fValue = param.min + fScale * (param.max - param.min);

	if (param.type == PARAM_INT)
		return ::rintf(fValue);
	else
		return fValue;
}


float synthv1_param::paramScale ( synthv1::ParamIndex index, float fValue )
{
	const ParamInfo& param = synthv1_params[index];

	if (param.type == PARAM_BOOL)
		return (fValue > 0.5f ? 1.0f : 0.0f);

	const float fScale = (fValue - param.min) / (param.max - param.min);

	if (param.type == PARAM_INT)
		return ::rintf(fScale);
	else
		return fScale;
}


bool synthv1_param::paramFloat ( synthv1::ParamIndex index )
{
	return (synthv1_params[index].type == PARAM_FLOAT);
}



// Preset serialization methods.
void synthv1_param::loadPreset ( synthv1 *pSynth, const QString& sFilename )
{
	if (pSynth == NULL)
		return;

	QFileInfo fi(sFilename);
	if (!fi.exists()) {
		synthv1_config *pConfig = synthv1_config::getInstance();
		if (pConfig) {
			const QString& sPresetFile
				= pConfig->presetFile(sFilename);
			if (sPresetFile.isEmpty())
				return;
			fi.setFile(sPresetFile);
			if (!fi.exists())
				return;
		}
	}

	QFile file(fi.filePath());
	if (!file.open(QIODevice::ReadOnly))
		return;

	static QHash<QString, synthv1::ParamIndex> s_hash;
	if (s_hash.isEmpty()) {
		for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i) {
			const synthv1::ParamIndex index = synthv1::ParamIndex(i);
			s_hash.insert(synthv1_param::paramName(index), index);
		}
	}

	QDomDocument doc(SYNTHV1_TITLE);
	if (doc.setContent(&file)) {
		QDomElement ePreset = doc.documentElement();
		if (ePreset.tagName() == "preset") {
		//	&& ePreset.attribute("name") == fi.completeBaseName()) {
			for (QDomNode nChild = ePreset.firstChild();
					!nChild.isNull();
						nChild = nChild.nextSibling()) {
				QDomElement eChild = nChild.toElement();
				if (eChild.isNull())
					continue;
				if (eChild.tagName() == "params") {
					for (QDomNode nParam = eChild.firstChild();
							!nParam.isNull();
								nParam = nParam.nextSibling()) {
						QDomElement eParam = nParam.toElement();
						if (eParam.isNull())
							continue;
						if (eParam.tagName() == "param") {
							synthv1::ParamIndex index = synthv1::ParamIndex(
								eParam.attribute("index").toULong());
							const QString& sName = eParam.attribute("name");
							if (!sName.isEmpty()) {
								if (!s_hash.contains(sName))
									continue;
								index = s_hash.value(sName);
							}
							const float fValue = eParam.text().toFloat();
							pSynth->setParamValue(index, fValue);
						}
					}
				}
			}
		}
	}

	file.close();

	pSynth->reset();
}


void synthv1_param::savePreset ( synthv1 *pSynth, const QString& sFilename )
{
	if (pSynth == NULL)
		return;

	const QFileInfo fi(sFilename);

	QDomDocument doc(SYNTHV1_TITLE);
	QDomElement ePreset = doc.createElement("preset");
	ePreset.setAttribute("name", fi.completeBaseName());
	ePreset.setAttribute("version", SYNTHV1_VERSION);

	QDomElement eParams = doc.createElement("params");
	for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i) {
		QDomElement eParam = doc.createElement("param");
		const synthv1::ParamIndex index = synthv1::ParamIndex(i);
		eParam.setAttribute("index", QString::number(i));
		eParam.setAttribute("name", synthv1_param::paramName(index));
		const float fValue = pSynth->paramValue(index);
		eParam.appendChild(doc.createTextNode(QString::number(fValue)));
		eParams.appendChild(eParam);
	}
	ePreset.appendChild(eParams);
	doc.appendChild(ePreset);

	QFile file(fi.filePath());
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		QTextStream(&file) << doc.toString();
		file.close();
	}
}


// end of synthv1_param.cpp
