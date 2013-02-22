// synthv1.cpp
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

#include "synthv1.h"

#include "synthv1_wave.h"
#include "synthv1_ramp.h"

#include "synthv1_list.h"

#include "synthv1_fx.h"


#ifdef CONFIG_DEBUG_0
#include <stdio.h>
#endif

#include <string.h>


//-------------------------------------------------------------------------
// synthv1_impl
//
// -- borrowed and revamped from synth.h of synth4
//    Copyright (C) 2007 jorgen, linux-vst.com
//

const uint16_t MAX_VOICES = 32;			// polyphony
const uint8_t  MAX_NOTES  = 128;

const float MIN_ENV_MSECS = 2.0f;		// min 2msec per stage
const float MAX_ENV_MSECS = 5000.0f;	// max 5 sec per stage

const float DETUNE_SCALE  = 0.5f;
const float PHASE_SCALE   = 0.5f;
const float OCTAVE_SCALE  = 12.0f;
const float TUNING_SCALE  = 1.0f;
const float SWEEP_SCALE   = 0.5f;
const float PITCH_SCALE   = 0.5f;

const float LFO_FREQ_MIN  = 0.4f;
const float LFO_FREQ_MAX  = 40.0f;


// hyperbolic-tangent fast approximation
inline float synthv1_tanhf ( const float x )
{
	const float x2 = x * x;
	return x * (27.0f + x2) / (27.0f + 9.0f * x2);
}


// sigmoids

inline float synthv1_sigmoid ( const float x )
{
//	return 2.0f / (1.0f + ::expf(-5.0f * x)) - 1.0f;
	return synthv1_tanhf(2.0f * x);
}

inline float synthv1_sigmoid_0 ( const float x, const float t0 )
{
	const float t1 = 1.0f - t0;
#if 0
	return (x < -1.0f ? -t1 : (x > +1.0f ? t1 : t1 * x * (1.5f - 0.5f * x * x)));
#else
	if (x > +t1)
		return +t1 + t0 * synthv1_tanhf(+(x - t1) / t0);
	else
	if (x < -t1)
		return -t1 - t0 * synthv1_tanhf(-(x + t1) / t0);
	else
		return x;
#endif
}

inline float synthv1_sigmoid_1 ( const float x, const float t0 = 0.01f )
{
	return 0.5f * (1.0f + synthv1_sigmoid_0(2.0f * x - 1.0f, t0));
}


// velocity hard-split curve

inline float synthv1_velocity ( const float x, const float p = 0.2f )
{
	return ::powf(x, (1.0f - p));
}


// simplest power-of-2 straight linearization
// -- x argument valid in [-1, 1] interval
inline float synthv1_pitchbend ( const float x )
{
//	return ::powf(2.0f, x);
	return 1.0f + (x < 0.0f ? 0.5f : 1.0f) * x;
}


// envelope

struct synthv1_env
{
	// envelope stages

	enum Stage { Attack = 0, Decay, Sustain, Release, Done };

	// per voice

	struct State
	{
		// process
		float tick()
		{
			if (running && frames > 0) {
				phase += delta;
				value = c1 * phase * (2.0f - phase) + c0;
				--frames;
			}
			return value;
		}

		// state
		bool running;
		Stage stage;
		float phase;
		float delta;
		float value;
		float c1, c0;
		uint32_t frames;
	};

	void start(State *p)
	{
		p->running = true;
		p->stage = Attack;
		p->frames = uint32_t(*attack * *attack * max_frames);
		p->phase = 0.0f;
		if (p->frames > 0) {
			p->delta = 1.0f / float(p->frames);
			p->value = 0.0f;
		} else {
			p->delta = 0.0f;
			p->value = 1.0f;
		}
		p->c1 = 1.0f;
		p->c0 = 0.0f;
	}

	void next(State *p)
	{
		if (p->stage == Attack) {
			p->stage = Decay;
			p->frames = uint32_t(*decay * *decay * max_frames);
			if (p->frames < min_frames) // prevent click on too fast decay
				p->frames = min_frames;
			p->phase = 0.0f;
			p->delta = 1.0f / float(p->frames);
			p->c1 = *sustain - 1.0f;
			p->c0 = p->value;
		}
		else if (p->stage == Decay) {
			p->running = false; // stay at this stage until note_off received
			p->stage = Sustain;
			p->frames = 0;
			p->phase = 0.0f;
			p->delta = 0.0f;
			p->c1 = 0.0f;
			p->c0 = p->value;
		}
		else if (p->stage == Release) {
			p->running = false;
			p->stage = Done;
			p->frames = 0;
			p->phase = 0.0f;
			p->delta = 0.0f;
			p->value = 0.0f;
			p->c1 = 0.0f;
			p->c0 = 0.0f;
		}
	}

	void note_on(State *p)
	{
		p->running = true;
		p->stage = Attack;
		p->frames = uint32_t(*attack * *attack * max_frames);
		if (p->frames < min_frames) // prevent click on too fast attack
			p->frames = min_frames;
		p->phase = 0.0f;
		p->delta = 1.0f / float(p->frames);
		p->c1 = 1.0f - p->value;
		p->c0 = p->value;
	}

	void note_off(State *p)
	{
		p->running = true;
		p->stage = Release;
		p->frames = uint32_t(*release * *release * max_frames);
		if (p->frames < min_frames) // prevent click on too fast release
			p->frames = min_frames;
		p->phase = 0.0f;
		p->delta = 1.0f / float(p->frames);
		p->c1 = -(p->value);
		p->c0 = p->value;
	}

	void note_off_fast(State *p)
	{
		p->running = true;
		p->stage = Release;
		if (p->frames > min_frames)
			p->frames = min_frames;
		p->phase = 0.0f;
		p->delta = 1.0f / float(p->frames);
		p->c1 = -(p->value);
		p->c0 = p->value;
	}

	// parameters

	float *attack;
	float *decay;
	float *sustain;
	float *release;

	uint32_t min_frames;
	uint32_t max_frames;
};


// midi control

struct synthv1_ctl
{
	synthv1_ctl() { reset(); }

	void reset()
	{
		pressure = 0.0f;
		pitchbend1 = pitchbend2 = 1.0f;
		modwheel1 = modwheel2 = 0.0f;
		panning = 0.0f;
		volume = 1.0f;
		sustain = false;
	}

	float pressure;
	float pitchbend1, pitchbend2;
	float modwheel1, modwheel2;
	float panning;
	float volume;
	bool  sustain;
};


// internal control

struct synthv1_aux
{
	synthv1_aux() { reset(); }

	void reset()
	{
		panning = 0.0f;
		volume = 1.0f;
	}

	float panning;
	float volume;
};


// dco

struct synthv1_dco
{
	float *shape1;
	float *width1;
	float *shape2;
	float *width2;
	float *balance;
	float *detune;
	float *phase;
	float *octave;
	float *tuning;
	float *glide;
};


// dcf

struct synthv1_dcf
{
	float *cutoff;
	float *reso;
	float *type;
	float *slope;
	float *envelope;

	synthv1_env env;
};


// lfo

struct synthv1_lfo
{
	float *shape;
	float *width;
	float *rate;
	float *sweep;
	float *pitch;
	float *cutoff;
	float *reso;
	float *panning;
	float *volume;

	synthv1_env env;
};


// dca

struct synthv1_dca
{
	float *volume;

	synthv1_env env;
};



// def (ranges)

struct synthv1_def
{
	float *pitchbend;
	float *modwheel;
	float *pressure;
	float *velocity;
};


// out (mix)

struct synthv1_out
{
	float *width;
	float *panning;
	float *volume;
};


// chorus (fx)

struct synthv1_cho
{
	float *wet;
	float *delay;
	float *feedb;
	float *rate;
	float *mod;
};


// flanger (fx)

struct synthv1_fla
{
	float *wet;
	float *delay;
	float *feedb;
	float *daft;
};


// phaser (fx)

struct synthv1_pha
{
	float *wet;
	float *rate;
	float *feedb;
	float *depth;
	float *daft;
};


// delay (fx)

struct synthv1_del
{
	float *wet;
	float *delay;
	float *feedb;
	float *bpm;
};


// dynamic(compressor/limiter)

struct synthv1_dyn
{
	float *compress;
	float *limiter;
};


// (Hal Chamberlin's state variable) filter

class synthv1_filter1
{
public:

	enum Type { Low = 0, Band, High, Notch };

	synthv1_filter1(Type type = Low, uint16_t nover = 2)
		{ reset(type, nover); }

	Type type() const
		{ return m_type; }

	void reset(Type type = Low, uint16_t nover = 2)
	{
		m_type  = type;
		m_nover = nover;

		m_low   = 0.0f;
		m_band  = 0.0f;
		m_high  = 0.0f;
		m_notch = 0.0f;

		switch (m_type) {
		case Notch:
			m_out = &m_notch;
			break;
		case High:
			m_out = &m_high;
			break;
		case Band:
			m_out = &m_band;
			break;
		case Low:
		default:
			m_out = &m_low;
			break;
		}
	}

	float output(float input, float cutoff, float reso)
	{
		const float q = (1.0f - reso);

		for (uint16_t i = 0; i < m_nover; ++i) {
			m_low  += cutoff * m_band;
			m_high  = input - m_low - q * m_band;
			m_band += cutoff * m_high;
			m_notch = m_high + m_low;
		}

		return *m_out;
	}

private:

	Type     m_type;

	uint16_t m_nover;

	float    m_low;
	float    m_band;
	float    m_high;
	float    m_notch;

	float   *m_out;
};


// (Stilson/Smith Moog 24dB/oct) filter

class synthv1_filter2
{
public:

	enum Type { Low = 0, Band, High, Notch };

	synthv1_filter2(Type type = Low) { reset(type); }

	Type type() const
		{ return m_type; }

	void reset(Type type = Low)
	{
		m_type = type;

		m_b0 = m_b1 = m_b2 = m_b3 = m_b4 = 0.0f;
		m_t1 = m_t2 = m_t3 = 0.0f;
		m_f  = m_p  = m_q  = 0.0f;
	}

	float output(float input, float cutoff, float reso)
	{
		m_q = 1.0f - cutoff;
		m_p = cutoff + 0.8f * cutoff * m_q;
		m_f = m_p + m_p - 1.0f;
		m_q = reso * (1.0f + 0.5f * m_q * (1.0f - m_q + 5.6f * m_q * m_q));

		input -= m_q * m_b4;	// feedback

		m_t1 = m_b1; m_b1 = (input + m_b0) * m_p - m_b1 * m_f;
		m_t2 = m_b2; m_b2 = (m_b1 + m_t1) * m_p - m_b2 * m_f;
		m_t1 = m_b3; m_b3 = (m_b2 + m_t2) * m_p - m_b3 * m_f;

		m_b4 = (m_b3 + m_t1) * m_p - m_b4 * m_f;
	//	m_b4 = m_b4 - m_b4 * m_b4 * m_b4 * 0.166667f;	-- clipping

		m_b0 = input;

		switch (m_type) {
		case Notch:
			return 3.0f * (m_b3 - m_b4) - input;
		case High:
			return input - m_b4;
		case Band:
			return 3.0f * (m_b3 - m_b4);
		case Low:
		default:
			return m_b4;
		}
	}

private:

	Type   m_type;

	float  m_b0, m_b1, m_b2, m_b3, m_b4;
	float  m_t1, m_t2, m_t3;
	float  m_f,  m_p,  m_q;
};


// glide (portamento)

struct synthv1_glide
{
	synthv1_glide(float& last) : m_last(last) { reset(); }

	void reset( uint32_t frames = 0, float freq = 0.0f )
	{
		m_frames = frames;

		if (m_frames > 0) {
			m_freq = m_last - freq;
			m_step = m_freq / float(m_frames);
		} else {
			m_freq = 0.0f;
			m_step = 0.0f;
		}

		m_last = freq;
	}

	float tick()
	{
		if (m_frames > 0) {
			m_freq -= m_step;
			--m_frames;
		}
		return m_freq;
	}

private:

	uint32_t m_frames;

	float m_freq;
	float m_step;

	float& m_last;
};


// balancing smoother (1 parameter)

class synthv1_bal : public synthv1_ramp1
{
public:

	synthv1_bal() : synthv1_ramp1(2) {}

protected:

	virtual float evaluate(uint16_t i)
	{
		synthv1_ramp1::evaluate(i);

		const float wbal = 0.25f * M_PI * (1.0f + m_param1_v);

		return M_SQRT2 * (i == 0 ? ::cosf(wbal) : ::sinf(wbal));
	}
};


// panning smoother (3 parameters)

class synthv1_pan : public synthv1_ramp3
{
public:

	synthv1_pan() : synthv1_ramp3(2) {}

protected:

	virtual float evaluate(uint16_t i)
	{
		synthv1_ramp3::evaluate(i);

		const float wpan = 0.25f * M_PI
			* (1.0f + m_param1_v)
			* (1.0f + m_param2_v)
			* (1.0f + m_param3_v);

		return M_SQRT2 * (i == 0 ? ::cosf(wpan) : ::sinf(wpan));
	}
};


// a forward decl.

class synthv1_impl;


// voice

struct synthv1_voice : public synthv1_list<synthv1_voice>
{
	synthv1_voice(synthv1_impl *pImpl);

	synthv1_oscillator dco1_osc1, dco1_osc2;
	synthv1_oscillator dco2_osc1, dco2_osc2;

	synthv1_oscillator lfo1_osc,  lfo2_osc;

	int note;									// voice note

	float vel1, vel2;							// velocities to vol

	float dco1_freq1, dco1_sample1;				// frequency and phase
	float dco1_freq2, dco1_sample2;
	float dco2_freq1, dco2_sample1;
	float dco2_freq2, dco2_sample2;

	float lfo1_sample, lfo2_sample;

	synthv1_bal dco1_bal, dco2_bal;

	synthv1_filter1 dcf11, dcf12, dcf13, dcf14;	// filters
	synthv1_filter2 dcf21, dcf22, dcf23, dcf24;

	synthv1_env::State dca1_env, dca2_env;		// envelope states
	synthv1_env::State dcf1_env, dcf2_env;
	synthv1_env::State lfo1_env, lfo2_env;

	synthv1_glide dco1_glide1, dco1_glide2;	// glides (portamento)
	synthv1_glide dco2_glide1, dco2_glide2;

	bool sustain;
};


// polyphonic synth implementation

class synthv1_impl
{
public:

	synthv1_impl(uint16_t iChannels, uint32_t iSampleRate);

	~synthv1_impl();

	void setChannels(uint16_t iChannels);
	uint16_t channels() const;

	void setSampleRate(uint32_t iSampleRate);
	uint32_t sampleRate() const;

	void setParamPort(synthv1::ParamIndex index, float *pfParam = 0);
	float *paramPort(synthv1::ParamIndex index);

	void process_midi(uint8_t *data, uint32_t size);
	void process(float **ins, float **outs, uint32_t nframes);

	void reset();

	synthv1_wave dco1_wave1, dco1_wave2;
	synthv1_wave dco2_wave1, dco2_wave2;

	synthv1_wave lfo1_wave,  lfo2_wave;

	float dco1_last1;
	float dco1_last2;
	float dco2_last1;
	float dco2_last2;

protected:

	void allSoundOff();
	void allControllersOff();
	void allNotesOff();
	void allSustainOff();

	synthv1_voice *alloc_voice ()
	{
		synthv1_voice *pv = m_free_list.next();
		if (pv) {
			m_free_list.remove(pv);
			m_play_list.append(pv);
		}
		return pv;
	}

	void free_voice ( synthv1_voice *pv )
	{
		m_play_list.remove(pv);
		m_free_list.append(pv);
	}

private:

	uint16_t m_iChannels;
	uint32_t m_iSampleRate;

	synthv1_ctl m_ctl;

	synthv1_dco m_dco1, m_dco2;
	synthv1_dcf m_dcf1, m_dcf2;
	synthv1_lfo m_lfo1, m_lfo2;
	synthv1_dca m_dca1, m_dca2;
	synthv1_out m_out1, m_out2;

	synthv1_def m_def1, m_def2;

	synthv1_cho m_cho;
	synthv1_fla m_fla;
	synthv1_pha m_pha;
	synthv1_del m_del;
	synthv1_dyn m_dyn;

	synthv1_voice **m_voices;
	synthv1_voice  *m_notes[MAX_NOTES];

	synthv1_list<synthv1_voice> m_free_list;
	synthv1_list<synthv1_voice> m_play_list;

	synthv1_aux   m_aux1, m_aux2;

	synthv1_ramp1 m_wid1, m_wid2;
	synthv1_pan   m_pan1, m_pan2;
	synthv1_ramp4 m_vol1, m_vol2;

	synthv1_ramp2 m_pre1, m_pre2;

	synthv1_fx_chorus   m_chorus;
	synthv1_fx_flanger *m_flanger;
	synthv1_fx_phaser  *m_phaser;
	synthv1_fx_delay   *m_delay;
	synthv1_fx_comp    *m_comp;
};


// voice constructor

synthv1_voice::synthv1_voice ( synthv1_impl *pImpl ) :
	dco1_osc1(&pImpl->dco1_wave1),
	dco1_osc2(&pImpl->dco1_wave2),
	dco2_osc1(&pImpl->dco2_wave1),
	dco2_osc2(&pImpl->dco2_wave2),
	lfo1_osc(&pImpl->lfo1_wave),
	lfo2_osc(&pImpl->lfo2_wave),
	dco1_glide1(pImpl->dco1_last1),
	dco1_glide2(pImpl->dco1_last2),
	dco2_glide1(pImpl->dco2_last1),
	dco2_glide2(pImpl->dco2_last2)
{
}

// constructor

synthv1_impl::synthv1_impl ( uint16_t iChannels, uint32_t iSampleRate )
{
	// glide notes
	dco1_last1 = 0.0f;
	dco1_last2 = 0.0f;
	dco2_last1 = 0.0f;
	dco2_last2 = 0.0f;

	// allocate voice pool.
	m_voices = new synthv1_voice * [MAX_VOICES];

	for (int i = 0; i < MAX_VOICES; ++i) {
		m_voices[i] = new synthv1_voice(this);
		m_free_list.append(m_voices[i]);
	}

	for (int note = 0; note < MAX_NOTES; ++note)
		m_notes[note] = 0;

	// flangers none yet
	m_flanger = 0;

	// phasers none yet
	m_phaser = 0;

	// delays none yet
	m_delay = 0;

	// compressors none yet
	m_comp = 0;

	// number of channels
	setChannels(iChannels);

	// parameters
	for (int i = 0; i < int(synthv1::NUM_PARAMS); ++i)
		setParamPort(synthv1::ParamIndex(i));

	// set default sample rate
	setSampleRate(iSampleRate);

	// reset all voices
	allControllersOff();
	allNotesOff();
}


// destructor

synthv1_impl::~synthv1_impl (void)
{
	// deallocate voice pool.
	for (int i = 0; i < MAX_VOICES; ++i)
		delete m_voices[i];

	delete [] m_voices;

	// deallocate channels
	setChannels(0);
}


void synthv1_impl::setChannels ( uint16_t iChannels )
{
	m_iChannels = iChannels;

	// deallocate flangers
	if (m_flanger) {
		delete [] m_flanger;
		m_flanger = 0;
	}

	// deallocate phasers
	if (m_phaser) {
		delete [] m_phaser;
		m_phaser = 0;
	}

	// deallocate delays
	if (m_delay) {
		delete [] m_delay;
		m_delay = 0;
	}

	// deallocate compressors
	if (m_comp) {
		delete [] m_comp;
		m_comp = 0;
	}
}


uint16_t synthv1_impl::channels (void) const
{
	return m_iChannels;
}


void synthv1_impl::setSampleRate ( uint32_t iSampleRate )
{
	// set internal sample rate
	m_iSampleRate = iSampleRate;

	// update waves sample rate
	dco1_wave1.setSampleRate(m_iSampleRate);
	dco1_wave2.setSampleRate(m_iSampleRate);
	dco2_wave1.setSampleRate(m_iSampleRate);
	dco2_wave2.setSampleRate(m_iSampleRate);

	lfo1_wave.setSampleRate(m_iSampleRate);
	lfo2_wave.setSampleRate(m_iSampleRate);

	// update envelope range times in frames
	const float srate_ms = 0.001f * float(m_iSampleRate);

	const uint32_t min_frames = uint32_t(srate_ms * MIN_ENV_MSECS);
	const uint32_t max_frames = uint32_t(srate_ms * MAX_ENV_MSECS);

	m_dcf1.env.min_frames = min_frames;
	m_dcf1.env.max_frames = max_frames;

	m_dcf2.env.min_frames = min_frames;
	m_dcf2.env.max_frames = max_frames;

	m_lfo1.env.min_frames = min_frames;
	m_lfo1.env.max_frames = max_frames;

	m_lfo2.env.min_frames = min_frames;
	m_lfo2.env.max_frames = max_frames;

	m_dca1.env.min_frames = min_frames;
	m_dca1.env.max_frames = max_frames;

	m_dca2.env.min_frames = min_frames;
	m_dca2.env.max_frames = max_frames;
}


uint32_t synthv1_impl::sampleRate (void) const
{
	return m_iSampleRate;
}


void synthv1_impl::setParamPort ( synthv1::ParamIndex index, float *pfParam )
{
	static float s_fDummy = 0.0f;

	if (pfParam == 0)
		pfParam = &s_fDummy;

	switch (index) {
	case synthv1::DCO1_SHAPE1:    m_dco1.shape1      = pfParam; break;
	case synthv1::DCO1_WIDTH1:    m_dco1.width1      = pfParam; break;
	case synthv1::DCO1_SHAPE2:    m_dco1.shape2      = pfParam; break;
	case synthv1::DCO1_WIDTH2:    m_dco1.width2      = pfParam; break;
	case synthv1::DCO1_BALANCE:   m_dco1.balance     = pfParam; break;
	case synthv1::DCO1_DETUNE:    m_dco1.detune      = pfParam; break;
	case synthv1::DCO1_PHASE:     m_dco1.phase       = pfParam; break;
	case synthv1::DCO1_OCTAVE:    m_dco1.octave      = pfParam; break;
	case synthv1::DCO1_TUNING:    m_dco1.tuning      = pfParam; break;
	case synthv1::DCO1_GLIDE:     m_dco1.glide       = pfParam; break;
	case synthv1::DCF1_CUTOFF:    m_dcf1.cutoff      = pfParam; break;
	case synthv1::DCF1_RESO:      m_dcf1.reso        = pfParam; break;
	case synthv1::DCF1_TYPE:      m_dcf1.type        = pfParam; break;
	case synthv1::DCF1_SLOPE:     m_dcf1.slope       = pfParam; break;
	case synthv1::DCF1_ENVELOPE:  m_dcf1.envelope    = pfParam; break;
	case synthv1::DCF1_ATTACK:    m_dcf1.env.attack  = pfParam; break;
	case synthv1::DCF1_DECAY:     m_dcf1.env.decay   = pfParam; break;
	case synthv1::DCF1_SUSTAIN:   m_dcf1.env.sustain = pfParam; break;
	case synthv1::DCF1_RELEASE:   m_dcf1.env.release = pfParam; break;
	case synthv1::LFO1_SHAPE:     m_lfo1.shape       = pfParam; break;
	case synthv1::LFO1_WIDTH:     m_lfo1.width       = pfParam; break;
	case synthv1::LFO1_RATE:      m_lfo1.rate        = pfParam; break;
	case synthv1::LFO1_SWEEP:     m_lfo1.sweep       = pfParam; break;
	case synthv1::LFO1_PITCH:     m_lfo1.pitch       = pfParam; break;
	case synthv1::LFO1_CUTOFF:    m_lfo1.cutoff      = pfParam; break;
	case synthv1::LFO1_RESO:      m_lfo1.reso        = pfParam; break;
	case synthv1::LFO1_PANNING:   m_lfo1.panning     = pfParam; break;
	case synthv1::LFO1_VOLUME:    m_lfo1.volume      = pfParam; break;
	case synthv1::LFO1_ATTACK:    m_lfo1.env.attack  = pfParam; break;
	case synthv1::LFO1_DECAY:     m_lfo1.env.decay   = pfParam; break;
	case synthv1::LFO1_SUSTAIN:   m_lfo1.env.sustain = pfParam; break;
	case synthv1::LFO1_RELEASE:   m_lfo1.env.release = pfParam; break;
	case synthv1::DCA1_VOLUME:    m_dca1.volume      = pfParam; break;
	case synthv1::DCA1_ATTACK:    m_dca1.env.attack  = pfParam; break;
	case synthv1::DCA1_DECAY:     m_dca1.env.decay   = pfParam; break;
	case synthv1::DCA1_SUSTAIN:   m_dca1.env.sustain = pfParam; break;
	case synthv1::DCA1_RELEASE:   m_dca1.env.release = pfParam; break;
	case synthv1::OUT1_WIDTH:     m_out1.width       = pfParam; break;
	case synthv1::OUT1_PANNING:   m_out1.panning     = pfParam; break;
	case synthv1::OUT1_VOLUME:    m_out1.volume      = pfParam; break;
	case synthv1::DEF1_PITCHBEND: m_def1.pitchbend   = pfParam; break;
	case synthv1::DEF1_MODWHEEL:  m_def1.modwheel    = pfParam; break;
	case synthv1::DEF1_PRESSURE:  m_def1.pressure    = pfParam; break;
	case synthv1::DEF1_VELOCITY:  m_def1.velocity    = pfParam; break;
	case synthv1::DCO2_SHAPE1:    m_dco2.shape1      = pfParam; break;
	case synthv1::DCO2_WIDTH1:    m_dco2.width1      = pfParam; break;
	case synthv1::DCO2_SHAPE2:    m_dco2.shape2      = pfParam; break;
	case synthv1::DCO2_WIDTH2:    m_dco2.width2      = pfParam; break;
	case synthv1::DCO2_BALANCE:   m_dco2.balance     = pfParam; break;
	case synthv1::DCO2_DETUNE:    m_dco2.detune      = pfParam; break;
	case synthv1::DCO2_PHASE:     m_dco2.phase       = pfParam; break;
	case synthv1::DCO2_OCTAVE:    m_dco2.octave      = pfParam; break;
	case synthv1::DCO2_TUNING:    m_dco2.tuning      = pfParam; break;
	case synthv1::DCO2_GLIDE:     m_dco2.glide       = pfParam; break;
	case synthv1::DCF2_CUTOFF:    m_dcf2.cutoff      = pfParam; break;
	case synthv1::DCF2_RESO:      m_dcf2.reso        = pfParam; break;
	case synthv1::DCF2_TYPE:      m_dcf2.type        = pfParam; break;
	case synthv1::DCF2_SLOPE:     m_dcf2.slope       = pfParam; break;
	case synthv1::DCF2_ENVELOPE:  m_dcf2.envelope    = pfParam; break;
	case synthv1::DCF2_ATTACK:    m_dcf2.env.attack  = pfParam; break;
	case synthv1::DCF2_DECAY:     m_dcf2.env.decay   = pfParam; break;
	case synthv1::DCF2_SUSTAIN:   m_dcf2.env.sustain = pfParam; break;
	case synthv1::DCF2_RELEASE:   m_dcf2.env.release = pfParam; break;
	case synthv1::LFO2_SHAPE:     m_lfo2.shape       = pfParam; break;
	case synthv1::LFO2_WIDTH:     m_lfo2.width       = pfParam; break;
	case synthv1::LFO2_RATE:      m_lfo2.rate        = pfParam; break;
	case synthv1::LFO2_SWEEP:     m_lfo2.sweep       = pfParam; break;
	case synthv1::LFO2_PITCH:     m_lfo2.pitch       = pfParam; break;
	case synthv1::LFO2_CUTOFF:    m_lfo2.cutoff      = pfParam; break;
	case synthv1::LFO2_RESO:      m_lfo2.reso        = pfParam; break;
	case synthv1::LFO2_PANNING:   m_lfo2.panning     = pfParam; break;
	case synthv1::LFO2_VOLUME:    m_lfo2.volume      = pfParam; break;
	case synthv1::LFO2_ATTACK:    m_lfo2.env.attack  = pfParam; break;
	case synthv1::LFO2_DECAY:     m_lfo2.env.decay   = pfParam; break;
	case synthv1::LFO2_SUSTAIN:   m_lfo2.env.sustain = pfParam; break;
	case synthv1::LFO2_RELEASE:   m_lfo2.env.release = pfParam; break;
	case synthv1::DCA2_VOLUME:    m_dca2.volume      = pfParam; break;
	case synthv1::DCA2_ATTACK:    m_dca2.env.attack  = pfParam; break;
	case synthv1::DCA2_DECAY:     m_dca2.env.decay   = pfParam; break;
	case synthv1::DCA2_SUSTAIN:   m_dca2.env.sustain = pfParam; break;
	case synthv1::DCA2_RELEASE:   m_dca2.env.release = pfParam; break;
	case synthv1::OUT2_WIDTH:     m_out2.width       = pfParam; break;
	case synthv1::OUT2_PANNING:   m_out2.panning     = pfParam; break;
	case synthv1::OUT2_VOLUME:    m_out2.volume      = pfParam; break;
	case synthv1::DEF2_PITCHBEND: m_def2.pitchbend   = pfParam; break;
	case synthv1::DEF2_MODWHEEL:  m_def2.modwheel    = pfParam; break;
	case synthv1::DEF2_PRESSURE:  m_def2.pressure    = pfParam; break;
	case synthv1::DEF2_VELOCITY:  m_def2.velocity    = pfParam; break;
	case synthv1::CHO1_WET:       m_cho.wet          = pfParam; break;
	case synthv1::CHO1_DELAY:     m_cho.delay        = pfParam; break;
	case synthv1::CHO1_FEEDB:     m_cho.feedb        = pfParam; break;
	case synthv1::CHO1_RATE:      m_cho.rate         = pfParam; break;
	case synthv1::CHO1_MOD:       m_cho.mod          = pfParam; break;
	case synthv1::FLA1_WET:       m_fla.wet          = pfParam; break;
	case synthv1::FLA1_DELAY:     m_fla.delay        = pfParam; break;
	case synthv1::FLA1_FEEDB:     m_fla.feedb        = pfParam; break;
	case synthv1::FLA1_DAFT:      m_fla.daft         = pfParam; break;
	case synthv1::PHA1_WET:       m_pha.wet          = pfParam; break;
	case synthv1::PHA1_RATE:      m_pha.rate         = pfParam; break;
	case synthv1::PHA1_FEEDB:     m_pha.feedb        = pfParam; break;
	case synthv1::PHA1_DEPTH:     m_pha.depth        = pfParam; break;
	case synthv1::PHA1_DAFT:      m_pha.daft         = pfParam; break;
	case synthv1::DEL1_WET:       m_del.wet          = pfParam; break;
	case synthv1::DEL1_DELAY:     m_del.delay        = pfParam; break;
	case synthv1::DEL1_FEEDB:     m_del.feedb        = pfParam; break;
	case synthv1::DEL1_BPM:       m_del.bpm          = pfParam; break;
	case synthv1::DYN1_COMPRESS:  m_dyn.compress     = pfParam; break;
	case synthv1::DYN1_LIMITER:   m_dyn.limiter      = pfParam; break;
	default: break;
	}
}


float *synthv1_impl::paramPort ( synthv1::ParamIndex index )
{
	float *pfParam= 0;

	switch (index) {
	case synthv1::DCO1_SHAPE1:    pfParam = m_dco1.shape1;      break;
	case synthv1::DCO1_WIDTH1:    pfParam = m_dco1.width1;      break;
	case synthv1::DCO1_SHAPE2:    pfParam = m_dco1.shape2;      break;
	case synthv1::DCO1_WIDTH2:    pfParam = m_dco1.width2;      break;
	case synthv1::DCO1_BALANCE:   pfParam = m_dco1.balance;     break;
	case synthv1::DCO1_DETUNE:    pfParam = m_dco1.detune;      break;
	case synthv1::DCO1_PHASE:     pfParam = m_dco1.phase;       break;
	case synthv1::DCO1_OCTAVE:    pfParam = m_dco1.octave;      break;
	case synthv1::DCO1_TUNING:    pfParam = m_dco1.tuning;      break;
	case synthv1::DCO1_GLIDE:     pfParam = m_dco1.glide;       break;
	case synthv1::DCF1_CUTOFF:    pfParam = m_dcf1.cutoff;      break;
	case synthv1::DCF1_RESO:      pfParam = m_dcf1.reso;        break;
	case synthv1::DCF1_TYPE:      pfParam = m_dcf1.type;        break;
	case synthv1::DCF1_SLOPE:     pfParam = m_dcf1.slope;       break;
	case synthv1::DCF1_ENVELOPE:  pfParam = m_dcf1.envelope;    break;
	case synthv1::DCF1_ATTACK:    pfParam = m_dcf1.env.attack;  break;
	case synthv1::DCF1_DECAY:     pfParam = m_dcf1.env.decay;   break;
	case synthv1::DCF1_SUSTAIN:   pfParam = m_dcf1.env.sustain; break;
	case synthv1::DCF1_RELEASE:   pfParam = m_dcf1.env.release; break;
	case synthv1::LFO1_SHAPE:     pfParam = m_lfo1.shape;       break;
	case synthv1::LFO1_WIDTH:     pfParam = m_lfo1.width;       break;
	case synthv1::LFO1_RATE:      pfParam = m_lfo1.rate;        break;
	case synthv1::LFO1_SWEEP:     pfParam = m_lfo1.sweep;       break;
	case synthv1::LFO1_PITCH:     pfParam = m_lfo1.pitch;       break;
	case synthv1::LFO1_CUTOFF:    pfParam = m_lfo1.cutoff;      break;
	case synthv1::LFO1_RESO:      pfParam = m_lfo1.reso;        break;
	case synthv1::LFO1_PANNING:   pfParam = m_lfo1.panning;     break;
	case synthv1::LFO1_VOLUME:    pfParam = m_lfo1.volume;      break;
	case synthv1::LFO1_ATTACK:    pfParam = m_lfo1.env.attack;  break;
	case synthv1::LFO1_DECAY:     pfParam = m_lfo1.env.decay;   break;
	case synthv1::LFO1_SUSTAIN:   pfParam = m_lfo1.env.sustain; break;
	case synthv1::LFO1_RELEASE:   pfParam = m_lfo1.env.release; break;
	case synthv1::DCA1_VOLUME:    pfParam = m_dca1.volume;      break;
	case synthv1::DCA1_ATTACK:    pfParam = m_dca1.env.attack;  break;
	case synthv1::DCA1_DECAY:     pfParam = m_dca1.env.decay;   break;
	case synthv1::DCA1_SUSTAIN:   pfParam = m_dca1.env.sustain; break;
	case synthv1::DCA1_RELEASE:   pfParam = m_dca1.env.release; break;
	case synthv1::OUT1_WIDTH:     pfParam = m_out1.width;       break;
	case synthv1::OUT1_PANNING:   pfParam = m_out1.panning;     break;
	case synthv1::OUT1_VOLUME:    pfParam = m_out1.volume;      break;
	case synthv1::DEF1_PITCHBEND: pfParam = m_def1.pitchbend;   break;
	case synthv1::DEF1_MODWHEEL:  pfParam = m_def1.modwheel;    break;
	case synthv1::DEF1_PRESSURE:  pfParam = m_def1.pressure;    break;
	case synthv1::DEF1_VELOCITY:  pfParam = m_def1.velocity;    break;
	case synthv1::DCO2_SHAPE1:    pfParam = m_dco2.shape1;      break;
	case synthv1::DCO2_WIDTH1:    pfParam = m_dco2.width1;      break;
	case synthv1::DCO2_SHAPE2:    pfParam = m_dco2.shape2;      break;
	case synthv1::DCO2_WIDTH2:    pfParam = m_dco2.width2;      break;
	case synthv1::DCO2_BALANCE:   pfParam = m_dco2.balance;     break;
	case synthv1::DCO2_DETUNE:    pfParam = m_dco2.detune;      break;
	case synthv1::DCO2_PHASE:     pfParam = m_dco2.phase;       break;
	case synthv1::DCO2_OCTAVE:    pfParam = m_dco2.octave;      break;
	case synthv1::DCO2_TUNING:    pfParam = m_dco2.tuning;      break;
	case synthv1::DCO2_GLIDE:     pfParam = m_dco2.glide;       break;
	case synthv1::DCF2_CUTOFF:    pfParam = m_dcf2.cutoff;      break;
	case synthv1::DCF2_RESO:      pfParam = m_dcf2.reso;        break;
	case synthv1::DCF2_TYPE:      pfParam = m_dcf2.type;        break;
	case synthv1::DCF2_SLOPE:     pfParam = m_dcf2.slope;       break;
	case synthv1::DCF2_ENVELOPE:  pfParam = m_dcf2.envelope;    break;
	case synthv1::DCF2_ATTACK:    pfParam = m_dcf2.env.attack;  break;
	case synthv1::DCF2_DECAY:     pfParam = m_dcf2.env.decay;   break;
	case synthv1::DCF2_SUSTAIN:   pfParam = m_dcf2.env.sustain; break;
	case synthv1::DCF2_RELEASE:   pfParam = m_dcf2.env.release; break;
	case synthv1::LFO2_SHAPE:     pfParam = m_lfo2.shape;       break;
	case synthv1::LFO2_WIDTH:     pfParam = m_lfo2.width;       break;
	case synthv1::LFO2_RATE:      pfParam = m_lfo2.rate;        break;
	case synthv1::LFO2_SWEEP:     pfParam = m_lfo2.sweep;       break;
	case synthv1::LFO2_PITCH:     pfParam = m_lfo2.pitch;       break;
	case synthv1::LFO2_CUTOFF:    pfParam = m_lfo2.cutoff;      break;
	case synthv1::LFO2_RESO:      pfParam = m_lfo2.reso;        break;
	case synthv1::LFO2_PANNING:   pfParam = m_lfo2.panning;     break;
	case synthv1::LFO2_VOLUME:    pfParam = m_lfo2.volume;      break;
	case synthv1::LFO2_ATTACK:    pfParam = m_lfo2.env.attack;  break;
	case synthv1::LFO2_DECAY:     pfParam = m_lfo2.env.decay;   break;
	case synthv1::LFO2_SUSTAIN:   pfParam = m_lfo2.env.sustain; break;
	case synthv1::LFO2_RELEASE:   pfParam = m_lfo2.env.release; break;
	case synthv1::DCA2_VOLUME:    pfParam = m_dca2.volume;      break;
	case synthv1::DCA2_ATTACK:    pfParam = m_dca2.env.attack;  break;
	case synthv1::DCA2_DECAY:     pfParam = m_dca2.env.decay;   break;
	case synthv1::DCA2_SUSTAIN:   pfParam = m_dca2.env.sustain; break;
	case synthv1::DCA2_RELEASE:   pfParam = m_dca2.env.release; break;
	case synthv1::OUT2_WIDTH:     pfParam = m_out2.width;       break;
	case synthv1::OUT2_PANNING:   pfParam = m_out2.panning;     break;
	case synthv1::OUT2_VOLUME:    pfParam = m_out2.volume;      break;
	case synthv1::DEF2_PITCHBEND: pfParam = m_def2.pitchbend;   break;
	case synthv1::DEF2_MODWHEEL:  pfParam = m_def2.modwheel;    break;
	case synthv1::DEF2_PRESSURE:  pfParam = m_def2.pressure;    break;
	case synthv1::DEF2_VELOCITY:  pfParam = m_def2.velocity;    break;
	case synthv1::CHO1_WET:       pfParam = m_cho.wet;          break;
	case synthv1::CHO1_DELAY:     pfParam = m_cho.delay;        break;
	case synthv1::CHO1_FEEDB:     pfParam = m_cho.feedb;        break;
	case synthv1::CHO1_RATE:      pfParam = m_cho.rate;         break;
	case synthv1::CHO1_MOD:       pfParam = m_cho.mod;          break;
	case synthv1::FLA1_WET:       pfParam = m_fla.wet;          break;
	case synthv1::FLA1_DELAY:     pfParam = m_fla.delay;        break;
	case synthv1::FLA1_FEEDB:     pfParam = m_fla.feedb;        break;
	case synthv1::FLA1_DAFT:      pfParam = m_fla.daft;         break;
	case synthv1::PHA1_WET:       pfParam = m_pha.wet;          break;
	case synthv1::PHA1_RATE:      pfParam = m_pha.rate;         break;
	case synthv1::PHA1_FEEDB:     pfParam = m_pha.feedb;        break;
	case synthv1::PHA1_DEPTH:     pfParam = m_pha.depth;        break;
	case synthv1::PHA1_DAFT:      pfParam = m_pha.daft;         break;
	case synthv1::DEL1_WET:       pfParam = m_del.wet;          break;
	case synthv1::DEL1_DELAY:     pfParam = m_del.delay;        break;
	case synthv1::DEL1_FEEDB:     pfParam = m_del.feedb;        break;
	case synthv1::DEL1_BPM:       pfParam = m_del.bpm;          break;
	case synthv1::DYN1_COMPRESS:  pfParam = m_dyn.compress;     break;
	case synthv1::DYN1_LIMITER:   pfParam = m_dyn.limiter;      break;
	default: break;
	}

	return pfParam;
}


// convert note to frequency (hertz)

inline float note_freq ( float note )
{
	return (440.0f / 32.0f) * ::powf(2.0f, (note - 9.0f) / 12.0f);
}


// handle midi input

void synthv1_impl::process_midi ( uint8_t *data, uint32_t size )
{
	// check data size (#1)
	if (size < 2)
		return;

	// note on
	const int status = (data[0] & 0xf0);
	const int key    = (data[1] & 0x7f);

	if (status == 0xd0) {
		// channel aftertouch
		m_ctl.pressure = float(key) / 127.0f;
	}

	// check data size (#2)
	if (size < 3)
		return;

	const int value  = (data[2] & 0x7f);

	// note on
	if (status == 0x90 && value > 0) {
		synthv1_voice *pv = m_notes[key];
		if (pv/* && !m_ctl.sustain*/) {
			// retrigger fast release
			m_dcf1.env.note_off_fast(&pv->dcf1_env);
			m_dcf2.env.note_off_fast(&pv->dcf2_env);
			m_lfo1.env.note_off_fast(&pv->lfo1_env);
			m_lfo2.env.note_off_fast(&pv->lfo2_env);
			m_dca1.env.note_off_fast(&pv->dca1_env);
			m_dca2.env.note_off_fast(&pv->dca2_env);
			pv->note = -1;
			m_notes[key] = 0;
		}
		// find free voice
		pv = alloc_voice();
		if (pv) {
			// waveform
			pv->note = key;
			// velocity
			float vel = float(value) / 127.0f;
			vel *= vel;	// quadratic velocity law
			pv->vel1 = synthv1_velocity(vel, *m_def1.velocity);
			pv->vel2 = synthv1_velocity(vel, *m_def2.velocity);
			// balance
			pv->dco1_bal.reset(m_dco1.balance);
			pv->dco2_bal.reset(m_dco2.balance);
			// phases
			pv->dco1_sample1 = pv->dco1_osc1.start();
			pv->dco1_sample2 = pv->dco1_osc2.start(*m_dco1.phase * PHASE_SCALE);
			pv->dco2_sample1 = pv->dco2_osc1.start();
			pv->dco2_sample2 = pv->dco2_osc2.start(*m_dco2.phase * PHASE_SCALE);
			// frequencies
			const float freq1 = float(key)
				+ *m_dco1.octave * OCTAVE_SCALE
				+ *m_dco1.tuning * TUNING_SCALE;
			const float detune1
				= *m_dco1.detune * DETUNE_SCALE;
			pv->dco1_freq1 = note_freq(freq1 - detune1);
			pv->dco1_freq2 = note_freq(freq1 + detune1);
			const float freq2 = float(key)
				+ *m_dco2.octave * OCTAVE_SCALE
				+ *m_dco2.tuning * TUNING_SCALE;
			const float detune2
				= *m_dco2.detune * DETUNE_SCALE;
			pv->dco2_freq1 = note_freq(freq2 - detune2);
			pv->dco2_freq2 = note_freq(freq2 + detune2);
			// filters
			const synthv1_filter1::Type type1
				= synthv1_filter1::Type(int(*m_dcf1.type));
			pv->dcf11.reset(type1);
			pv->dcf12.reset(type1);
			pv->dcf13.reset(type1);
			pv->dcf14.reset(type1);
			const synthv1_filter2::Type type2
				= synthv1_filter2::Type(int(*m_dcf2.type));
			pv->dcf21.reset(type2);
			pv->dcf22.reset(type2);
			pv->dcf23.reset(type2);
			pv->dcf24.reset(type2);
			// envelopes
			m_dcf1.env.start(&pv->dcf1_env);
			m_dcf2.env.start(&pv->dcf2_env);
			m_lfo1.env.start(&pv->lfo1_env);
			m_lfo2.env.start(&pv->lfo2_env);
			m_dca1.env.start(&pv->dca1_env);
			m_dca2.env.start(&pv->dca2_env);
			// lfos
			pv->lfo1_sample = pv->lfo1_osc.start();
			pv->lfo2_sample = pv->lfo2_osc.start();
			// glides (portamentoa)
			const float srate = float(m_iSampleRate);
			float frames = uint32_t(*m_dco1.glide * *m_dco1.glide * srate);
			pv->dco1_glide1.reset(frames, pv->dco1_freq1);
			pv->dco1_glide2.reset(frames, pv->dco1_freq2);
			frames = uint32_t(*m_dco2.glide * *m_dco2.glide * srate);
			pv->dco2_glide1.reset(frames, pv->dco2_freq1);
			pv->dco2_glide2.reset(frames, pv->dco2_freq2);
			// sustain
			pv->sustain = false;
			// allocated
			m_notes[key] = pv;
		}
	}
	// note off
	else if (status == 0x80 || (status == 0x90 && value == 0)) {
		synthv1_voice *pv = m_notes[key];
		if (pv && pv->note >= 0) {
			if (m_ctl.sustain) {
				pv->sustain = true;
			} else {
				if (pv->dca1_env.stage != synthv1_env::Release) {
					m_dca1.env.note_off(&pv->dca1_env);
					m_dcf1.env.note_off(&pv->dcf1_env);
					m_lfo1.env.note_off(&pv->lfo1_env);
				}
				if (pv->dca2_env.stage != synthv1_env::Release) {
					m_dca2.env.note_off(&pv->dca2_env);
					m_dcf2.env.note_off(&pv->dcf2_env);
					m_lfo2.env.note_off(&pv->lfo2_env);
				}
			}
		}
	}
	// control change
	else if (status == 0xb0) {
		switch (key) {
		case 0x01: {
			// modulation wheel (cc#1)
			const float modwheel = float(value) / 127.0f;
			m_ctl.modwheel1 = *m_def1.modwheel * modwheel;
			m_ctl.modwheel2 = *m_def2.modwheel * modwheel;
			break;
		}
		case 0x07:
			// channel volume (cc#7)
			m_ctl.volume = float(value) / 127.0f;
			break;
		case 0x0a:
			// channel panning (cc#10)
			m_ctl.panning = float(value - 64) / 64.0f;
			break;
		case 0x40:
			// sustain/damper pedal (cc#64)
			if (m_ctl.sustain && value <  64)
				allSustainOff();
			m_ctl.sustain = bool(value >= 64);
			break;
		case 0x78:
			// all sound off (cc#120)
			allSoundOff();
			break;
		case 0x79:
			// all controllers off (cc#121)
			allControllersOff();
			break;
		case 0x7b:
			// all notes off (cc#123)
			allNotesOff();
			break;
		}
	}
	// pitch bend
	else if (status == 0xe0) {
		const float pitchbend = float(key + (value << 7) - 0x2000) / 8192.0f;
		m_ctl.pitchbend1 = synthv1_pitchbend(*m_def1.pitchbend * pitchbend);
		m_ctl.pitchbend2 = synthv1_pitchbend(*m_def2.pitchbend * pitchbend);
	}
}


// all controllers off

void synthv1_impl::allControllersOff (void)
{
	m_ctl.reset();
}


// all sound off

void synthv1_impl::allSoundOff (void)
{
	m_chorus.setSampleRate(m_iSampleRate);
	m_chorus.reset();

	for (uint16_t k = 0; k < m_iChannels; ++k) {
		m_phaser[k].setSampleRate(m_iSampleRate);
		m_delay[k].setSampleRate(m_iSampleRate);
		m_comp[k].setSampleRate(m_iSampleRate);
		m_flanger[k].reset();
		m_phaser[k].reset();
		m_delay[k].reset();
		m_comp[k].reset();
	}
}


// all notes off

void synthv1_impl::allNotesOff (void)
{
	synthv1_voice *pv = m_play_list.next();
	while (pv) {
		if (pv->note >= 0)
			m_notes[pv->note] = 0;
		free_voice(pv);
		pv = m_play_list.next();
	}

	dco1_last1 = 0.0f;
	dco1_last2 = 0.0f;
	dco2_last1 = 0.0f;
	dco2_last2 = 0.0f;

	m_aux1.reset();
	m_aux2.reset();
}


// all sustained notes off

void synthv1_impl::allSustainOff (void)
{
	synthv1_voice *pv = m_play_list.next();
	while (pv) {
		if (pv->note >= 0 && pv->sustain) {
			pv->sustain = false;
			if (pv->dca1_env.stage != synthv1_env::Release) {
				m_dca1.env.note_off(&pv->dca1_env);
				m_dcf1.env.note_off(&pv->dcf1_env);
				m_lfo1.env.note_off(&pv->lfo1_env);
			}
			if (pv->dca2_env.stage != synthv1_env::Release) {
				m_dca2.env.note_off(&pv->dca2_env);
				m_dcf2.env.note_off(&pv->dcf2_env);
				m_lfo2.env.note_off(&pv->lfo2_env);
			}
		}
		pv = pv->next();
	}
}


// all reset clear

void synthv1_impl::reset (void)
{
//--legacy support < 0.3.0.4 -- begin
	if (*m_del.bpm < 3.6f)
		*m_del.bpm *= 100.0f;
//--legacy support < 0.3.0.4 -- end.

	m_vol1.reset(m_out1.volume, m_dca1.volume, &m_ctl.volume, &m_aux1.volume);
	m_pan1.reset(m_out1.panning, &m_ctl.panning, &m_aux1.panning);
	m_wid1.reset(m_out1.width);

	m_vol2.reset(m_out2.volume, m_dca2.volume, &m_ctl.volume, &m_aux2.volume);
	m_pan2.reset(m_out2.panning, &m_ctl.panning, &m_aux2.panning);
	m_wid2.reset(m_out2.width);
	
	m_pre1.reset(m_def1.pressure, &m_ctl.pressure);
	m_pre2.reset(m_def2.pressure, &m_ctl.pressure);

	// flangers
	if (m_flanger == 0)
		m_flanger = new synthv1_fx_flanger [m_iChannels];

	// phasers
	if (m_phaser == 0)
		m_phaser = new synthv1_fx_phaser [m_iChannels];

	// delays
	if (m_delay == 0)
		m_delay = new synthv1_fx_delay [m_iChannels];

	// compressors
	if (m_comp == 0)
		m_comp = new synthv1_fx_comp [m_iChannels];

	allSoundOff();
//	allControllersOff();
	allNotesOff();
}


// synthesize

void synthv1_impl::process ( float **ins, float **outs, uint32_t nframes )
{
	float *v_outs[m_iChannels];

	// buffer i/o transfer

	uint16_t k;

	for (k = 0; k < m_iChannels; ++k)
		::memcpy(outs[k], ins[k], nframes * sizeof(float));

	// controls

	const float lfo1_rate = *m_lfo1.rate * *m_lfo1.rate;
	const float lfo2_rate = *m_lfo2.rate * *m_lfo2.rate;
	const float lfo1_freq = LFO_FREQ_MIN + lfo1_rate * (LFO_FREQ_MAX - LFO_FREQ_MIN);
	const float lfo2_freq = LFO_FREQ_MIN + lfo2_rate * (LFO_FREQ_MAX - LFO_FREQ_MIN);
	const float modwheel1 = m_ctl.modwheel1 + PITCH_SCALE * *m_lfo1.pitch;
	const float modwheel2 = m_ctl.modwheel2 + PITCH_SCALE * *m_lfo2.pitch;

	if (int(*m_dco1.shape1) != int(dco1_wave1.shape()) || *m_dco1.width1 != dco1_wave1.width())
		dco1_wave1.reset(synthv1_wave::Shape(*m_dco1.shape1), *m_dco1.width1);
	if (int(*m_dco1.shape2) != int(dco1_wave2.shape()) || *m_dco1.width2 != dco1_wave2.width())
		dco1_wave2.reset(synthv1_wave::Shape(*m_dco1.shape2), *m_dco1.width2);

	if (int(*m_dco2.shape1) != int(dco2_wave1.shape()) || *m_dco2.width1 != dco2_wave1.width())
		dco2_wave1.reset(synthv1_wave::Shape(*m_dco2.shape1), *m_dco2.width1);
	if (int(*m_dco2.shape2) != int(dco2_wave2.shape()) || *m_dco2.width2 != dco2_wave2.width())
		dco2_wave2.reset(synthv1_wave::Shape(*m_dco2.shape2), *m_dco2.width2);
	
	if (int(*m_lfo1.shape) != int(lfo1_wave.shape()) || *m_lfo1.width != lfo1_wave.width())
		lfo1_wave.reset(synthv1_wave::Shape(*m_lfo1.shape), *m_lfo1.width);
	if (int(*m_lfo2.shape) != int(lfo2_wave.shape()) || *m_lfo2.width != lfo2_wave.width())
		lfo2_wave.reset(synthv1_wave::Shape(*m_lfo2.shape), *m_lfo2.width);

	m_wid1.process(nframes);
	m_pan1.process(nframes);
	m_vol1.process(nframes);

	m_wid2.process(nframes);
	m_pan2.process(nframes);
	m_vol2.process(nframes);

	m_pre1.process(nframes);
	m_pre2.process(nframes);

	// per voice

	synthv1_voice *pv = m_play_list.next();

	while (pv) {

		synthv1_voice *pv_next = pv->next();

		// output buffers

		for (k = 0; k < m_iChannels; ++k)
			v_outs[k] = outs[k];

		uint32_t nblock = nframes;

		while (nblock > 0) {

			uint32_t ngen = nblock;

			// process envelope stages

			if (pv->dca1_env.running && pv->dca1_env.frames < ngen)
				ngen = pv->dca1_env.frames;
			if (pv->dca2_env.running && pv->dca2_env.frames < ngen)
				ngen = pv->dca2_env.frames;
			if (pv->dcf1_env.running && pv->dcf1_env.frames < ngen)
				ngen = pv->dcf1_env.frames;
			if (pv->dcf2_env.running && pv->dcf2_env.frames < ngen)
				ngen = pv->dcf2_env.frames;
			if (pv->lfo1_env.running && pv->lfo1_env.frames < ngen)
				ngen = pv->lfo1_env.frames;
			if (pv->lfo2_env.running && pv->lfo2_env.frames < ngen)
				ngen = pv->lfo2_env.frames;

			pv->dco1_bal.process(ngen);
			pv->dco2_bal.process(ngen);

			for (uint32_t j = 0; j < ngen; ++j) {

				// velocities

				const float vel1
					= (pv->vel1 + (1.0f - pv->vel1) * m_pre1.value(j));
				const float vel2
					= (pv->vel2 + (1.0f - pv->vel2) * m_pre2.value(j));

				// generators

				const float lfo1_env = pv->lfo1_env.tick();
				const float lfo2_env = pv->lfo2_env.tick();

				const float lfo1 = pv->lfo1_sample * lfo1_env;
				const float lfo2 = pv->lfo2_sample * lfo2_env;

				float dco11 = pv->dco1_sample1 * pv->dco1_bal.value(j, 0);
				float dco12 = pv->dco1_sample2 * pv->dco1_bal.value(j, 1);
				float dco21 = pv->dco2_sample1 * pv->dco2_bal.value(j, 0);
				float dco22 = pv->dco2_sample2 * pv->dco2_bal.value(j, 1);

				pv->dco1_sample1 = pv->dco1_osc1.sample(pv->dco1_freq1
					* (m_ctl.pitchbend1 + modwheel1 * lfo1)
					+ pv->dco1_glide1.tick());
				pv->dco1_sample2 = pv->dco1_osc2.sample(pv->dco1_freq2
					* (m_ctl.pitchbend1 + modwheel1 * lfo1)
					+ pv->dco1_glide2.tick());

				pv->dco2_sample1 = pv->dco2_osc1.sample(pv->dco2_freq1
					* (m_ctl.pitchbend2 + modwheel2 * lfo2)
					+ pv->dco2_glide1.tick());
				pv->dco2_sample2 = pv->dco2_osc2.sample(pv->dco2_freq2
					* (m_ctl.pitchbend2 + modwheel2 * lfo2)
					+ pv->dco2_glide2.tick());

				pv->lfo1_sample = pv->lfo1_osc.sample(lfo1_freq
					* (1.0f + SWEEP_SCALE * *m_lfo1.sweep * lfo1_env));
				pv->lfo2_sample = pv->lfo2_osc.sample(lfo2_freq
					* (1.0f + SWEEP_SCALE * *m_lfo2.sweep * lfo2_env));

				// filters

				const float env1 = 0.5f * (1.0f + vel1
					* *m_dcf1.envelope * pv->dcf1_env.tick());
				const float cutoff1 = synthv1_sigmoid_1(*m_dcf1.cutoff
					* env1 * (1.0f + *m_lfo1.cutoff * lfo1));
				const float reso1 = synthv1_sigmoid_1(*m_dcf1.reso
					* env1 * (1.0f + *m_lfo1.reso * lfo1));

				dco11 = pv->dcf11.output(dco11, cutoff1, reso1);
				dco12 = pv->dcf12.output(dco12, cutoff1, reso1);
				if (int(*m_dcf1.slope) > 0) { // 24db/octave
					dco11 = pv->dcf13.output(dco11, cutoff1, reso1);
					dco12 = pv->dcf14.output(dco12, cutoff1, reso1);
				}

				const float env2 = 0.5f * (1.0f + vel2
					* *m_dcf2.envelope * pv->dcf2_env.tick());
				const float cutoff2 = synthv1_sigmoid_1(*m_dcf2.cutoff
					* env2 * (1.0f + *m_lfo2.cutoff * lfo2));
				const float reso2 = synthv1_sigmoid_1(*m_dcf2.reso
					* env2 * (1.0f + *m_lfo2.reso * lfo2));

				dco21 = pv->dcf21.output(dco21, cutoff2, reso2);
				dco22 = pv->dcf22.output(dco22, cutoff2, reso2);
				if (int(*m_dcf2.slope) > 0) { // 24db/octave
					dco21 = pv->dcf23.output(dco21, cutoff2, reso2);
					dco22 = pv->dcf24.output(dco22, cutoff2, reso2);
				}

				// volumes

				const float wid1 = m_wid1.value(j);
				const float mid1 = 0.5f * (dco11 + dco12);
				const float sid1 = 0.5f * (dco11 - dco12);
				const float vol1 = vel1 * m_vol1.value(j)
					* pv->dca1_env.tick();

				const float wid2 = m_wid2.value(j);
				const float mid2 = 0.5f * (dco21 + dco22);
				const float sid2 = 0.5f * (dco21 - dco22);
				const float vol2 = vel2 * m_vol2.value(j)
					* pv->dca2_env.tick();

				// outputs

				const float out1
					= vol1 * (mid1 + sid1 * wid1) * m_pan1.value(j, 0)
					+ vol2 * (mid2 + sid2 * wid2) * m_pan2.value(j, 0);
				const float out2
					= vol1 * (mid1 - sid1 * wid1) * m_pan1.value(j, 1)
					+ vol2 * (mid2 - sid2 * wid2) * m_pan2.value(j, 1);

				for (k = 0; k < m_iChannels; ++k)
					*v_outs[k]++ += (k & 1 ? out2 : out1);

				if (j == 0) {
					m_aux1.panning = lfo1 * *m_lfo1.panning;
					m_aux1.volume  = lfo1 * *m_lfo1.volume + 1.0f;
					m_aux2.panning = lfo2 * *m_lfo2.panning;
					m_aux2.volume  = lfo2 * *m_lfo2.volume + 1.0f;
				}
			}

			nblock -= ngen;

			// envelope countdowns

			if (pv->dca1_env.running && pv->dca1_env.frames == 0)
				m_dca1.env.next(&pv->dca1_env);
			if (pv->dca2_env.running && pv->dca2_env.frames == 0)
				m_dca2.env.next(&pv->dca2_env);

			if (pv->dca1_env.stage == synthv1_env::Done &&
				pv->dca2_env.stage == synthv1_env::Done) {
				if (pv->note >= 0)
					m_notes[pv->note] = 0;
				free_voice(pv);
				nblock = 0;
			} else {
				if (pv->dcf1_env.running && pv->dcf1_env.frames == 0)
					m_dcf1.env.next(&pv->dcf1_env);
				if (pv->dcf2_env.running && pv->dcf2_env.frames == 0)
					m_dcf2.env.next(&pv->dcf2_env);
				if (pv->lfo1_env.running && pv->lfo1_env.frames == 0)
					m_lfo1.env.next(&pv->lfo1_env);
				if (pv->lfo2_env.running && pv->lfo2_env.frames == 0)
					m_lfo2.env.next(&pv->lfo2_env);
			}
		}

		// next playing voice

		pv = pv_next;
	}

	// effects

	for (k = 0; k < m_iChannels; ++k) {
		float *in = outs[k];
		float *out = in;
		// chorus
		if (k > 0) {
			m_chorus.process(outs[k - 1], outs[k], nframes, *m_cho.wet,
				*m_cho.delay, *m_cho.feedb, *m_cho.rate, *m_cho.mod);
		}
		// flanger
		m_flanger[k].process(in, nframes, *m_fla.wet,
			*m_fla.delay, *m_fla.feedb, *m_fla.daft * float(k));
		// phaser
		m_phaser[k].process(in, nframes, *m_pha.wet,
			*m_pha.rate, *m_pha.feedb, *m_pha.depth, *m_pha.daft * float(k));
		// delay
		m_delay[k].process(in, nframes, *m_del.wet,
			*m_del.delay, *m_del.feedb, *m_del.bpm);
		// compressor
		if (int(*m_dyn.compress) > 0)
			m_comp[k].process(in, nframes);
		// limiter
		if (int(*m_dyn.limiter) > 0) {
			for (uint32_t n = 0; n < nframes; ++n)
				*out++ = synthv1_sigmoid(*in++);
		}
	}
}


//-------------------------------------------------------------------------
// synthv1 - decl.
//

synthv1::synthv1 ( uint16_t iChannels, uint32_t iSampleRate )
{
	m_pImpl = new synthv1_impl(iChannels, iSampleRate);
}


synthv1::~synthv1 (void)
{
	delete m_pImpl;
}


void synthv1::setChannels ( uint16_t iChannels )
{
	m_pImpl->setChannels(iChannels);
}


uint16_t synthv1::channels (void) const
{
	return m_pImpl->channels();
}


void synthv1::setSampleRate ( uint32_t iSampleRate )
{
	m_pImpl->setSampleRate(iSampleRate);
}


uint32_t synthv1::sampleRate (void) const
{
	return m_pImpl->sampleRate();
}


void synthv1::setParamPort ( ParamIndex index, float *pfParam )
{
	m_pImpl->setParamPort(index, pfParam);
}

float *synthv1::paramPort ( ParamIndex index ) const
{
	return m_pImpl->paramPort(index);
}


void synthv1::process_midi ( uint8_t *data, uint32_t size )
{
#ifdef CONFIG_DEBUG_0
	fprintf(stderr, "synthv1[%p]::process_midi(%u)", this, size);
	for (uint32_t i = 0; i < size; ++i)
		fprintf(stderr, " %02x", data[i]);
	fprintf(stderr, "\n");
#endif

	m_pImpl->process_midi(data, size);
}


void synthv1::process ( float **ins, float **outs, uint32_t nframes )
{
	m_pImpl->process(ins, outs, nframes);
}


// all reset clear

void synthv1::reset (void)
{
	m_pImpl->reset();
}


// end of synthv1.cpp
