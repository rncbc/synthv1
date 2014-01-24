// synthv1_reverb.h
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

#ifndef __synthv1_reverb_h
#define __synthv1_reverb_h

#include <string.h>

//-------------------------------------------------------------------------
// synthv1_reverb
//
// -- borrowed, stirred and refactored from original FreeVerb --
//    by Jezar at Dreampoint, June 2000 (public domain)
//

class synthv1_reverb
{
public:

	synthv1_reverb (uint32_t iSampleRate = 44100)
		: m_srate(float(iSampleRate)),
			m_feedb(0.5f), m_room(0.5f), m_damp(0.5f)
			{ reset(false); }

	void setSampleRate(uint32_t iSampleRate)
		{ m_srate = float(iSampleRate); reset(true); }
	uint32_t sampleRate() const
		{ return uint32_t(m_srate); }

	// call this after changing roomsize or dampening
	void reset(bool bReset = false)
	{
		const float sr = m_srate / 44100.0f;

		uint32_t j;

		for (j = 0; j < NUM_ALLPASSES; ++j) {
			m_allpasses[j][0].resize(uint32_t(s_allpasses[j] * sr));
			m_allpasses[j][1].resize(uint32_t((s_allpasses[j] + STEREO_SPREAD) * sr));
			if (bReset) {
				m_allpasses[j][0].reset();
				m_allpasses[j][1].reset();
			}
		}

		for (j = 0; j < NUM_COMBS; ++j) {
			m_combs[j][0].resize(uint32_t(s_combs[j] * sr));
			m_combs[j][1].resize(uint32_t((s_combs[j] + STEREO_SPREAD) * sr));
			if (bReset) {
				m_combs[j][0].reset();
				m_combs[j][1].reset();
			}
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
				tmp0 += m_combs[j][0].output(out0);
				tmp1 += m_combs[j][1].output(out1);
			}

			for (j = 0; j < NUM_ALLPASSES; ++j) {
				tmp0 = m_allpasses[j][0].output(tmp0);
				tmp1 = m_allpasses[j][1].output(tmp1);
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

	static const uint32_t STEREO_SPREAD = 23;
	static const uint32_t NUM_COMBS     = 8;
	static const uint32_t NUM_ALLPASSES = 4;

	static const uint32_t s_combs[NUM_COMBS];
	static const uint32_t s_allpasses[NUM_ALLPASSES];

	void reset_feedb()
	{
		for (uint32_t j = 0; j < NUM_ALLPASSES; ++j) {
			m_combs[j][0].set_feedb(m_feedb);
			m_combs[j][1].set_feedb(m_feedb);
		}
	}

	void reset_room()
	{
		for (uint32_t j = 0; j < NUM_COMBS; ++j) {
			m_combs[j][0].set_feedb(m_room);
			m_combs[j][1].set_feedb(m_room);
		}
	}

	void reset_damp()
	{
		const float damp2 = m_damp * m_damp;
		for (uint32_t j = 0; j < NUM_COMBS; ++j) {
			m_combs[j][0].set_damp(damp2);
			m_combs[j][1].set_damp(damp2);
		}
	}

	class heap_buffer
	{
	public:

		heap_buffer(uint32_t size = 2048)
			: m_buffer(0), m_size(0) { resize(size); }

		float *ptr() const
			{ return m_buffer; }

		uint32_t size() const
			{ return m_size; }

		void resize(uint32_t new_size)
		{
			const uint32_t old_size = m_size;
			if (new_size > old_size) {
				float *old_buffer = m_buffer;
				m_buffer = new float [new_size];
				m_size = new_size;
				if (old_buffer) {
					::memcpy(m_buffer, old_buffer, old_size * sizeof(float));
					delete [] old_buffer;
				}
			}
		}

	private:

		float *m_buffer;
		uint32_t m_size;
	};

	class comb_filter
	{
	public:

		comb_filter (uint32_t size = 0)
			: m_feedb(0.5f), m_damp(0.5f), m_index(0), m_size(0), m_filt0(0.0f)
			{ resize(size); }

		void set_feedb(float feedb)
			{ m_feedb = feedb; }
		float feedb() const
			{ return m_feedb; }

		void set_damp(float damp)
			{ m_damp = damp; }
		float damp() const
			{ return m_damp; }

		void reset()
			{ ::memset(m_buf.ptr(), 0, m_size * sizeof(float)); m_filt0 = 0.0f; }

		void resize(uint32_t size)
		{
			if (size < 1)
				size = 1;
			if (m_size != size) {
				m_size  = size;
				m_index = 0;
				m_buf.resize(size);
				reset();
			}
		}

		float output(float in)
		{
			float *buf = m_buf.ptr() + m_index;
			float  out = *buf;
			m_filt0 = denormal((out * (1 - m_damp)) + (m_filt0 * m_damp));
			*buf = in + (m_filt0 * m_feedb);
			if (++m_index >= m_size)
				m_index = 0;
			return out;
		}

	private:

		float m_feedb;
		float m_damp;
		uint32_t m_index;
		uint32_t m_size;
		heap_buffer m_buf;
		float m_filt0;
	};

	class allpass_filter
	{
	public:

		allpass_filter(uint32_t size = 0)
			: m_feedb(0.5f), m_index(0), m_size(0)
			{ resize(size); }

		void set_feedb(float feedb)
			{ m_feedb = feedb; }
		float feedb () const
			{ return m_feedb; }

		void reset()
			{ ::memset(m_buf.ptr(), 0, m_size * sizeof(float)); }

		void resize(uint32_t size)
		{
			if (size < 1)
				size = 1;
			if (m_size != size) {
				m_size  = size;
				m_index = 0;
				m_buf.resize(size);
				reset();
			}
		}

		float output(float in)
		{
			float *buf = m_buf.ptr() + m_index;
			float  out = *buf;
			*buf = denormal(in + (out * m_feedb));
			if (++m_index >= m_size)
				m_index = 0;
			return out - in;
		}

	private:

		float m_feedb;
		uint32_t m_index;
		uint32_t m_size;
		heap_buffer m_buf;
	};

	static float denormal(float v)
	{
		union { float f; unsigned int w; } u;
		u.f = v;
		return (u.w & 0x7f800000) ? v : 0.0f;
	}

private:

	float m_srate;

	float m_feedb;
	float m_room;
	float m_damp;

	comb_filter m_combs[NUM_COMBS][2];
	allpass_filter m_allpasses[NUM_ALLPASSES][2];
};


const uint32_t synthv1_reverb::s_combs[]
	= { 1116, 1188, 1277, 1356, 1422, 1491, 1557, 1617 };

const uint32_t synthv1_reverb::s_allpasses[]
	= { 556, 441, 341, 225 };


#endif	// __synthv1_reverb_h
