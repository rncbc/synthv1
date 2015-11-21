// synthv1_formant.cpp
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

#include "synthv1_formant.h"


//---------------------------------------------------------------------
// synthv1_formant - formant parallel filter after Dennis H. Klatt's
//                   Software for a cascade/parallel formant synthesizer
//                   1979 MIT; 1980 Acoustical Society of America.
//

// formant tables.
//
// The Canonical Csound Reference Manual
// Appendix D. Formant Values
// http://www.csounds.com/manual/html/MiscFormants.html
//

synthv1_formant::Vtab synthv1_formant::g_alto_vtab[NUM_VOWELS] = {
	// Table D.1. alto “a”
	{{  800.0f, 1150.0f, 2800.0f, 3500.0f, 4950.0f },
	 {    0.0f,   -4.0f,  -20.0f,  -36.0f,  -60.0f },
	 {   80.0f,   90.0f,  120.0f,  130.0f,  140.0f }},
	// Table D.2. alto “e”
	{{  400.0f, 1600.0f, 2700.0f, 3300.0f, 4950.0f },
	 {    0.0f,  -24.0f,  -30.0f,  -35.0f,  -60.0f },
	 {   60.0f,   80.0f,  120.0f,  150.0f,  200.0f }},
	// Table D.3. alto “i”
	{{  350.0f, 1700.0f, 2700.0f, 3700.0f, 4950.0f },
	 {    0.0f,  -20.0f,  -30.0f,  -36.0f,  -60.0f },
	 {   50.0f,  100.0f,  120.0f,  150.0f,  200.0f }},
	// Table D.4. alto “o”
	{{  450.0f,  800.0f, 2830.0f, 3500.0f, 4950.0f },
	 {    0.0f,   -9.0f,  -16.0f,  -28.0f,  -55.0f },
	 {   70.0f,   80.0f,  100.0f,  130.0f,  135.0f }},
	// Table D.5. alto “u”
	{{  325.0f,  700.0f, 2530.0f, 3500.0f, 4950.0f },
	 {    0.0f,  -12.0f,  -30.0f,  -40.0f,  -64.0f },
	 {   50.0f,   60.0f,  170.0f,  180.0f,  200.0f }}
};

synthv1_formant::Vtab synthv1_formant::g_bass_vtab[NUM_VOWELS] = {
	// Table D.6. bass “a”
	{{  600.0f, 1040.0f, 2250.0f, 2450.0f, 2750.0f },
	 {    0.0f,   -7.0f,   -9.0f,   -9.0f,  -20.0f },
	 {   60.0f,   70.0f,  110.0f,  120.0f,  130.0f }},
	// Table D.7. bass “e”
	{{  400.0f, 1620.0f, 2400.0f, 2800.0f, 3100.0f },
	 {    0.0f,  -12.0f,   -9.0f,  -12.0f,  -18.0f },
	 {   40.0f,   80.0f,  100.0f,  120.0f,  120.0f }},
	// Table D.8. bass “i”
	{{  250.0f, 1750.0f, 2600.0f, 3050.0f, 3340.0f },
	 {    0.0f,  -30.0f,  -16.0f,  -22.0f,  -28.0f },
	 {   60.0f,   90.0f,  100.0f,  120.0f,  120.0f }},
	// Table D.9. bass “o”
	{{  400.0f,  750.0f, 2400.0f, 2600.0f, 2900.0f },
	 {    0.0f,  -11.0f,  -21.0f,  -20.0f,  -40.0f },
	 {   40.0f,   80.0f,  100.0f,  120.0f,  120.0f }},
	// Table D.10. bass “u”
	{{  350.0f, 600.0f,  2400.0f, 2675.0f, 2950.0f },
	 {    0.0f,  -20.0f,  -32.0f,  -28.0f,  -36.0f },
	 {   40.0f,   80.0f,  100.0f,  120.0f,  120.0f }}
};

synthv1_formant::Vtab synthv1_formant::g_countertenor_vtab[NUM_VOWELS] = {
	// Table D.11. countertenor “a”
	{{  660.0f, 1120.0f, 2750.0f, 3000.0f, 3350.0f },
	 {    0.0f,   -6.0f,  -23.0f,  -24.0f,  -38.0f },
	 {   80.0f,   90.0f,  120.0f,  130.0f,  140.0f }},
	// Table D.12. countertenor “e”
	{{  440.0f, 1800.0f, 2700.0f, 3000.0f, 3300.0f },
	 {    0.0f,  -14.0f,  -18.0f,  -20.0f,  -20.0f },
	 {   70.0f,   80.0f,  100.0f,  120.0f,  120.0f }},
	// Table D.13. countertenor “i”
	{{  270.0f, 1850.0f, 2900.0f, 3350.0f, 3590.0f },
	 {    0.0f,  -24.0f,  -24.0f,  -36.0f,  -36.0f },
	 {   40.0f,   90.0f,  100.0f,  120.0f,  120.0f }},
	// Table D.14. countertenor “o”
	{{  430.0f,  820.0f, 2700.0f, 3000.0f, 3300.0f },
	 {    0.0f,  -10.0f,  -26.0f,  -22.0f,  -34.0f },
	 {   40.0f,   80.0f,  100.0f,  120.0f,  120.0f }},
	// Table D.15. countertenor “u”
	{{  370.0f,  630.0f, 2750.0f, 3000.0f, 3400.0f },
	 {    0.0f,  -20.0f,  -23.0f,  -30.0f,  -34.0f },
	 {   40.0f,   60.0f,  100.0f,  120.0f,  120.0f }}
};

synthv1_formant::Vtab synthv1_formant::g_soprano_vtab[NUM_VOWELS] = {
	// Table D.16. soprano “a”
	{{  800.0f, 1150.0f, 2900.0f, 3900.0f, 4950.0f },
	 {    0.0f,   -6.0f,  -32.0f,  -20.0f,  -50.0f },
	 {   80.0f,   90.0f,  120.0f,  130.0f,  140.0f }},
	// Table D.17. soprano “e”
	{{  350.0f, 2000.0f, 2800.0f, 3600.0f, 4950.0f },
	 {    0.0f,  -20.0f,  -15.0f,  -40.0f,  -56.0f },
	 {   60.0f,  100.0f,  120.0f,  150.0f,  200.0f }},
	// Table D.18. soprano “i”
	{{  270.0f, 2140.0f, 2950.0f, 3900.0f, 4950.0f },
	 {    0.0f,  -12.0f,  -26.0f,  -26.0f,  -44.0f },
	 {   60.0f,   90.0f,  100.0f,  120.0f,  120.0f }},
	// Table D.19. soprano “o”
	{{  450.0f,  800.0f, 2830.0f, 3800.0f, 4950.0f },
	 {    0.0f,  -11.0f,  -22.0f,  -22.0f,  -50.0f },
	 {   40.0f,   80.0f,  100.0f,  120.0f,  120.0f }},
	// Table D.20. soprano “u”
	{{  325.0f,  700.0f, 2700.0f, 3800.0f, 4950.0f },
	 {    0.0f,  -16.0f,  -35.0f,  -40.0f,  -60.0f },
	 {   50.0f,   60.0f,  170.0f,  180.0f,  200.0f }}
};

synthv1_formant::Vtab synthv1_formant::g_tenor_vtab[NUM_VOWELS] = {
	// Table D.21. tenor “a”
	{{  650.0f, 1080.0f, 2650.0f, 2900.0f, 3250.0f },
	 {    0.0f,   -6.0f,   -7.0f,   -8.0f,  -22.0f },
	 {   80.0f,   90.0f,  120.0f,  130.0f,  140.0f }},
	// Table D.22. tenor “e”
	{{  400.0f, 1700.0f, 2600.0f, 3200.0f, 3580.0f },
	 {    0.0f,  -14.0f,  -12.0f,  -14.0f,  -20.0f },
	 {   70.0f,   80.0f,  100.0f,  120.0f,  120.0f }},
	// Table D.23. tenor “i”
	{{  290.0f, 1870.0f, 2800.0f, 3250.0f, 3540.0f },
	 {    0.0f,  -15.0f,  -18.0f,  -20.0f,  -30.0f },
	 {   40.0f,   90.0f,  100.0f,  120.0f,  120.0f }},
	// Table D.24. tenor “o”
	{{  400.0f,  800.0f, 2600.0f, 2800.0f, 3000.0f },
	 {    0.0f,  -10.0f,  -12.0f,  -12.0f,  -26.0f },
	 {   70.0f,   80.0f,  100.0f,  130.0f,  135.0f }},
	// Table D.25. tenor “u”
	{{  350.0f,  600.0f, 2700.0f, 2900.0f, 3300.0f },
	 {    0.0f,  -20.0f,  -17.0f,  -14.0f,  -26.0f },
	 {   40.0f,   60.0f,  100.0f,  120.0f,  120.0f }}
};

// base vocal tables.
synthv1_formant::Vtab *synthv1_formant::g_vtabs[NUM_VTABS] = {
	g_bass_vtab,
	g_tenor_vtab,
	g_countertenor_vtab,
	g_soprano_vtab,
	g_alto_vtab
};


// compute coeffs. for given vocal formant table
void synthv1_formant::Impl::vtab_coeffs (
	Coeffs& coeffs, const Vtab *vtab, uint32_t i, float p )
{
	const float Fi = vtab->freq[i];
	const float Gi = vtab->gain[i];
	const float Bi = vtab->band[i] * p;

	const float Ai = ::powf(10.0f, (0.05f * Gi));
	const float Ri = ::expf(-M_PI * Bi / m_srate);

	coeffs.b2 = Ri * Ri;
	coeffs.b1 = 2.0f * Ri * ::cosf(2.0f * M_PI * Fi / m_srate);
	coeffs.a0 = Ai * (1.0f - coeffs.b1 + coeffs.b2);
}


// reset method impl.
void synthv1_formant::Impl::reset_coeffs (void)
{
	const float   fK = m_cutoff * float(NUM_VTABS);
	const uint32_t k = uint32_t(fK);
	const float   fJ = (fK - float(k)) * float(NUM_VOWELS);
	const uint32_t j = uint32_t(fJ);
	const float   dJ = (fJ - float(j)); // vowel morph fraction

	const float q = 4.0f * m_reso * m_reso + 1.0f;
	const float p = 1.0f / q;

	// vocal/vowel formant morphing
	const Vtab *vtabs1 = g_vtabs[k];
	const Vtab *vtabs2 = (k < NUM_VTABS - 1 ? g_vtabs[k + 1] : vtabs1);

	const Vtab *vtab1 = &vtabs1[j];
	const Vtab *vtab2 = &vtabs2[j];
	if (j < NUM_VOWELS - 1)
		vtab2 = &vtabs1[j + 1];
	else
	if (k < NUM_VTABS - 1)
		vtab2 = &vtabs2[0];

	Coeffs coeff2;
	for (uint32_t i = 0; i < NUM_FORMANTS; ++i) {
		Coeffs& coeff1 = m_ctabs[i];
		vtab_coeffs(coeff1, vtab1, i, p);
		vtab_coeffs(coeff2, vtab2, i, p);
		coeff1.a0 += dJ * (coeff2.a0 - coeff1.a0);
		coeff1.b1 += dJ * (coeff2.b1 - coeff1.b1);
		coeff1.b2 += dJ * (coeff2.b2 - coeff1.b2);
	}
}


// reset coeffs. method
void synthv1_formant::reset_coeffs (void)
{
	if (m_pImpl) {
		m_pImpl->update(m_cutoff, m_reso);
		for (uint32_t i = 0; i < NUM_FORMANTS; ++i)
			m_filters[i].reset_coeffs(m_pImpl->coeffs(i));
	}
}


// end of synthv1_formant.cpp
