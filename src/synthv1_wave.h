// synthv1_wave.h
//
/****************************************************************************
   Copyright (C) 2012-2014, rncbc aka Rui Nuno Capela. All rights reserved.

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
#include <stdlib.h>

#include <math.h>


//-------------------------------------------------------------------------
// synthv1_wave - smoothed (integrating oversampled) wave table.
//

class synthv1_wave
{
public:

	// shape.
	enum Shape { Pulse = 0, Saw, Sine, Noise };

	// ctor.
	synthv1_wave(uint32_t nsize = 1024, uint16_t nover = 24)
		: m_nsize(nsize), m_nover(nover),
			m_shape(Pulse), m_width(1.0f), m_srate(44100.0f)
	{
		m_table = new float [m_nsize + 4];

		reset(m_shape, m_width);
	}

	// dtor.
	~synthv1_wave()
		{ delete [] m_table; }

	// properties.
	Shape shape() const
		{ return m_shape; }
	float width() const
		{ return m_width; }

	// sample rate.
	void setSampleRate(uint32_t iSampleRate)
		{ m_srate = float(iSampleRate); }
	float sampleRate() const
		{ return uint32_t(m_srate); }

	// init.
	void reset(Shape shape = Pulse, float width = 1.0f)
	{
		m_shape = shape;
		m_width = width;;

		switch (m_shape) {
		case Pulse:
			reset_pulse();
			break;
		case Saw:
			reset_saw();
			break;
		case Sine:
			reset_sine();
			break;
		case Noise:
			reset_noise();
			// thru...
		default:
			break;
		}
	}

	// begin.
	float start(float& phase, float pshift = 0.0f, float freq = 0.0f) const
	{
		const float p0 = float(m_nsize);

		phase = m_phase0 + pshift * p0;
		if (phase >= p0)
			phase -= p0;

		return sample(phase, freq);
	}

	// iterate.
	float sample(float& phase, float freq) const
	{
		const uint32_t i = uint32_t(phase);
		const float alpha = phase - float(i);
		const float p0 = float(m_nsize);

		phase += p0 * freq / m_srate;
		if (phase >= p0)
			phase -= p0;

		// cubic interpolation...
		const float x0 = m_table[i];
		const float x1 = m_table[i + 1];
		const float x2 = m_table[i + 2];
		const float x3 = m_table[i + 3];

		const float c1 = (x2 - x0) * 0.5f;
		const float b1 = (x1 - x2);
		const float b2 = (c1 + b1);
		const float c3 = (x3 - x1) * 0.5f + b2 + b1;
		const float c2 = (c3 + b2);

		return (((c3 * alpha) - c2) * alpha + c1) * alpha + x1;
	}

	// absolute value.
	float value(float phase) const
	{
		const float p0 = float(m_nsize);

		phase *= p0;
		phase += m_phase0;
		if (phase >= p0)
			phase -= p0;

		return m_table[uint32_t(phase)];
	}

protected:

	// init pulse table.
	void reset_pulse()
	{
		const float p0 = float(m_nsize);
		const float w2 = p0 * m_width * 0.5f;

		for (uint32_t i = 0; i < m_nsize; ++i) {
			const float p = float(i);
			m_table[i] = (p < w2 ? 1.0f : -1.0f);
		}

		reset_filter();
		reset_normalize();
		reset_interp();
	}

	// init saw table.
	void reset_saw()
	{
		const float p0 = float(m_nsize);
		const float w0 = p0 * m_width;

		for (uint32_t i = 0; i < m_nsize; ++i) {
			const float p = float(i);
			if (p < w0) {
				m_table[i] = 2.0f * p / w0 - 1.0f;
			} else {
				m_table[i] = 1.0f - 2.0f * (1.0f + (p - w0)) / (p0 - w0);
			}
		}

		reset_filter();
		reset_normalize();
		reset_interp();
	}

	// init sine table.
	void reset_sine()
	{
		const float p0 = float(m_nsize);
		const float w0 = p0 * m_width;
		const float w2 = w0 * 0.5f;

		for (uint32_t i = 0; i < m_nsize; ++i) {
			float p = float(i);
			if (p < w2)
				m_table[i] = ::sinf(2.0f * M_PI * p / w0);
			else
				m_table[i] = ::sinf(M_PI * (p + (p0 - w0)) / (p0 - w2));
		}

		if (m_width < 1.0f) {
			reset_filter();
			reset_normalize();
		}

		reset_interp();
	}

	// init noise table.
	void reset_noise()
	{
		const float p0 = float(m_nsize);
		const float w0 = p0 * m_width;
		const uint32_t ihold = (uint32_t(p0 - w0) >> 3) + 1;

		::srand(long(this));

		float p = 0.0f;

		for (uint32_t i = 0; i < m_nsize; ++i) {
			if ((i % ihold) == 0)
				p = (2.0f * float(::rand()) / float(RAND_MAX)) - 1.0f;
			m_table[i] = p;
		}

		reset_filter();
		reset_normalize();
		reset_interp();
	}

	// post-processors
	void reset_filter()
	{
		uint32_t i, k = 0;

		for (i = 1; i < m_nsize; ++i) {
			const float p1 = m_table[i - 1];
			const float p2 = m_table[i];
			if (p1 < 0.0f && p2 >= 0.0f) {
				k = i;
				break;
			}
		}

		for (uint16_t n = 0; n < m_nover; ++n) {
			float p = m_table[k];
			for (i = 0; i < m_nsize; ++i) {
				if (++k >= m_nsize) k = 0;
				p = 0.5f * (m_table[k] + p);
				m_table[k] = p;
			}
		}
	}

	void reset_normalize()
	{
		uint32_t i;

		float pmax = 0.0f;
		float pmin = 0.0f;

		for (i = 0; i < m_nsize; ++i) {
			const float p = m_table[i];
			if (pmax < p)
				pmax = p;
			else
			if (pmin > p)
				pmin = p;
		}

		const float pmid = 0.5f * (pmax + pmin);

		pmax = 0.0f;
		for (i = 0; i < m_nsize; ++i) {
			m_table[i] -= pmid;
			const float p = ::fabs(m_table[i]);
			if (pmax < p)
				pmax = p;
		}

		if (pmax > 0.0f) {
			const float gain = 1.0f / pmax;
			for (i = 0; i < m_nsize; ++i)
				m_table[i] *= gain;
		}
	}

	void reset_interp()
	{
		uint32_t i, pk = 0;

		for (i = m_nsize; i < m_nsize + 4; ++i)
			m_table[i] = m_table[i - m_nsize];

		for (i = 1; i < m_nsize; ++i) {
			const float p1 = m_table[i - 1];
			const float p2 = m_table[i];
			if (p1 < 0.0f && p2 >= 0.0f)
				pk = i;
		}

		m_phase0 = float(pk);
	}

private:

	uint32_t m_nsize;
	uint16_t m_nover;

	Shape    m_shape;
	float    m_width;

	float    m_srate;
	float   *m_table;
	float    m_phase0;
};


//-------------------------------------------------------------------------
// synthv1_wave_lf - hard/non-smoothed wave table (eg. LFO).
//

class synthv1_wave_lf : public synthv1_wave
{
public:

	// ctor.
	synthv1_wave_lf(uint32_t nsize = 128)
		: synthv1_wave(nsize, 0) {}
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
		{ m_wave = wave; m_phase = 0.0f; }

	synthv1_wave *wave() const
		{ return m_wave; }

	// begin.
	float start(float pshift = 0.0f)
		{ return m_wave->start(m_phase, pshift); }

	// iterate.
	float sample(float freq)
		{ return m_wave->sample(m_phase, freq); }

private:

	synthv1_wave *m_wave;

	float m_phase;
};


#endif	// __synthv1_wave_h

// end of synthv1_wave.h
