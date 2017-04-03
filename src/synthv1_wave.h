// synthv1_wave.h
//
/****************************************************************************
   Copyright (C) 2012-2017, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __synthv1_wave_h
#define __synthv1_wave_h

#include <stdint.h>

// forward decls.
class synthv1_wave_sched;


//-------------------------------------------------------------------------
// synthv1_wave - smoothed (integrating oversampled) wave table.
//

class synthv1_wave
{
public:

	// shape.
	enum Shape { Pulse = 0, Saw, Sine, Rand, Noise };

	// ctor.
	synthv1_wave(uint32_t nsize = 4096, uint16_t nover = 24, uint16_t ntabs = 8);

	// dtor.
	~synthv1_wave();

	// properties.
	Shape shape() const
		{ return m_shape; }
	float width() const
		{ return m_width; }
	bool bandlimited() const
		{ return m_bandl; }

	// sample rate.
	void setSampleRate(float srate)
		{ m_srate = srate; }
	float sampleRate() const
		{ return m_srate; }

	// table size (in frames)
	uint32_t size() const
		{ return m_nsize; }

	// init.
	void reset(Shape shape, float width, bool bandl = false);
	// init.sync.
	void reset_sync();

	// init.test
	void reset_test(Shape shape, float width, bool bandl = false)
	{
		if (shape != m_shape || width != m_width
			|| (m_ntabs > 0 && bandl != m_bandl))
			reset(shape, width, bandl);
	}

	// phasor.
	struct Phase
	{
		float    phase;
		float    ftab;
		uint16_t itab;

		void reset() { phase = ftab = 0.0f; itab =  0; }
	};

	// begin.
	float start(Phase& phase, float pshift = 0.0f, float freq = 0.0f)
	{
		const float p0 = float(m_nsize);

		update(phase, freq);

		phase.phase = m_phase0 + pshift * p0;
		if (phase.phase >= p0)
			phase.phase -= p0;

		return sample(phase, freq);
	}

	// iterate.
	float sample(Phase& phase, float freq) const
	{
		const uint32_t i = uint32_t(phase.phase);
		const float alpha = phase.phase - float(i);
		const float p0 = float(m_nsize);

		phase.phase += p0 * freq / m_srate;
		if (phase.phase >= p0)
			phase.phase -= p0;

		if (phase.itab < m_ntabs) {
			const float x0 = interp(i, phase.itab, alpha);
			const float x1 = interp(i, phase.itab + 1, alpha);
			return x0 + phase.ftab * (x1 - x0);
		} else {
			return interp(i, phase.itab, alpha);
		}
	}

	// interpolate.
	float interp(uint32_t i, uint16_t itab, float alpha) const
	{
		float *frames = m_tables[itab];

		const float x0 = frames[i];
		const float x1 = frames[i + 1];
#if 0	// cubic interp.
		const float x2 = frames[i + 2];
		const float x3 = frames[i + 3];

		const float c1 = (x2 - x0) * 0.5f;
		const float b1 = (x1 - x2);
		const float b2 = (c1 + b1);
		const float c3 = (x3 - x1) * 0.5f + b2 + b1;
		const float c2 = (c3 + b2);

		return (((c3 * alpha) - c2) * alpha + c1) * alpha + x1;
#else	// linear interp.
		return x0 + alpha * (x1 - x0);
#endif
	}

	// absolute value.
	float value(float phase) const
	{
		const float p0 = float(m_nsize);

		phase *= p0;
		phase += m_phase0;
		if (phase >= p0)
			phase -= p0;

		return m_tables[m_ntabs][uint32_t(phase)];
	}

	// post-iter.
	void update(Phase& phase, float freq)
	{
		if (freq < m_min_freq) {
			phase.itab  = m_ntabs;
			phase.ftab  = 0.0f;
		} else if (freq < m_max_freq) {
			phase.ftab  = fast_flog2f(m_max_freq / freq);
			phase.itab  = uint16_t(phase.ftab);
			phase.ftab -= float(phase.itab);
		} else {
			phase.itab  = 0;
			phase.ftab  = 0.0f;
		}
	}

protected:

	// fast log2f approximation.
	static inline float fast_flog2f ( float x )
	{
		union { float f; uint32_t i; } u;
		u.f = x;
		return (u.i * 1.192092896e-7f) - 126.943612f;
	}

	// init pulse tables.
	void reset_pulse();
	// init pulse partial table.
	void reset_pulse_part(uint16_t itab);

	// init saw tables.
	void reset_saw();
	// init saw partial table.
	void reset_saw_part(uint16_t itab);

	// init sine tables.
	void reset_sine();
	// init sine partial table.
	void reset_sine_part(uint16_t itab);

	// init random tables.
	void reset_rand();
	// init random partial table.
	void reset_rand_part(uint16_t itab);

	// init noise tables.
	void reset_noise();
	// init noise partial table.
	void reset_noise_part(uint16_t itab);

	// post-processors.
	void reset_filter(uint16_t itab);
	void reset_normalize(uint16_t itab);
	void reset_interp(uint16_t itab);

	// Hal Chamberlain's pseudo-random linear congruential method.
	uint32_t pseudo_srand ()
		{ return (m_srand = (m_srand * 196314165) + 907633515); }
	float pseudo_randf ()
		{ return pseudo_srand() / float(0x8000U << 16) - 1.0f; }

private:

	uint32_t m_nsize;
	uint16_t m_nover;
	uint16_t m_ntabs;

	Shape    m_shape;
	float    m_width;
	bool     m_bandl;

	float    m_srate;
	float  **m_tables;
	float    m_phase0;

	uint32_t m_srand;

	float    m_min_freq;
	float    m_max_freq;

	synthv1_wave_sched *m_sched;
};


//-------------------------------------------------------------------------
// synthv1_wave_lf - hard/non-smoothed wave table (eg. LFO).
//

class synthv1_wave_lf : public synthv1_wave
{
public:

	// ctor.
	synthv1_wave_lf(uint32_t nsize = 1024)
		: synthv1_wave(nsize, 0, 0) {}
};


//-------------------------------------------------------------------------
// synthv1_oscillator - wave table oscillator
//

class synthv1_oscillator
{
public:

	// ctor.
	synthv1_oscillator(synthv1_wave *wave = 0) { reset(wave); }

	// wave and phase accessors.
	void reset(synthv1_wave *wave)
		{ m_wave = wave; m_phase.reset(); }

	synthv1_wave *wave() const
		{ return m_wave; }

	// begin.
	float start(float pshift = 0.0f, float freq = 0.0f)
		{ return m_wave->start(m_phase, pshift, freq); }

	// iterate.
	float sample(float freq)
		{ return m_wave->sample(m_phase, freq); }

	// post-iter.
	void update(float freq)
		{ m_wave->update(m_phase, freq); }

private:

	synthv1_wave *m_wave;

	synthv1_wave::Phase m_phase;
};


#endif	// __synthv1_wave_h

// end of synthv1_wave.h
