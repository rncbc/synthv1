// synthv1_ramp.h
//
/****************************************************************************
   Copyright (C) 2012-2019, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __synthv1_ramp_h
#define __synthv1_ramp_h

#include <stdint.h>
#include <math.h>


//-------------------------------------------------------------------------
// synthv1_ramp - ramp/smooth parameter changes

class synthv1_ramp
{
public:

	synthv1_ramp(uint16_t nvalues = 1)
	{
		m_nvalues = nvalues;
		m_value0 = new float [m_nvalues];
		m_value1 = new float [m_nvalues];
		m_delta  = new float [m_nvalues];

		for (uint16_t i = 0; i < m_nvalues; ++i)
			m_value0[i] = m_value1[i] = m_delta[i] = 0.0f;

		m_frames = 0;
	}

	virtual ~synthv1_ramp()
	{
		delete [] m_delta;
		delete [] m_value1;
		delete [] m_value0;
	}

	void reset()
	{
		for (uint16_t i = 0; i < m_nvalues; ++i) {
			m_value0[i] = m_value1[i];
			m_value1[i] = evaluate(i);
		}
	}

	void process(uint32_t nframes)
	{
		if (m_frames > 0) {
			if (nframes > m_frames)
				nframes = m_frames;
			for (uint16_t i = 0; i < m_nvalues; ++i)
				m_value0[i] += float(nframes) * m_delta[i];
			m_frames -= nframes;
		}
		else
		if (probe()) {
			reset();
			m_frames = nframes;
			const uint32_t MIN_FRAMES = 32;
			if (m_frames < MIN_FRAMES)
				m_frames = MIN_FRAMES;
			for (uint16_t i = 0; i < m_nvalues; ++i)
				m_delta[i] = (m_value1[i] - m_value0[i]) / float(m_frames);
		}
	}

	float value(uint32_t n, uint16_t i = 0) const
	{
		return (n < m_frames ? (m_value0[i] + float(n) * m_delta[i]) : m_value1[i]);
	}

protected:

	virtual bool probe() const = 0;
	virtual float evaluate(uint16_t i) = 0;

private:

	uint16_t m_nvalues;

	float   *m_value1;
	float   *m_value0;
	float   *m_delta;

	uint32_t m_frames;
};


//-------------------------------------------------------------------------
// synthv1_ramp1 (1 port tracking)

class synthv1_ramp1 : public synthv1_ramp
{
public:

	synthv1_ramp1(uint16_t nvalues = 1)
		: synthv1_ramp(nvalues), m_param1(0), m_param1_v(0.0f) {}

	void reset(float *param1)
	{
		m_param1 = param1;
		m_param1_v = 0.0f;

		synthv1_ramp::reset();
	}

protected:

	virtual bool probe() const
	{
		return m_param1 && ::fabsf(*m_param1 - m_param1_v) > 0.001f;
	}

	virtual float evaluate(uint16_t)
	{
		update();

		return m_param1_v;
	}

	void update()
	{
		if (m_param1)
			m_param1_v = *m_param1;
	}

	float *m_param1;
	float  m_param1_v;
};


//-------------------------------------------------------------------------
// synthv1_ramp2 (2 port tracking)

class synthv1_ramp2 : public synthv1_ramp1
{
public:

	synthv1_ramp2(uint16_t nvalues = 1)
		: synthv1_ramp1(nvalues), m_param2(0), m_param2_v(0.0f) {}

	void reset(float *param1, float *param2)
	{
		m_param2 = param2;
		m_param2_v = 0.0f;

		synthv1_ramp1::reset(param1);
	}

protected:

	virtual bool probe() const
	{
		return synthv1_ramp1::probe()
			|| (m_param2 && ::fabsf(*m_param2 - m_param2_v) > 0.001f);
	}

	virtual float evaluate(uint16_t i)
	{
		update();

		return synthv1_ramp1::evaluate(i) * m_param2_v;
	}

	void update()
	{
		synthv1_ramp1::update();

		if (m_param2)
			m_param2_v = *m_param2;
	}

	float *m_param2;
	float  m_param2_v;
};


//-------------------------------------------------------------------------
// synthv1_ramp3 (3 port tracking)

class synthv1_ramp3 : public synthv1_ramp2
{
public:

	synthv1_ramp3(uint16_t nvalues = 1)
		: synthv1_ramp2(nvalues), m_param3(0), m_param3_v(0.0f) {}

	void reset(float *param1, float *param2, float *param3)
	{
		m_param3 = param3;
		m_param3_v = 0.0f;

		synthv1_ramp2::reset(param1, param2);
	}

protected:

	virtual bool probe() const
	{
		return synthv1_ramp2::probe()
			|| (m_param3 && ::fabsf(*m_param3 - m_param3_v) > 0.001f);
	}

	virtual float evaluate(uint16_t i)
	{
		update();

		return synthv1_ramp2::evaluate(i) * m_param3_v;
	}

	void update()
	{
		synthv1_ramp2::update();

		if (m_param3)
			m_param3_v = *m_param3;
	}

	float *m_param3;
	float  m_param3_v;
};




//-------------------------------------------------------------------------
// synthv1_ramp4 (4 port tracking)

class synthv1_ramp4 : public synthv1_ramp3
{
public:

	synthv1_ramp4(uint16_t nvalues = 1)
		: synthv1_ramp3(nvalues), m_param4(0), m_param4_v(0.0f) {}

	void reset(float *param1, float *param2, float *param3, float *param4)
	{
		m_param4 = param4;
		m_param4_v = 0.0f;

		synthv1_ramp3::reset(param1, param2, param3);
	}

protected:

	virtual bool probe() const
	{
		return synthv1_ramp3::probe()
			|| (m_param4 && ::fabsf(*m_param4 - m_param4_v) > 0.001f);
	}

	virtual float evaluate(uint16_t i)
	{
		update();

		return synthv1_ramp3::evaluate(i) * m_param4_v;
	}

	void update()
	{
		synthv1_ramp3::update();

		if (m_param4)
			m_param4_v = *m_param4;
	}

	float *m_param4;
	float  m_param4_v;
};


#endif	// __synthv1_ramp_h

// end of synthv1_ramp.h
