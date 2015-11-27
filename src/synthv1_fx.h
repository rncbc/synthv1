// synthv1_fx.h
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

#ifndef __synthv1_fx_h
#define __synthv1_fx_h

#include <stdint.h>
#include <stdlib.h>
#include <math.h>


//-------------------------------------------------------------------------
// synthv1_fx
//
// -- borrowed, stirred and refactored from Highlife --
//    Copyright (C) 2007 arguru, discodsp.com
//

//-------------------------------------------------------------------------
// synthv1_fx_filter - RBJ biquad filter implementation.
//
//   http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt

class synthv1_fx_filter
{
public:

	enum Type {
		Low = 0, High, Band1, Band2, Notch, AllPass, Peak, LoShelf, HiShelf
	};

	synthv1_fx_filter(float srate = 44100.0f)
		: m_srate(srate) { reset(); }

	void setSampleRate(float srate)
		{ m_srate = srate; }
	float sampleRate() const
		{ return m_srate; }

	void reset(Type type, float freq, float q, float gain, bool bwq = false)
	{
		reset();

		// temp vars
		float alpha, a0, a1, a2, b0, b1, b2;

		// peaking, lowshelf and hishelf
		if (type >= Peak) {
			const float amp   = ::powf(10.0f, (gain / 40.0f));
			const float omega = 2.0f * M_PI * freq / m_srate;
			const float tsin  = ::sinf(omega);
			const float tcos  = ::cosf(omega);
			const float beta  = ::sqrtf(amp) / q;
			if (bwq)
				alpha = tsin * ::sinhf(::logf(2.0f) / 2.0f * q * omega / tsin);
			else
				alpha = tsin / (2.0f * q);
			switch (type) {
			case Peak:
				// peaking
				b0 =  1.0f + alpha * amp;
				b1 = -2.0f * tcos;
				b2 =  1.0f - alpha * amp;
				a0 =  1.0f + alpha / amp;
				a1 = -2.0f * tcos;
				a2 =  1.0f - alpha / amp;
				break;
			case LoShelf:
				// low-shelf
				b0 = amp * ((amp + 1.0f) - (amp - 1.0f) * tcos + beta * tsin);
				b1 = 2.0f * amp *((amp - 1.0f) - (amp + 1.0f) * tcos);
				b2 = amp * ((amp + 1.0f) - (amp - 1.0f) * tcos - beta * tsin);
				a0 = (amp + 1.0f) + (amp - 1.0f) * tcos + beta * tsin;
				a1 = -2.0f *((amp - 1.0f) + (amp + 1.0f) * tcos);
				a2 = (amp + 1.0f) + (amp - 1.0f) * tcos - beta * tsin;
				break;
			case HiShelf:
			default:
				// high-shelf
				b0 = amp * ((amp + 1.0f) + (amp - 1.0f) * tcos + beta * tsin);
				b1 = -2.0f * amp * ((amp - 1.0f) + (amp + 1.0f) * tcos);
				b2 = amp * ((amp + 1.0f) + (amp - 1.0f) * tcos - beta * tsin);
				a0 = (amp + 1.0f) - (amp - 1.0f) * tcos + beta * tsin;
				a1 = 2.0f * ((amp - 1.0f) - (amp + 1.0f) * tcos);
				a2 = (amp + 1.0f) - (amp - 1.0f) * tcos - beta * tsin;
				break;
			}
		} else {
			// other filters
			const float omega = 2.0f * M_PI * freq / m_srate;
			const float tsin  = ::sinf(omega);
			const float tcos  = ::cosf(omega);
			if (bwq)
				alpha = tsin * ::sinhf(::logf(2.0f) / 2.0f * q * omega / tsin);
			else
				alpha = tsin / (2.0f * q);
			switch (type) {
			case Low:
				// low-pass
				b0 = (1.0f - tcos) / 2.0f;
				b1 =  1.0f - tcos;
				b2 = (1.0f - tcos) / 2.0f;
				a0 =  1.0f + alpha;
				a1 = -2.0f * tcos;
				a2 =  1.0f - alpha;
				break;
			case High:
				// high-pass
				b0 = (1.0f + tcos) / 2.0f;
				b1 = -1.0f - tcos;
				b2 = (1.0f + tcos) / 2.0f;
				a0 =  1.0f + alpha;
				a1 = -2.0f * tcos;
				a2 =  1.0f - alpha;
				break;
			case Band1:
				// band-pass csg
				b0 =  tsin / 2.0f;
				b1 =  0.0f;
				b2 = -tsin / 2.0f;
				a0 =  1.0f + alpha;
				a1 = -2.0f * tcos;
				a2 =  1.0f - alpha;
				break;
			case Band2:
				// band-pass czpg
				b0 =  alpha;
				b1 =  0.0f;
				b2 = -alpha;
				a0 =  1.0f + alpha;
				a1 = -2.0f * tcos;
				a2 =  1.0f - alpha;
				break;
			case Notch:
				// notch
				b0 =  1.0f;
				b1 = -2.0f * tcos;
				b2 =  1.0f;
				a0 =  1.0f + alpha;
				a1 = -2.0f * tcos;
				a2 =  1.0f - alpha;
				break;
			case AllPass:
			default:
				// all-pass
				b0 =  1.0f - alpha;
				b1 = -2.0f * tcos;
				b2 =  1.0f + alpha;
				a0 =  1.0f + alpha;
				a1 = -2.0f * tcos;
				a2 =  1.0f - alpha;
				break;
			}
		}
		// set filter coeffs
		m_b0a0 = b0 / a0;
		m_b1a0 = b1 / a0;
		m_b2a0 = b2 / a0;
		m_a1a0 = a1 / a0;
		m_a2a0 = a2 / a0;
	};

	float output(float in)
	{
		// filter
		const float out = m_b0a0 * in
			+ m_b1a0 * m_in1  + m_b2a0 * m_in2
			- m_a1a0 * m_out1 - m_a2a0 * m_out2;
		// push in/out buffers
		m_in2  = m_in1;
		m_in1  = in;
		m_out2 = m_out1;
		m_out1 = out;
		// return output
		return out;
	}

protected:

	void reset()
	{
		m_b0a0 = m_b1a0 = m_b2a0 = m_a1a0 = m_a2a0 = 0.0f;
		m_out1 = m_out2 = 0.0f;
		m_in1 = m_in2 = 0.0f;
	}

private:

	// nominal sample-rate
	float m_srate;

	// filter coeffs
	float m_b0a0, m_b1a0, m_b2a0, m_a1a0, m_a2a0;

	// in/out history
	float m_out1, m_out2, m_in1, m_in2;
};


//-------------------------------------------------------------------------
// synthv1_fx_comp - DiscoDSP's "rock da disco" compressor/eq.

class synthv1_fx_comp
{
public:

	synthv1_fx_comp(float srate = 44100.0f)
		: m_srate(srate), m_peak(0.0f),
			m_attack(0.0f), m_release(0.0f),
			m_lo(srate), m_mi(srate), m_hi(srate) {}

	void setSampleRate(float srate)
	{
		m_srate = srate;

		m_lo.setSampleRate(srate);
		m_mi.setSampleRate(srate);
		m_hi.setSampleRate(srate);
	}

	float sampleRate() const
		{ return m_srate; }

	void reset()
	{
		m_peak = 0.0f;

		m_attack  = ::expf(-1000.0f / (m_srate * 3.6f));
		m_release = ::expf(-1000.0f / (m_srate * 150.0f));

		// rock-da-house eq.
		m_lo.reset(synthv1_fx_filter::Peak,      100.0f, 1.0f, 6.0f);
		m_mi.reset(synthv1_fx_filter::LoShelf,  1000.0f, 1.0f, 3.0f);
		m_hi.reset(synthv1_fx_filter::HiShelf, 10000.0f, 1.0f, 4.0f);
	}

	void process(float *in, uint32_t nframes)
	{
		// compressor
		const float threshold = 0.251f;	//~= powf(10.0f, -12.0f / 20.0f);
		const float post_gain = 1.995f;	//~= powf(10.0f, 6.0f / 20.0f);
		// process buffers
		for (uint32_t i = 0; i < nframes; ++i) {
			// anti-denormalizer noise
			const float ad = 1E-14f * float(::rand());
			// process
			const float lo = m_lo.output(m_mi.output(m_hi.output(*in + ad)));
			// compute peak
			const float peak = ::fabsf(lo);
			// compute gain
			float gain = 1.0f;
			if (peak > threshold)
				gain = threshold / peak;
			// envelope
			if (m_peak > gain) {
				m_peak *= m_attack;
				m_peak += (1.0f - m_attack) * gain;
			} else {
				m_peak *= m_release;
				m_peak += (1.0f - m_release) * gain;
			}
			// output
			*in++ = lo * m_peak * post_gain;
		}
	}

private:

	float m_srate;

	float m_peak;
	float m_attack;
	float m_release;

	synthv1_fx_filter m_lo, m_mi, m_hi;
};


//-------------------------------------------------------------------------
// synthv1_fx_flanger - Flanger implementation.


class synthv1_fx_flanger
{
public:

	synthv1_fx_flanger()
		{ reset(); }

	void reset()
	{
		for(uint32_t i = 0; i < MAX_SIZE; ++i)
			m_buffer[i] = 0.0f;

		m_frames = 0;
	}

	float output(float in, float delay, float feedb)
	{
		// calculate delay offset
		float delta = float(m_frames) - delay;
		// clip lookback buffer-bound
		if (delta < 0.0f)
			delta += float(MAX_SIZE);
		// get index
		const uint32_t index = uint32_t(delta);
		// 4 samples hermite
		const float y0 = m_buffer[(index + 0) & MAX_MASK];
		const float y1 = m_buffer[(index + 1) & MAX_MASK];
		const float y2 = m_buffer[(index + 2) & MAX_MASK];
		const float y3 = m_buffer[(index + 3) & MAX_MASK];
		// csi calculate
		const float c0 = y1;
		const float c1 = 0.5f * (y2 - y0);
		const float c2 = y0 - 2.5f * y1 + 2.0f * y2 - 0.5f * y3;
		const float c3 = 0.5f * (y3 - y0) + 1.5f * (y1 - y2);
		// compute interpolation x
		const float x = delta - ::floorf(delta);
		// get output
		const float out = ((c3 * x + c2) * x + c1) * x + c0;
		// add to delay buffer
		m_buffer[(m_frames++) & MAX_MASK] = in + out * feedb;
		// return output
		return out;
	}

	void process(float *in, uint32_t nframes,
		float wet, float delay, float feedb, float daft)
	{
		if (wet < 1E-9f)
			return;
		// daft effect
		if (daft > 0.001f) {
			delay *= (1.0f - daft);
		//	feedb *= (1.0f - daft);
		}
		delay *= float(MAX_SIZE);
		// process
		for (uint32_t i = 0; i < nframes; ++i)
			in[i] += wet * output(in[i], delay, feedb);
	}

	static const uint32_t MAX_SIZE = (1 << 12);	//= 4096;
	static const uint32_t MAX_MASK = MAX_SIZE - 1;

private:

	float m_buffer[MAX_SIZE];

	uint32_t m_frames;
};


//-------------------------------------------------------------------------
// synthv1_fx_chorus - Chorus implementation.

class synthv1_fx_chorus
{
public:

	synthv1_fx_chorus(float srate = 44100.0f)
		: m_srate(srate) { reset(); }

	void setSampleRate(float srate)
		{ m_srate = srate; }
	float sampleRate() const
		{ return m_srate; }

	void reset()
	{
		m_flang1.reset();
		m_flang2.reset();

		m_lfo = 0.0f;
	}

	void process(float *in1, float *in2, uint32_t nframes,
		float wet, float delay, float feedb, float rate, float mod)
	{
		if (wet < 1E-9f)
			return;
		// constrained feedback
		feedb *= 0.95f;
		// calculate delay time
		const float d0 = 0.5f * delay * float(synthv1_fx_flanger::MAX_SIZE);
		const float a1 = 0.99f * d0 * mod * mod;
		const float r2 = 4.0f * M_PI * rate * rate / m_srate;
		// process
		for (uint32_t i = 0; i < nframes; ++i) {
			// modulation
			const float lfo = a1 * pseudo_sinf(m_lfo);
			const float delay1 = d0 - lfo;
			const float delay2 = d0 - lfo * 0.9f;
			// chorus mix
			in1[i] += wet * m_flang1.output(in1[i], delay1, feedb);
			in2[i] += wet * m_flang2.output(in2[i], delay2, feedb);
			// lfo advance
			m_lfo += r2;
			// lfo wrap
			if (m_lfo >= 1.0f)
				m_lfo -= 2.0f;
		}
	}

protected:

	float pseudo_sinf(float x) const
	{
		x *= x;
		x -= 1.0f;
		return x * x;
	}

private:

	float m_srate;

	synthv1_fx_flanger m_flang1;
	synthv1_fx_flanger m_flang2;

	float m_lfo;
};


//-------------------------------------------------------------------------
// synthv1_fx_delay - Delay implementation.

class synthv1_fx_delay
{
public:

	synthv1_fx_delay(float srate = 44100.0f)
		: m_srate(srate) { reset(); }

	void setSampleRate(float srate)
		{ m_srate = srate; }
	float sampleRate() const
		{ return m_srate; }

	void reset()
	{
		for (uint32_t i = 0; i < MAX_SIZE; ++i)
			m_buffer[i] = 0.0f;

		m_out = 0.0f;
		m_frames = 0;
	}

	void process(float *in, uint32_t nframes,
		float wet, float delay, float feedb, float bpm = 0.0f)
	{
		if (wet < 1E-9f)
			return;
		// constrained feedback
		feedb *= 0.95f;
		// calculate delay time
		float delay_time = delay * m_srate;
		if (bpm > 0.0f)
			delay_time *= 60.f / bpm;
		// set integer delay
		uint32_t ndelay = uint32_t(delay_time);
		// clamp
		if (ndelay < MIN_SIZE)
			ndelay = MIN_SIZE;
		else
		if (ndelay > MAX_SIZE)
			ndelay = MAX_SIZE;
		// delay process
		for (uint32_t i = 0; i < nframes; ++i) {
			const uint32_t j = (m_frames++) & MAX_MASK;
			m_out = m_buffer[(j - ndelay) & MAX_MASK];
			m_buffer[j] = *in + m_out * feedb;
			*in++ += wet * m_out;
		}
	}

	static const uint32_t MIN_SIZE = (1 <<  8);	//= 256;
	static const uint32_t MAX_SIZE = (1 << 16);	//= 65536;
	static const uint32_t MAX_MASK = MAX_SIZE - 1;

private:

	float m_srate;

	float m_buffer[MAX_SIZE];
	float m_out;

	uint32_t m_frames;
};


//-------------------------------------------------------------------------
// synthv1_fx_allpass - All-pass delay implementation.

class synthv1_fx_allpass
{
public:

	synthv1_fx_allpass()
		{ reset(); }

	void reset()
		{ m_out = 0.0f; }

	float output(float in, float delay)
	{
		const float a1 = (1.0f - delay) / (1.0f + delay);
		const float out = m_out - a1 * in;
		m_out = in + a1 * out;
		return out;
	}

private:

	float m_out;
};


//-------------------------------------------------------------------------
// synthv1_fx_phaser - Phaser implementation.

class synthv1_fx_phaser
{
public:

	synthv1_fx_phaser(float srate = 44100.0f)
		: m_srate(srate) { reset(); }

	void setSampleRate(float srate)
		{ m_srate = srate; }
	float sampleRate() const
		{ return m_srate; }

	void reset()
	{
		// initialize vars
		m_lfo_phase = 0.0f;
		m_out = 0.0f;
		// reset taps
		for (uint16_t n = 0; n < MAX_TAPS; ++n)
			m_taps[n].reset();
	}

	void process(float *in, uint32_t nframes, float wet,
		float rate, float feedb, float depth, float daft)
	{
		if (wet < 1E-9f)
			return;
		// daft effect
		if (daft > 0.001f && daft < 1.0f) {
			rate  *= (1.0f - 0.5f * daft);
		//	feedb *= (1.0f - daft);
			depth *= (1.0f - daft);
		}
		depth += 1.0f;
		// update coeffs
		const float delay_min = 2.0f * 440.0f / m_srate;
		const float delay_max = 2.0f * 4400.0f / m_srate;
		const float lfo_inc   = 2.0f * M_PI * rate / m_srate;
		// anti-denormal noise
		const float adenormal = 1E-14f * float(::rand());
		// sweep...
		for (uint32_t i = 0; i < nframes; ++i) {
			// calculate and update phaser lfo
			const float delay = delay_min + (delay_max - delay_min)
				* 0.5f * (1.0f + ::sinf(m_lfo_phase));
			// increment phase
			m_lfo_phase += lfo_inc;
			// positive wrap phase
			if (m_lfo_phase >= 2.0f * M_PI)
				m_lfo_phase -= 2.0f * M_PI;
			// get input
			m_out = in[i] + adenormal + m_out * feedb;
			// update filter coeffs and calculate output
			for (uint16_t n = 0; n < MAX_TAPS; ++n)
				m_out = m_taps[n].output(m_out, delay);
			// output
			in[i] += wet * m_out * depth;
		}
	}

private:

	float m_srate;

	static const uint16_t MAX_TAPS = 6;

	synthv1_fx_allpass m_taps[MAX_TAPS];

	float m_dmin;
	float m_dmax;
	float m_feedb;
	float m_lfo_phase;
	float m_lfo_inc;
	float m_depth;

	float m_out;
};


#endif	// __synthv1_fx_h

// end of synthv1_fx.h
