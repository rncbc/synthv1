// synthv1.h
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

#ifndef __synthv1_h
#define __synthv1_h

#include "config.h"

#include <stdint.h>

// forward declarations
class synthv1_impl;
class synthv1_controls;
class synthv1_programs;


//-------------------------------------------------------------------------
// synthv1 - decl.
//

class synthv1
{
public:

	synthv1(uint16_t nchannels = 2, float srate = 44100.0f);

	~synthv1();

	void setChannels(uint16_t nchannels);
	uint16_t channels() const;

	void setSampleRate(float srate);
	float sampleRate() const;

	void setBufferSize(uint32_t nsize);
	uint32_t bufferSize() const;

	enum ParamIndex	 {

		DCO1_SHAPE1 = 0,
		DCO1_WIDTH1,
		DCO1_BANDL1,
		DCO1_SHAPE2,
		DCO1_WIDTH2,
		DCO1_BANDL2,
		DCO1_BALANCE,
		DCO1_DETUNE,
		DCO1_PHASE,
		DCO1_RINGMOD,
		DCO1_OCTAVE,
		DCO1_TUNING,
		DCO1_GLIDE,
		DCO1_ENVTIME,
		DCF1_CUTOFF,
		DCF1_RESO,
		DCF1_TYPE,
		DCF1_SLOPE,
		DCF1_ENVELOPE,
		DCF1_ATTACK,
		DCF1_DECAY,
		DCF1_SUSTAIN,
		DCF1_RELEASE,
		LFO1_SHAPE,
		LFO1_WIDTH,
		LFO1_BPM,
		LFO1_RATE,
		LFO1_SYNC,
		LFO1_SWEEP,
		LFO1_PITCH,
		LFO1_RINGMOD,
		LFO1_CUTOFF,
		LFO1_RESO,
		LFO1_PANNING,
		LFO1_VOLUME,
		LFO1_ATTACK,
		LFO1_DECAY,
		LFO1_SUSTAIN,
		LFO1_RELEASE,
		LFO1_BPMSYNC,
		DCA1_VOLUME,
		DCA1_ATTACK,
		DCA1_DECAY,
		DCA1_SUSTAIN,
		DCA1_RELEASE,
		OUT1_WIDTH,
		OUT1_PANNING,
		OUT1_FXSEND,
		OUT1_VOLUME,

		DEF1_PITCHBEND,
		DEF1_MODWHEEL,
		DEF1_PRESSURE,
		DEF1_VELOCITY,
		DEF1_CHANNEL,
		DEF1_MONO,

		DCO2_SHAPE1,
		DCO2_WIDTH1,
		DCO2_BANDL1,
		DCO2_SHAPE2,
		DCO2_WIDTH2,
		DCO2_BANDL2,
		DCO2_BALANCE,
		DCO2_DETUNE,
		DCO2_PHASE,
		DCO2_RINGMOD,
		DCO2_OCTAVE,
		DCO2_TUNING,
		DCO2_GLIDE,
		DCO2_ENVTIME,
		DCF2_CUTOFF,
		DCF2_RESO,
		DCF2_TYPE,
		DCF2_SLOPE,
		DCF2_ENVELOPE,
		DCF2_ATTACK,
		DCF2_DECAY,
		DCF2_SUSTAIN,
		DCF2_RELEASE,
		LFO2_SHAPE,
		LFO2_WIDTH,
		LFO2_BPM,
		LFO2_RATE,
		LFO2_SYNC,
		LFO2_SWEEP,
		LFO2_PITCH,
		LFO2_RINGMOD,
		LFO2_CUTOFF,
		LFO2_RESO,
		LFO2_PANNING,
		LFO2_VOLUME,
		LFO2_ATTACK,
		LFO2_DECAY,
		LFO2_SUSTAIN,
		LFO2_RELEASE,
		LFO2_BPMSYNC,
		DCA2_VOLUME,
		DCA2_ATTACK,
		DCA2_DECAY,
		DCA2_SUSTAIN,
		DCA2_RELEASE,
		OUT2_WIDTH,
		OUT2_PANNING,
		OUT2_FXSEND,
		OUT2_VOLUME,

		DEF2_PITCHBEND,
		DEF2_MODWHEEL,
		DEF2_PRESSURE,
		DEF2_VELOCITY,
		DEF2_CHANNEL,
		DEF2_MONO,

		CHO1_WET,
		CHO1_DELAY,
		CHO1_FEEDB,
		CHO1_RATE,
		CHO1_MOD,
		FLA1_WET,
		FLA1_DELAY,
		FLA1_FEEDB,
		FLA1_DAFT,
		PHA1_WET,
		PHA1_RATE,
		PHA1_FEEDB,
		PHA1_DEPTH,
		PHA1_DAFT,
		DEL1_WET,
		DEL1_DELAY,
		DEL1_FEEDB,
		DEL1_BPM,
		DEL1_BPMSYNC,
		REV1_WET,
		REV1_ROOM,
		REV1_DAMP,
		REV1_FEEDB,
		REV1_WIDTH,
		DYN1_COMPRESS,
		DYN1_LIMITER,

		NUM_PARAMS
	};

	void setParamPort(ParamIndex index, float *pfParam);
	float *paramPort(ParamIndex index) const;

	synthv1_controls *controls() const;
	synthv1_programs *programs() const;

	void setParamValue(ParamIndex index, float fValue);
	float paramValue(ParamIndex index) const;

	void reset();

	void process_midi(uint8_t *data, uint32_t size);
	void process(float **ins, float **outs, uint32_t nframes);

	static float lfo_rate_bpm(float bpm);

private:

	synthv1_impl *m_pImpl;
};


#endif// __synthv1_h

// end of synthv1.h
