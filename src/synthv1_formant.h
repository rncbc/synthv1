// synthv1_formant.h
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

#ifndef __synthv1_formant_h
#define __synthv1_formant_h

#include <stdint.h>
#include <math.h>


//---------------------------------------------------------------------
// synthv1_formant - formant parallel filter after Dennis H. Klatt's
//                 Software for a cascade/parallel formant synthesizer.
//

class synthv1_formant
{
public:

	// ctor.
	synthv1_formant(float srate = 44100.0f)
		: m_srate(srate) { reset(); }

	// sample-rate accessors
	void setSampleRate(float srate)
		{ m_srate = srate; }
	float sampleRate() const
		{ return m_srate; }

	// reset method
	void reset(float cutoff = 0.5f, float reso = 0.0f)
	{
		m_cutoff = cutoff;
		m_reso = reso;

		const float   fK = cutoff * float(NUM_VTABS);
		const uint32_t k = uint32_t(fK);
		const float   fJ = (fK - float(k)) * float(NUM_VOWELS);
		const uint32_t j = uint32_t(fJ);
		const float   fX = (fJ - float(j)); // linear morph fract.

		const float q = 4.0f * m_reso * m_reso + 1.0f;
		const float p = 1.0f / q;

		// vocal formant morphing
		Coeff coeff1, coeff2;
		const Vtab *vtabs = g_vtabs[k];
		const Vtab& vtab1 = vtabs[j];
		const Vtab& vtab2 = (j < NUM_VOWELS - 1 ? vtabs[j + 1] : vtab1);
		for (uint32_t i = 0; i < NUM_FORMANTS; ++i) {
			vtab_coeffs(coeff1, vtab1, i, p);
			vtab_coeffs(coeff2, vtab2, i, p);
			coeff1.a += fX * (coeff2.a - coeff1.a);
			coeff1.b += fX * (coeff2.b - coeff1.b);
			coeff1.c += fX * (coeff2.c - coeff1.c);
			m_filters[i].reset_coeffs(coeff1);
		}
	}

	// output tick
	float output(float in, float cutoff, float reso)
	{
		if (::fabs(m_cutoff - cutoff) > 0.001f ||
			::fabs(m_reso - reso) > 0.001f)
			reset(cutoff, reso);

		float out = 0.0f;
		for (uint32_t i = 0; i < NUM_FORMANTS; ++i)
			out += m_filters[i].output(in);
		return out;
	}

	// process block
	void process(float *in, uint32_t nframes, float wet, float cutoff, float reso)
	{
		for (uint32_t i = 0; i < nframes; ++i) {
			const float out = output(in[i], cutoff, reso);
			in[i] *= (1.0f - wet);
			in[i] += (wet * out);
		}
	}

	// constants
	static const uint32_t NUM_VTABS = 5;
	static const uint32_t NUM_VOWELS = 5;
	static const uint32_t NUM_FORMANTS = 5;
	
	struct Vtab
	{
		float F[NUM_FORMANTS];	// frequency [Hz]
		float G[NUM_FORMANTS];	// peak gain [dB]
		float B[NUM_FORMANTS];	// bandwidth [Hz]
	};

	struct Coeff { float a, b, c; };

protected:

	// 2-pole resonator filter
	class Filter
	{
	public:

		Filter() { reset(); }

		void reset()
		{
			m_a = m_b = m_c = 0.0f;
			m_out1 = m_out2 = 0.0f;
		}
	
		void reset_coeffs(const Coeff& coeff)
		{
			m_a = coeff.a;
			m_b = coeff.b;
			m_c = coeff.c;
		}
	
		float output(float in)
		{
			const float out = m_a * in + m_b * m_out1 - m_c * m_out2;
			m_out2 = m_out1;
			m_out1 = out;
			return out;
		}
	
	private:

		float m_a, m_b, m_c;
		float m_out1, m_out2;
	};
	
	void reset_filters()
	{
		for (uint32_t i = 0; i < NUM_FORMANTS; ++i)
			m_filters[i].reset();
	}

	// compute coeffs. for given vocal formant table
	void vtab_coeffs(Coeff& coeff, const Vtab& vtab, uint32_t i, float p)
	{
		const float Fi = vtab.F[i];
		const float Gi = vtab.G[i];
		const float Bi = vtab.B[i] * p;
		const float Ai = ::powf(10.0f, (Gi / 20.0f));

		coeff.c = ::expf(-2.0f * M_PI * Bi / m_srate);
		coeff.b = 2.0f * ::expf(- M_PI * Bi / m_srate)
				* ::cosf(2.0f * M_PI * Fi / m_srate);
		coeff.a = Ai * (1.0f - coeff.b + coeff.c);
	}

private:

	// instance members
	float m_srate;

	// parameters
	float m_cutoff;
	float m_reso;

	// formant filters
	Filter m_filters[NUM_FORMANTS];

	// base vocal tables.
	static Vtab  g_bass_vtab[NUM_VOWELS];
	static Vtab  g_tenor_vtab[NUM_VOWELS];
	static Vtab  g_countertenor_vtab[NUM_VOWELS];
	static Vtab  g_soprano_vtab[NUM_VOWELS];
	static Vtab  g_alto_vtab[NUM_VOWELS];

	static Vtab *g_vtabs[NUM_VTABS];
};


#endif	// __synthv1_formant_h

// end of synthv1_formant.h
