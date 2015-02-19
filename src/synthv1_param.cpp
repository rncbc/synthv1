// synthv1_param.cpp
//
/****************************************************************************
   Copyright (C) 2012-2015, rncbc aka Rui Nuno Capela. All rights reserved.

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


//-------------------------------------------------------------------------
// default state (params)

static
struct {

	const char *name;
	float value;

} synthv1_default_params[synthv1::NUM_PARAMS] = {

	{ "DCO1_SHAPE1",    1.0f },
	{ "DCO1_WIDTH1",    1.0f },
	{ "DCO1_BANDL1",    0.0f },
	{ "DCO1_SHAPE2",    1.0f },
	{ "DCO1_WIDTH2",    1.0f },
	{ "DCO1_BANDL2",    0.0f },
	{ "DCO1_BALANCE",   0.0f },
	{ "DCO1_DETUNE",    0.1f },
	{ "DCO1_PHASE",     0.0f },
	{ "DCO1_OCTAVE",    0.0f },
	{ "DCO1_TUNING",    0.0f },
	{ "DCO1_GLIDE",     0.0f },
	{ "DCO1_ENVTIME",   0.5f },
	{ "DCF1_CUTOFF",    0.5f },
	{ "DCF1_RESO",      0.0f },
	{ "DCF1_TYPE",      0.0f },
	{ "DCF1_SLOPE",     0.0f },
	{ "DCF1_ENVELOPE",  1.0f },
	{ "DCF1_ATTACK",    0.0f },
	{ "DCF1_DECAY",     0.2f },
	{ "DCF1_SUSTAIN",   0.5f },
	{ "DCF1_RELEASE",   0.5f },
	{ "LFO1_SHAPE",     1.0f },
	{ "LFO1_WIDTH",     1.0f },
	{ "LFO1_RATE",      0.5f },
	{ "LFO1_SWEEP",     0.0f },
	{ "LFO1_PITCH",     0.0f },
	{ "LFO1_CUTOFF",    0.0f },
	{ "LFO1_RESO",      0.0f },
	{ "LFO1_PANNING",   0.0f },
	{ "LFO1_VOLUME",    0.0f },
	{ "LFO1_ATTACK",    0.0f },
	{ "LFO1_DECAY",     0.1f },
	{ "LFO1_SUSTAIN",   1.0f },
	{ "LFO1_RELEASE",   0.5f },
	{ "DCA1_VOLUME",    0.5f },
	{ "DCA1_ATTACK",    0.0f },
	{ "DCA1_DECAY",     0.1f },
	{ "DCA1_SUSTAIN",   1.0f },
	{ "DCA1_RELEASE",   0.1f },
	{ "OUT1_WIDTH",     0.0f },
	{ "OUT1_PANNING",   0.0f },
	{ "OUT1_VOLUME",    0.5f },

	{ "DEF1_PITCHBEND", 0.2f },
	{ "DEF1_MODWHEEL",  0.2f },
	{ "DEF1_PRESSURE",  0.2f },
	{ "DEF1_VELOCITY",  0.2f },
	{ "DEF1_CHANNEL",   0.0f },
	{ "DEF1_MONO",      0.0f },

	{ "DCO2_SHAPE1",    1.0f },
	{ "DCO2_WIDTH1",    1.0f },
	{ "DCO2_BANDL1",    0.0f },
	{ "DCO2_SHAPE2",    1.0f },
	{ "DCO2_WIDTH2",    1.0f },
	{ "DCO2_BANDL2",    0.0f },
	{ "DCO2_BALANCE",   0.0f },
	{ "DCO2_DETUNE",    0.1f },
	{ "DCO2_PHASE",     0.0f },
	{ "DCO2_OCTAVE",   -2.0f },
	{ "DCO2_TUNING",    0.0f },
	{ "DCO2_GLIDE",     0.0f },
	{ "DCO2_ENVTIME",   0.5f },
	{ "DCF2_CUTOFF",    0.5f },
	{ "DCF2_RESO",      0.0f },
	{ "DCF2_TYPE",      0.0f },
	{ "DCF2_SLOPE",     0.0f },
	{ "DCF2_ENVELOPE",  1.0f },
	{ "DCF2_ATTACK",    0.0f },
	{ "DCF2_DECAY",     0.2f },
	{ "DCF2_SUSTAIN",   0.5f },
	{ "DCF2_RELEASE",   0.5f },
	{ "LFO2_SHAPE",     1.0f },
	{ "LFO2_WIDTH",     1.0f },
	{ "LFO2_RATE",      0.5f },
	{ "LFO2_SWEEP",     0.0f },
	{ "LFO2_PITCH",     0.0f },
	{ "LFO2_CUTOFF",    0.0f },
	{ "LFO2_RESO",      0.0f },
	{ "LFO2_PANNING",   0.0f },
	{ "LFO2_VOLUME",    0.0f },
	{ "LFO2_ATTACK",    0.0f },
	{ "LFO2_DECAY",     0.1f },
	{ "LFO2_SUSTAIN",   1.0f },
	{ "LFO2_RELEASE",   0.5f },
	{ "DCA2_VOLUME",    0.5f },
	{ "DCA2_ATTACK",    0.0f },
	{ "DCA2_DECAY",     0.1f },
	{ "DCA2_SUSTAIN",   1.0f },
	{ "DCA2_RELEASE",   0.1f },
	{ "OUT2_WIDTH",     0.0f },
	{ "OUT2_PANNING",   0.0f },
	{ "OUT2_VOLUME",    0.5f },

	{ "DEF2_PITCHBEND", 0.2f },
	{ "DEF2_MODWHEEL",  0.2f },
	{ "DEF2_PRESSURE",  0.2f },
	{ "DEF2_VELOCITY",  0.2f },
	{ "DEF2_CHANNEL",   0.0f },
	{ "DEF2_MONO",      0.0f },

	{ "CHO1_WET",       0.0f },
	{ "CHO1_DELAY",     0.5f },
	{ "CHO1_FEEDB",     0.5f },
	{ "CHO1_RATE",      0.5f },
	{ "CHO1_MOD",       0.5f },
	{ "FLA1_WET",       0.0f },
	{ "FLA1_DELAY",     0.5f },
	{ "FLA1_FEEDB",     0.5f },
	{ "FLA1_DAFT",      0.0f },
	{ "PHA1_WET",       0.0f },
	{ "PHA1_RATE",      0.5f },
	{ "PHA1_FEEDB",     0.5f },
	{ "PHA1_DEPTH",     0.5f },
	{ "PHA1_DAFT",      0.0f },
	{ "DEL1_WET",       0.0f },
	{ "DEL1_DELAY",     0.5f },
	{ "DEL1_FEEDB",     0.5f },
	{ "DEL1_BPM",     180.0f },
	{ "DEL1_BPMSYNC",   0.0f },
	{ "DEL1_BPMHOST", 180.0f },
	{ "REV1_WET",       0.0f },
	{ "REV1_ROOM",      0.5f },
	{ "REV1_DAMP",      0.5f },
	{ "REV1_FEEDB",     0.5f },
	{ "REV1_WIDTH",     0.0f },
	{ "DYN1_COMPRESS",  0.0f },
	{ "DYN1_LIMITER",   1.0f }
};


const char *synthv1_param::paramName ( synthv1::ParamIndex index )
{
	return synthv1_default_params[index].name;
}


float synthv1_param::paramDefaultValue ( synthv1::ParamIndex index )
{
	return synthv1_default_params[index].value;
}


// Preset serialization methods.
void synthv1_param::loadPreset ( synthv1_ui *pSynthUi, const QString& sFilename )
{
	if (pSynthUi == NULL)
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
			synthv1::ParamIndex index = synthv1::ParamIndex(i);
			s_hash.insert(synthv1_param::paramName(index), index);
		}
	}

	QDomDocument doc(SYNTHV1_TITLE);
	if (doc.setContent(&file)) {
		QDomElement ePreset = doc.documentElement();
		if (ePreset.tagName() == "preset"
			&& ePreset.attribute("name") == fi.completeBaseName()) {
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
							float fValue = eParam.text().toFloat();
						#if 1//--legacy support < 0.3.0.4
							if (index == synthv1::DEL1_BPM && fValue < 3.6f)
								fValue *= 100.0f;
						#endif
							pSynthUi->setParamValue(index, fValue);
						}
					}
				}
			}
		}
	}

	file.close();

	pSynthUi->reset();
}


void synthv1_param::savePreset ( synthv1_ui *pSynthUi, const QString& sFilename )
{
	if (pSynthUi == NULL)
		return;

	const QFileInfo fi(sFilename);

	QDomDocument doc(SYNTHV1_TITLE);
	QDomElement ePreset = doc.createElement("preset");
	ePreset.setAttribute("name", fi.completeBaseName());
	ePreset.setAttribute("version", SYNTHV1_VERSION);

	QDomElement eParams = doc.createElement("params");
	for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i) {
		QDomElement eParam = doc.createElement("param");
		synthv1::ParamIndex index = synthv1::ParamIndex(i);
		eParam.setAttribute("index", QString::number(i));
		eParam.setAttribute("name", synthv1_param::paramName(index));
		const float fValue = pSynthUi->paramValue(index);
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
