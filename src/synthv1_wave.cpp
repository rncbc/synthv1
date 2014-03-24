// synthv1_wave.cpp
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

#include "synthv1_wave.h"

#include <stdlib.h>
#include <math.h>


//-------------------------------------------------------------------------
// synthv1_wave_sched - local module schedule thread stuff.
//

#include "synthv1_sched.h"


class synthv1_wave_sched : public synthv1_sched
{
public:

	// ctor.
	synthv1_wave_sched (synthv1_wave *wave)	: synthv1_sched(),
		m_wave(wave), m_shape(synthv1_wave::Pulse), m_width(1.0f) {}

	// schedule reset.
	void reset(synthv1_wave::Shape shape, float width)
	{
		m_shape = shape;
		m_width = width;

		schedule();
	}

	// process reset (virtual).
	void process()
	{
		m_wave->reset_sync(m_shape, m_width);
	}

private:

	// instance variables.
	synthv1_wave *m_wave;
	synthv1_wave::Shape m_shape;
	float m_width;
};


//-------------------------------------------------------------------------
// synthv1_wave - smoothed (integrating oversampled) wave table.
//

// ctor.
synthv1_wave::synthv1_wave ( uint32_t nsize, uint16_t nover, uint16_t ntabs )
	: m_nsize(nsize), m_nover(nover), m_ntabs(ntabs),
		m_shape(Saw), m_width(1.0f), m_srate(44100.0f),
		m_min_freq(0.0f), m_max_freq(0.0f), m_ftab(0.0f), m_itab(0),
		m_sched(NULL)
{
	const uint16_t ntabs1 = m_ntabs + 1;

	m_tables = new float * [ntabs1];
	for (uint16_t itab = 0; itab < ntabs1; ++itab)
		m_tables[itab] = new float [m_nsize + 4];

	reset(m_shape, m_width);

	if (ntabs > 0)
		m_sched = new synthv1_wave_sched(this);
}


// dtor.
synthv1_wave::~synthv1_wave (void)
{
	if (m_sched)
		delete m_sched;

	const uint16_t ntabs1 = m_ntabs + 1;

	for (uint16_t itab = 0; itab < ntabs1; ++itab)
		delete [] m_tables[itab];

	delete [] m_tables;
}


// init.
void synthv1_wave::reset ( Shape shape, float width )
{
	if (m_sched)
		m_sched->reset(shape, width);
	else
		reset_sync(shape, width);
}


void synthv1_wave::reset_sync ( Shape shape, float width )
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


// init pulse table.
void synthv1_wave::reset_pulse (void)
{
	for (uint16_t n = 0; n < m_ntabs; ++n)
		reset_pulse_part(n, 1 << n);

	reset_pulse_part(m_ntabs, 0);

	m_max_freq = (0.25f * m_srate);
	m_min_freq = m_max_freq / float(1 << m_ntabs);
}


// init saw table.
void synthv1_wave::reset_saw (void)
{
	for (uint16_t n = 0; n < m_ntabs; ++n)
		reset_saw_part(n, 1 << n);

	reset_saw_part(m_ntabs, 0);

	m_max_freq = (0.25f * m_srate);
	m_min_freq = m_max_freq / float(1 << m_ntabs);
}


// init sine table.
void synthv1_wave::reset_sine (void)
{
	if (m_width < 1.0f) {
		for (uint16_t itab = 0; itab < m_ntabs; ++itab)
			reset_sine_part(itab);
	}

	reset_sine_part(m_ntabs);

	if (m_width < 1.0f) {
		m_max_freq = (0.25f * m_srate);
		m_min_freq = m_max_freq / float(1 << m_ntabs);
	} else {
		m_max_freq = (0.5f * m_srate);
		m_min_freq = m_max_freq;
	}
}


// init noise table.
void synthv1_wave::reset_noise (void)
{
	const float p0 = float(m_nsize);
	const float w0 = p0 * m_width;
	const uint32_t ihold = (uint32_t(p0 - w0) >> 3) + 1;

	float *frames = m_tables[m_ntabs];

	::srand(long(this));

	float p = 0.0f;

	for (uint32_t i = 0; i < m_nsize; ++i) {
		if ((i % ihold) == 0)
			p = (2.0f * float(::rand()) / float(RAND_MAX)) - 1.0f;
		frames[i] = p;
	}

	reset_filter(m_ntabs);
	reset_normalize(m_ntabs);
	reset_interp(m_ntabs);

	m_max_freq = (0.5f * m_srate);
	m_min_freq = m_max_freq;
}


// init pulse partial table.
void synthv1_wave::reset_pulse_part ( uint16_t itab, uint16_t nparts )
{
	const float p0 = float(m_nsize);
	const float w2 = p0 * m_width * 0.5f + 0.001f;

	float *frames = m_tables[itab];

	for (uint32_t i = 0; i < m_nsize; ++i) {
		const float p = float(i);
		if (nparts > 0) {
			const float gibbs = 0.5f * M_PI / float(nparts);
			float sum = 0.0f;
			for (uint32_t n = 0; n < nparts; ++n) {
				const float gn = ::cosf(gibbs * float(n));
				const float dn = float(n + 1) * M_PI;
				const float wn = 2.0f * dn;
				const float g2 = gn * gn / dn;
				sum += g2 * ::sinf(wn * (w2 - p) / p0);
				sum += g2 * ::sinf(wn * (p - p0) / p0);
			}
			frames[i] = 2.0f * sum;
		} else {
			frames[i] = (p < w2 ? 1.0f : -1.0f);
		}
	}

	reset_filter(itab);
	reset_normalize(itab);
	reset_interp(itab);
}


// init saw partial table.
void synthv1_wave::reset_saw_part ( uint16_t itab, uint16_t nparts )
{
	const float p0 = float(m_nsize);
	const float w0 = p0 * m_width;

	float *frames = m_tables[itab];

	for (uint32_t i = 0; i < m_nsize; ++i) {
		const float p = float(i);
		if (nparts > 0) {
			const float gibbs = 0.5f * M_PI / float(nparts);
			float sum = 0.0f;
			float sgn = 2.0f;
			for (uint32_t n = 0; n < nparts; ++n) {
				const float gn = ::cosf(gibbs * float(n));
				const float dn = float(n + 1) * M_PI;
				const float wn = 2.0f * dn;
				const float g2 = gn * gn / dn;
				if (w0 < 1.0f)
					sum += g2 * ::sinf(wn * p / p0);
				else
				if (w0 >= p0) 
					sum += g2 * ::sinf(wn * (p0 - p) / p0);
				else {
					sum -= sgn * g2 * ::cosf(wn * (w0 - p) / p0) / dn;
					sum += sgn * g2 * ::cosf(wn * (p - p0) / p0) / dn;
					sgn = -sgn;
				}
			}
			frames[i] = 2.0f * sum;
		}
		else if (p < w0) {
			frames[i] = 2.0f * p / w0 - 1.0f;
		} else {
			frames[i] = 1.0f - 2.0f * (1.0f + (p - w0)) / (p0 - w0);
		}
	}

	reset_filter(itab);
	reset_normalize(itab);
	reset_interp(itab);
}


// init sine partial table.
void synthv1_wave::reset_sine_part ( uint16_t itab )
{
	const float width = (itab >= m_ntabs ? m_width
		: 1.0f + float(itab) * (m_width - 1.0f) / float(m_ntabs));

	const float p0 = float(m_nsize);
	const float w0 = p0 * width;
	const float w2 = w0 * 0.5f;

	float *frames = m_tables[itab];

	for (uint32_t i = 0; i < m_nsize; ++i) {
		float p = float(i);
		if (p < w2)
			frames[i] = ::sinf(2.0f * M_PI * p / w0);
		else
			frames[i] = ::sinf(M_PI * (p + (p0 - w0)) / (p0 - w2));
	}

	if (width < 1.0f) {
		reset_filter(itab);
		reset_normalize(itab);
	}
	reset_interp(itab);
}


// post-processors.
void synthv1_wave::reset_filter ( uint16_t itab )
{
	float *frames = m_tables[itab];

	uint32_t i, k = 0;

	for (i = 1; i < m_nsize; ++i) {
		const float p1 = frames[i - 1];
		const float p2 = frames[i];
		if (p1 < 0.0f && p2 >= 0.0f) {
			k = i;
			break;
		}
	}

	for (uint16_t n = 0; n < m_nover; ++n) {
		float p = frames[k];
		for (i = 0; i < m_nsize; ++i) {
			if (++k >= m_nsize) k = 0;
			p = 0.5f * (frames[k] + p);
			frames[k] = p;
		}
	}
}


void synthv1_wave::reset_normalize ( uint16_t itab )
{
	float *frames = m_tables[itab];
	
	uint32_t i;

	float pmax = 0.0f;
	float pmin = 0.0f;

	for (i = 0; i < m_nsize; ++i) {
		const float p = frames[i];
		if (pmax < p)
			pmax = p;
		else
		if (pmin > p)
			pmin = p;
	}

	const float pmid = 0.5f * (pmax + pmin);

	pmax = 0.0f;
	for (i = 0; i < m_nsize; ++i) {
		frames[i] -= pmid;
		const float p = ::fabs(frames[i]);
		if (pmax < p)
			pmax = p;
	}

	if (pmax > 0.0f) {
		const float gain = 1.0f / pmax;
		for (i = 0; i < m_nsize; ++i)
			frames[i] *= gain;
	}
}


void synthv1_wave::reset_interp ( uint16_t itab )
{
	float *frames = m_tables[itab];

	uint32_t i;

	for (i = m_nsize; i < m_nsize + 4; ++i)
		frames[i] = frames[i - m_nsize];

	if (itab == m_ntabs) {
		uint32_t pk = 0;
		for (i = 1; i < m_nsize; ++i) {
			const float p1 = frames[i - 1];
			const float p2 = frames[i];
			if (p1 < 0.0f && p2 >= 0.0f)
				pk = i;
		}
		m_phase0 = float(pk);
	}
}


// end of synthv1_wave.cpp
