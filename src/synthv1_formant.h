// synthv1_formant.h
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

#ifndef __synthv1_formant_h
#define __synthv1_formant_h

#include <stdint.h>
#include <math.h>


//---------------------------------------------------------------------
// synthv1_formant - formant parallel filter after Dennis H. Klatt's
//                   Software for a cascade/parallel formant synthesizer
//                   1979 MIT; 1980 Acoustical Society of America.
//

class synthv1_formant
{
public:

	// constants
	static const uint32_t NUM_VTABS = 5;
	static const uint32_t NUM_VOWELS = 5;
	static const uint32_t NUM_FORMANTS = 5;

	// 2-pole filter coeffs.
	struct Coeffs { float a0, b1, b2; };

	// vocal/vowel table
	struct Vtab
	{
		float freq[NUM_FORMANTS];	// frequency [Hz]
		float gain[NUM_FORMANTS];	// peak gain [dB]
		float band[NUM_FORMANTS];	// bandwidth [Hz]
	};

	// main impl.
	class Impl
	{
	public:

		// ctor.
		Impl(float srate = 44100.0f)
			: m_srate(srate), m_cutoff(0.0f), m_reso(0.0f)
			{ reset_coeffs(); }

		// sample-rate accessors
		void setSampleRate(float srate)
			{ m_srate = srate; reset_coeffs(); }
		float sampleRate() const
			{ return m_srate; }

		// formant coeffs. accessor
		const Coeffs& coeffs(uint32_t i) const
			{ return m_ctabs[i]; }

		// update method
		void update(float cutoff = 0.5f, float reso = 0.0f)
		{
			if (::fabsf(m_cutoff - cutoff) > 0.001f ||
				::fabsf(m_reso   - reso)   > 0.001f) {
				m_cutoff = cutoff;
				m_reso = reso;
				reset_coeffs();
			}
		}

	protected:

		// compute coeffs. for given vocal formant table
		void vtab_coeffs(Coeffs& coeffs, const Vtab *vtab, uint32_t i, float p);

		// reset coeffs. method
		void reset_coeffs();

	private:

		// instance members
		float m_srate;

		// parameters
		float m_cutoff;
		float m_reso;

		// filter coeffs.
		Coeffs m_ctabs[NUM_FORMANTS];
	};

	// ctor.
	synthv1_formant(Impl *pImpl = 0)
		: m_pImpl(pImpl), m_cutoff(0.0f), m_reso(0.0f)
		{ reset_coeffs(); }

	// reset impl.
	void reset(Impl *pImpl)
		{ m_pImpl = pImpl; reset_coeffs(); }

	void reset_filters(float cutoff, float reso)
	{
		for (uint32_t i = 0; i < NUM_FORMANTS; ++i)
			m_filters[i].reset();

		update(cutoff, reso);
	}

	// output tick
	float output(float in, float cutoff, float reso)
	{
		update(cutoff, reso);

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

protected:

	// step-wise smoothed coeff.
	class Coeff
	{
	public:

		Coeff() { reset(); }

		void reset()
		{
			m_value = m_vstep = 0.0f;
			m_nstep = 0;
		}

		void set_value(float value)
		{
			const uint32_t NUM_STEPS = 32;
			m_nstep = NUM_STEPS;
			m_vstep = (value - m_value) / float(m_nstep);
		}

		float value() const
			{ return m_value + m_vstep * float(m_nstep); }

		float tick()
		{
			if (m_nstep > 0) {
				m_value += m_vstep;
				--m_nstep;
			}
			return m_value;
		}

	private:

		float    m_value;
		float    m_vstep;
		uint32_t m_nstep;
	};

	// 2-pole resonator filter
	class Filter
	{
	public:

		Filter() { reset(); }

		void reset()
		{
			m_a0.reset();
			m_b1.reset();
			m_b2.reset();

			m_out1 = m_out2 = 0.0f;
		}

		void reset_coeffs(const Coeffs& coeffs)
		{
			m_a0.set_value(coeffs.a0);
			m_b1.set_value(coeffs.b1);
			m_b2.set_value(coeffs.b2);
		}

		float output(float in)
		{
			const float out
				= m_a0.tick() * in
				+ m_b1.tick() * m_out1
				- m_b2.tick() * m_out2;

			m_out2 = m_out1;
			m_out1 = out;
			return out;
		}

	private:

		Coeff m_a0, m_b1, m_b2;
		float m_out1, m_out2;
	};

	// update method
	void update(float cutoff, float reso)
	{
		if (::fabsf(m_cutoff - cutoff) > 0.001f ||
			::fabsf(m_reso   - reso)   > 0.001f) {
			m_cutoff = cutoff;
			m_reso = reso;
			reset_coeffs();
		}
	}

	// reset coeffs. method
	void reset_coeffs();

private:

	// instance members
	Impl *m_pImpl;

	// parameters.
	float m_cutoff;
	float m_reso;

	// formant filters
	Filter m_filters[NUM_FORMANTS];

	// base vocal tables
	static Vtab  g_bass_vtab[NUM_VOWELS];
	static Vtab  g_tenor_vtab[NUM_VOWELS];
	static Vtab  g_countertenor_vtab[NUM_VOWELS];
	static Vtab  g_soprano_vtab[NUM_VOWELS];
	static Vtab  g_alto_vtab[NUM_VOWELS];

	static Vtab *g_vtabs[NUM_VTABS];
};


#endif	// __synthv1_formant_h

// end of synthv1_formant.h
