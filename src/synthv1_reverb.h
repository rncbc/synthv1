// synthv1_reverb.h
//
/****************************************************************************
   Copyright (C) 2012-2021, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __synthv1_reverb_h
#define __synthv1_reverb_h

#include <cstdint>
#include <cstring>


//-------------------------------------------------------------------------
// synthv1_reverb
//
// -- borrowed, stirred and refactored from original FreeVerb --
//    by Jezar at Dreampoint, June 2000 (public domain)
//

class synthv1_reverb
{
public:

	synthv1_reverb (float srate = 44100.0f)
		: m_srate(srate), m_room(0.5f), m_damp(0.5f), m_feedb(0.5f)
			{ reset(); }

	void setSampleRate(float srate)
		{ m_srate = srate; }
	float sampleRate() const
		{ return m_srate; }

	void reset()
	{
		static const uint32_t s_comb[NUM_COMBS]
			= { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617, 1685, 1748 };
		static const uint32_t s_allpass[NUM_ALLPASSES]
			= { 556, 441, 341, 225, 180, 153 };

		const float sr = m_srate / 44100.0f;

		uint32_t j;

		for (j = 0; j < NUM_ALLPASSES; ++j) {
			m_allpass0[j].resize(uint32_t(s_allpass[j] * sr));
			m_allpass0[j].reset();
			m_allpass1[j].resize(uint32_t((s_allpass[j] + STEREO_SPREAD) * sr));
			m_allpass1[j].reset();
		}

		for (j = 0; j < NUM_COMBS; ++j) {
			m_comb0[j].resize(uint32_t(s_comb[j] * sr));
			m_comb0[j].reset();
			m_comb1[j].resize(uint32_t((s_comb[j] + STEREO_SPREAD) * sr));
			m_comb1[j].reset();
		}

		reset_feedb();
		reset_room();
		reset_damp();
	}

	void process(float *in0, float *in1, uint32_t nframes,
		float wet, float feedb, float room, float damp, float width)
	{
		if (wet < 1E-9f)
			return;

		if (m_feedb != feedb) {
			m_feedb  = feedb;
			reset_feedb();
		}

		if (m_room != room) {
			m_room  = room;
			reset_room();
		}

		if (m_damp != damp) {
			m_damp  = damp;
			reset_damp();
		}

		uint32_t i, j;

		for (i = 0; i < nframes; ++i) {

			float out0 = *in0 * 0.05f; // 0.015f;
			float out1 = *in1 * 0.05f; // 0.015f;

			float tmp0 = 0.0f;
			float tmp1 = 0.0f;

			for (j = 0; j < NUM_COMBS; ++j) {
				tmp0 += m_comb0[j].output(out0);
				tmp1 += m_comb1[j].output(out1);
			}

			for (j = 0; j < NUM_ALLPASSES; ++j) {
				tmp0 = m_allpass0[j].output(tmp0);
				tmp1 = m_allpass1[j].output(tmp1);
			}

			if (width < 0.0f) {
				out0 = tmp0 * (1.0f + width) - tmp1 * width;
				out1 = tmp1 * (1.0f + width) - tmp0 * width;
			} else {
				out0 = tmp0 * width + tmp1 * (1.0f - width);
				out1 = tmp1 * width + tmp0 * (1.0f - width);
			}

			*in0++ += wet * out0;
			*in1++ += wet * out1;
		}
	}

protected:

	static const uint32_t NUM_COMBS     = 10;
	static const uint32_t NUM_ALLPASSES = 6;
	static const uint32_t STEREO_SPREAD = 23;

	void reset_room()
	{
		for (uint32_t j = 0; j < NUM_COMBS; ++j) {
			m_comb0[j].set_feedb(m_room);
			m_comb1[j].set_feedb(m_room);
		}
	}

	void reset_damp()
	{
		const float damp2 = m_damp * m_damp;
		for (uint32_t j = 0; j < NUM_COMBS; ++j) {
			m_comb0[j].set_damp(damp2);
			m_comb1[j].set_damp(damp2);
		}
	}

	void reset_feedb()
	{
		const float feedb2 = 2.0f * m_feedb * (2.0f - m_feedb) / 3.0f;
		for (uint32_t j = 0; j < NUM_ALLPASSES; ++j) {
			m_allpass0[j].set_feedb(feedb2);
			m_allpass1[j].set_feedb(feedb2);
		}
	}

	class sample_buffer
	{
	public:

		sample_buffer (uint32_t size = 0)
			: m_buffer(0), m_size(0), m_index(0)
			{ resize(size); }

		virtual ~sample_buffer()
			{ delete [] m_buffer; }

		void reset()
			{ ::memset(m_buffer, 0, m_size * sizeof(float)); m_index = 0; }

		void resize(uint32_t size)
		{
			if (size < 1)
				size = 1;
			if (m_size != size) {
				const uint32_t old_size = m_size;
				if (size > old_size) {
					float *old_buffer = m_buffer;
					m_buffer = new float [size];
					m_size = size;
					if (old_buffer) {
						::memcpy(m_buffer, old_buffer,
							old_size * sizeof(float));
						delete [] old_buffer;
					}
				}
			}
		}

		float *tick()
		{
			float *buf = m_buffer + m_index;
			if (++m_index >= m_size)
				m_index = 0;
			return buf;
		}

	private:

		float   *m_buffer;
		uint32_t m_size;
		uint32_t m_index;
	};

	class comb_filter : public sample_buffer
	{
	public:

		comb_filter (uint32_t size = 0)
			: sample_buffer(size), m_feedb(0.5f), m_damp(0.5f), m_out(0.0f) {}

		void set_feedb(float feedb)
			{ m_feedb = feedb; }
		float feedb() const
			{ return m_feedb; }

		void set_damp(float damp)
			{ m_damp = damp; }
		float damp() const
			{ return m_damp; }

		void reset()
			{ sample_buffer::reset(); m_out = 0.0f; }

		float output(float in)
		{
			float *buf = tick();
			float  out = *buf;
			m_out = denormal(out * (1.0f - m_damp) + m_out * m_damp);
			*buf = in + (m_out * m_feedb);
			return out;
		}

	private:

		float m_feedb;
		float m_damp;
		float m_out;
	};

	class allpass_filter : public sample_buffer
	{
	public:

		allpass_filter(uint32_t size = 0)
			: sample_buffer(size), m_feedb(0.5f) {}

		void set_feedb(float feedb)
			{ m_feedb = feedb; }
		float feedb () const
			{ return m_feedb; }

		float output(float in)
		{
			float *buf = tick();
			float  out = *buf;
			*buf = denormal(in + out * m_feedb);
			return out - in;
		}

	private:

		float m_feedb;
	};

	static float denormal(float v)
	{
		union { float f; uint32_t w; } u;
		u.f = v;
		return (u.w & 0x7f800000) ? v : 0.0f;
	}

private:

	float m_srate;

	float m_room;
	float m_damp;
	float m_feedb;

	comb_filter m_comb0[NUM_COMBS];
	comb_filter m_comb1[NUM_COMBS];

	allpass_filter m_allpass0[NUM_ALLPASSES];
	allpass_filter m_allpass1[NUM_ALLPASSES];
};


#endif	// __synthv1_reverb_h
