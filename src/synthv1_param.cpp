// synthv1_param.cpp
//
/****************************************************************************
   Copyright (C) 2012-2013, rncbc aka Rui Nuno Capela. All rights reserved.

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


//-------------------------------------------------------------------------
// default state (params)

static
struct {

	const char *name;
	float value;

} synthv1_default_params[synthv1::NUM_PARAMS] = {

	{ "DCO1_SHAPE1",    1.0f },
	{ "DCO1_WIDTH1",    1.0f },
	{ "DCO1_SHAPE2",    1.0f },
	{ "DCO1_WIDTH2",    1.0f },
	{ "DCO1_BALANCE",   0.0f },
	{ "DCO1_DETUNE",    0.1f },
	{ "DCO1_PHASE",     0.0f },
	{ "DCO1_OCTAVE",    0.0f },
	{ "DCO1_TUNING",    0.0f },
	{ "DCO1_GLIDE",     0.0f },
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
	{ "DCO2_SHAPE2",    1.0f },
	{ "DCO2_WIDTH2",    1.0f },
	{ "DCO2_BALANCE",   0.0f },
	{ "DCO2_DETUNE",    0.1f },
	{ "DCO2_PHASE",     0.0f },
	{ "DCO2_OCTAVE",   -2.0f },
	{ "DCO2_TUNING",    0.0f },
	{ "DCO2_GLIDE",     0.0f },
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
	{ "DYN1_COMPRESS",  0.0f },
	{ "DYN1_LIMIT",     1.0f }
};


QString synthv1_param::paramName ( synthv1::ParamIndex index )
{
	return synthv1_default_params[index].name;
}


float synthv1_param::paramDefaultValue ( synthv1::ParamIndex index )
{
	return synthv1_default_params[index].value;
}


// end of synthv1_param.cpp
