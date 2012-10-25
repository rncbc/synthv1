// synthv1_wave.h
//
/****************************************************************************
   Copyright (C) 2012, rncbc aka Rui Nuno Capela. All rights reserved.

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
			m_shape(Pulse), m_width(1.0f), m_srate(44100)
	{
		m_table0 = new float [m_nsize];
		m_table1 = new float [m_nsize];

		reset(m_shape, m_width);
	}

	// dtor.
	~synthv1_wave()
	{
		delete [] m_table0;
		delete [] m_table1;
	}

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

		const float p0 = float(m_nsize);
		const float w0 = p0 * m_width;
		const float w2 = w0 * 0.5f;

		const uint32_t ihold = (uint32_t(p0 - w0) >> 3) + 1;
		if (m_shape == Noise)
			::srand(long(this));
		float phold = 0.0f;

		for (uint32_t i = 0; i < m_nsize; ++i) {
			float p = float(i);
			switch (m_shape) {
			case Pulse:
				p = (p < w2 ? 1.0f : -1.0f);
				break;
			case Saw:
				if (p < w0)
					p = 2.0f * p / w0 - 1.0f;
				else
					p = 1.0f - 2.0f * (1.0f + (p - w0)) / (p0 - w0);
				break;
			case Sine:
				if (p < w2)
					p = ::sinf(2.0f * M_PI * p / w0);
				else
					p = ::sinf(M_PI * (p + (p0 - w0))/ (p0 - w2));
				break;
			case Noise:
			default:
				if ((i % ihold) == 0)
					phold = (2.0f * float(::rand()) / float(RAND_MAX)) - 1.0f;
				p = phold;
				break;
			}
			m_table0[i] = p;
		}

		if (m_nover > 0)
			reset_filter();
		if (m_shape == Noise)
			reset_normalize();

		reset_interp();
	}

	// begin.
	float start(float& phase, float pshift = 0.0f) const
	{
		const float p0 = float(m_nsize);

		phase = m_phase0 + pshift * p0;
		if (phase >= p0)
			phase -= p0;

		return sample(phase);
	}

	// iterate.
	float sample(float& phase, float freq = 0.0f) const
	{
		const uint32_t k = uint32_t(phase);
		const float alpha = phase - float(k);
		const float p0 = float(m_nsize);

		phase += p0 * freq / m_srate;
		if (phase >= p0)
			phase -= p0;

		return m_table0[k] + alpha * m_table1[k];
	}

	// absolute value.
	float value(float phase) const
	{
		const float p0 = float(m_nsize);

		phase *= float(m_nsize);
		phase += m_phase0;
		if (phase >= p0)
			phase -= p0;

		return m_table0[uint32_t(phase)];
	}

protected:

	void reset_filter()
	{
		uint32_t i, k = 0;

		for (i = 1; i < m_nsize; ++i) {
			const float p1 = m_table0[i - 1];
			const float p2 = m_table0[i];
			if (p1 < 0.0f && p2 >= 0.0f) {
				k = i;
				break;
			}
		}

		for (uint16_t n = 0; n < m_nover; ++n) {
			float p = m_table0[k];
			for (i = 0; i < m_nsize; ++i) {
				if (++k >= m_nsize) k = 0;
				p = 0.5f * (m_table0[k] + p);
				m_table0[k] = p;
			}
		}
	}

	void reset_normalize()
	{
		uint32_t i;

		float pmax = 0.0f;
		float pmin = 0.0f;

		for (i = 0; i < m_nsize; ++i) {
			const float p = m_table0[i];
			if (pmax < p)
				pmax = p;
			else
			if (pmin > p)
				pmin = p;
		}

		const float pmid = 0.5f * (pmax + pmin);

		pmax = 0.0f;
		for (i = 0; i < m_nsize; ++i) {
			m_table0[i] -= pmid;
			const float p = ::fabs(m_table0[i]);
			if (pmax < p)
				pmax = p;
		}

		if (pmax > 0.0f) {
			const float gain = 1.0f / pmax;
			for (i = 0; i < m_nsize; ++i)
				m_table0[i] *= gain;
		}
	}

	void reset_interp()
	{
		uint32_t pk = 0;

		m_table1[0] = m_table0[m_nsize - 1] - m_table0[0];

		for (uint32_t i = 1; i < m_nsize; ++i) {
			const float p1 = m_table0[i - 1];
			const float p2 = m_table0[i];
			m_table1[i] = p2 - p1;
			if (p1 < 0.0f && p2 >= 0.0f)
				pk = i;
		}

		m_phase0 = float(pk);
	}

private:

	uint32_t m_nsize;
	uint16_t m_nover;

	Shape  m_shape;
	float  m_width;

	float  m_srate;

	float *m_table0;
	float *m_table1;

	float  m_phase0;
};


//-------------------------------------------------------------------------
// synthv1_wave - smoothed (integrating oversampled) oscillator

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
