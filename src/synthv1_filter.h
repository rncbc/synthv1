// synthv1_filter.h
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

#ifndef __synthv1_filter_h
#define __synthv1_filter_h

#include <stdint.h>
#include <stdlib.h>
#include <math.h>


//-------------------------------------------------------------------------
// synthv1_filter1 - Hal Chamberlin's State Variable (12dB/oct) filter
//

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

	float output(float in, float cutoff, float reso)
	{
		const float q = (1.0f - reso);

		for (uint16_t i = 0; i < m_nover; ++i) {
			m_low  += cutoff * m_band;
			m_high  = in - m_low - q * m_band;
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


//-------------------------------------------------------------------------
// synthv1_filter2 - Stilson/Smith Moog (24dB/oct) filter
//

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
		m_t1 = m_t2 = 0.0f;
	}

	float output(float in, float cutoff, float reso)
	{
		const float c = 1.0f - cutoff;
		const float p = cutoff + 0.8f * cutoff * c;
		const float f = p + p - 1.0f;
		const float q = reso * (1.0f + 0.5f * c * (1.0f - c + 5.6f * c * c));

		in -= q * m_b4; // feedback

		m_t1 = m_b1; m_b1 = (in   + m_b0) * p - m_b1 * f;
		m_t2 = m_b2; m_b2 = (m_b1 + m_t1) * p - m_b2 * f;
		m_t1 = m_b3; m_b3 = (m_b2 + m_t2) * p - m_b3 * f;

		m_b4 = (m_b3 + m_t1) * p - m_b4 * f;
		m_b4 = m_b4 - m_b4 * m_b4 * m_b4 * 0.166667f; // clipping

		m_b0 = in;

		switch (m_type) {
		case Notch:
			return 3.0f * (m_b3 - m_b4) - in;
		case High:
			return in - m_b4;
		case Band:
			return 3.0f * (m_b3 - m_b4);
		case Low:
		default:
			return m_b4;
		}
	}

private:

	// filter type 
	Type  m_type;

	float m_b0, m_b1, m_b2, m_b3, m_b4;
	float m_t1, m_t2;
};


//-------------------------------------------------------------------------
// synthv1_filter3 - RBJ biquad filter implementation.
//
//   http://www.musicdsp.org/files/Audio-EQ-Cookbook.txt

class synthv1_filter3
{
public:

	enum Type { Low = 0, Band, High, Notch };

	synthv1_filter3(Type type = Low)
		: m_type(type), m_cutoff(0.5f), m_reso(0.0f) { reset(type); }

	Type type() const
		{ return m_type; }

	void reset(Type type)
	{
		m_type = type;

		m_out1 = m_out2 = 0.0f;
		m_in1 = m_in2 = 0.0f;

		reset();
	}

	float output(float in, float cutoff, float reso)
	{
		// parameter changes
		if (::fabsf(m_cutoff - cutoff) > 0.001f ||
			::fabsf(m_reso   - reso)   > 0.001f) {
			m_cutoff = cutoff;
			m_reso = reso;
			reset();
		}

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
		const float q = 2.0f * m_reso * m_reso + 1.0f;

		const float omega = M_PI * m_cutoff;
		const float tsin  = ::sinf(omega);
		const float tcos  = ::cosf(omega);
		const float alpha = tsin / (2.0f * q);

		// temp vars
		const float a0 =  1.0f + alpha;
		const float a1 = -2.0f * tcos;
		const float a2 =  1.0f - alpha;

		float b0, b1, b2;

		switch (m_type) {
		case Notch:
			b0 =  1.0f;
			b1 = -2.0f * tcos;
			b2 =  1.0f;
			break;
		case High:
			b0 = (1.0f + tcos) / 2.0f;
			b1 = -1.0f - tcos;
			b2 =  b0;
			break;
		case Band:
			b0 =  tsin / 2.0f;
			b1 =  0.0f;
			b2 = -b0;
			break;
		case Low:
		default:
			b0 = (1.0f - tcos) / 2.0f;
			b1 =  1.0f - tcos;
			b2 =  b0;
			break;
		}

		// set filter coeffs
		m_b0a0 = b0 / a0;
		m_b1a0 = b1 / a0;
		m_b2a0 = b2 / a0;
		m_a1a0 = a1 / a0;
		m_a2a0 = a2 / a0;
	}

private:

	// filter type 
	Type  m_type;

	// filter params
	float m_cutoff;
	float m_reso;

	// filter coeffs
	float m_b0a0, m_b1a0, m_b2a0, m_a1a0, m_a2a0;

	// in/out history
	float m_out1, m_out2, m_in1, m_in2;
};


#endif	// __synthv1_filter_h


// end of synthv1_filter.h
