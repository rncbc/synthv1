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
// synthv1_wave_thread - local module thread stuff.
//

#include <pthread.h>
#include <string.h>


class synthv1_wave_thread
{
public:

	// ctor.
	synthv1_wave_thread ( uint32_t nsize = 8 )
	{
		m_nsize = (4 << 1);
		while (m_nsize < nsize)
			m_nsize <<= 1;
		m_nmask = (m_nmask - 1);
		m_items = new Item [m_nsize];

		m_iread  = 0;
		m_iwrite = 0;

		::memset(m_items, 0, m_nsize * sizeof(Item));

		m_running = false;

		pthread_mutex_init(&m_mutex, NULL);
		pthread_cond_init(&m_cond, NULL);
	}

	// dtor.
	~synthv1_wave_thread ()
	{
		// fake sync and wait 
		if (m_running) {
			pthread_mutex_lock(&m_mutex);
			m_running = false;
			pthread_cond_signal(&m_cond);
			pthread_mutex_unlock(&m_mutex);
			pthread_join(m_thread, NULL);
		}

		pthread_cond_destroy(&m_cond);
		pthread_mutex_destroy(&m_mutex);

		delete [] m_items;
	}

	// real thread start executive.
	void start ()
	{
		pthread_create(&m_thread, NULL, synthv1_wave_thread::run, this);
	}

	// wake from executive wait condition.
	void reset_sync (
		synthv1_wave *wave, synthv1_wave::Shape shape, float width )
	{
		if (!wave->reset_sync_wait()) {
			const uint32_t w = (m_iwrite + 1) & m_nmask;
			if (w != m_iread) {
				Item& item = m_items[m_iwrite];
				item.wave  = wave;
				item.shape = shape;
				item.width = width;
				m_iwrite = w;
			}
		}

		if (pthread_mutex_trylock(&m_mutex) == 0) {
			pthread_cond_signal(&m_cond);
			pthread_mutex_unlock(&m_mutex);
		}
	}

protected:

	// main thread executive.
	void *run ()
	{
		pthread_mutex_lock(&m_mutex);

		m_running = true;

		while (m_running) {
			// do whatever we must...
			uint32_t r = m_iread;
			while (r != m_iwrite) {
				Item& item = m_items[r];
				if (item.wave) {
					item.wave->reset_sync(item.shape, item.width);
					item.wave = NULL;
				}
				++r &= m_nmask;
			}
			m_iread = r;
			// wait for sync...
			pthread_cond_wait(&m_cond, &m_mutex);
		}

		pthread_mutex_unlock(&m_mutex);
		return NULL;
	}

	static void *run ( void *arg )
	{
		return static_cast<synthv1_wave_thread *> (arg)->run();
	}

private:

	// sync queue instance reference.
	uint32_t m_nsize;
	uint32_t m_nmask;

	// sync ref.item.
	struct Item
	{
		synthv1_wave *wave;
		synthv1_wave::Shape shape;
		float width;
	};

	Item *m_items;

	volatile uint32_t m_iread;
	volatile uint32_t m_iwrite;

	// whether the thread is logically running.
	volatile bool   m_running;

	// thread synchronization objects.
	pthread_t       m_thread;
	pthread_mutex_t m_mutex;
	pthread_cond_t  m_cond;
};


static synthv1_wave_thread *g_sync_thread   = NULL;
static uint32_t             g_sync_refcount = 0;


//-------------------------------------------------------------------------
// synthv1_wave - smoothed (integrating oversampled) wave table.
//

// ctor.
synthv1_wave::synthv1_wave ( uint32_t nsize, uint16_t nover, uint16_t ntabs )
	: m_nsize(nsize), m_nover(nover), m_ntabs(ntabs),
		m_shape(Saw), m_width(1.0f), m_srate(44100.0f),
		m_min_freq(0.0f), m_max_freq(0.0f), m_ftab(0.0f), m_itab(0),
		m_sync_thread(NULL), m_sync_wait(false)
{
	const uint16_t ntabs1 = m_ntabs + 1;

	m_tables = new float * [ntabs1];
	for (uint16_t itab = 0; itab < ntabs1; ++itab)
		m_tables[itab] = new float [m_nsize + 4];

	if (ntabs > 0) {
		if (++g_sync_refcount == 1 && g_sync_thread == NULL) {
			g_sync_thread = new synthv1_wave_thread();
			g_sync_thread->start();
		}
		m_sync_thread = g_sync_thread;
	}

	reset(m_shape, m_width);
}


// dtor.
synthv1_wave::~synthv1_wave (void)
{
	if (m_sync_thread) {
		if (--g_sync_refcount == 0 && g_sync_thread) {
			delete g_sync_thread;
			g_sync_thread = NULL;
		}
	}

	const uint16_t ntabs1 = m_ntabs + 1;

	for (uint16_t itab = 0; itab < ntabs1; ++itab)
		delete [] m_tables[itab];

	delete [] m_tables;
}


// init.
void synthv1_wave::reset ( Shape shape, float width )
{
	if (m_sync_thread)
		m_sync_thread->reset_sync(this, shape, width);
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

	m_sync_wait = false;
}


bool synthv1_wave::reset_sync_wait (void)
{
	const bool sync_wait = m_sync_wait;
	if (!sync_wait)
		m_sync_wait = true;
	return sync_wait;
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
	const float p0 = float(m_nsize);
	const float w0 = p0 * m_width;
	const float w2 = w0 * 0.5f;

	float *frames = m_tables[m_ntabs];

	for (uint32_t i = 0; i < m_nsize; ++i) {
		float p = float(i);
		if (p < w2)
			p = ::sinf(2.0f * M_PI * p / w0);
		else
			p = ::sinf(M_PI * (p + (p0 - w0)) / (p0 - w2));
		frames[i] = p;
	}

	if (m_width < 1.0f) {
		reset_filter(m_ntabs);
		reset_normalize(m_ntabs);
	}
	reset_interp(m_ntabs);

	m_max_freq = (0.5f * m_srate);
	m_min_freq = m_max_freq;
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
