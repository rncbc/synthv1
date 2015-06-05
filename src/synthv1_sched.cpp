// synthv1_sched.cpp
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

#include "synthv1_sched.h"

#include <QThread>
#include <QMutex>
#include <QWaitCondition>

#include <QHash>


//-------------------------------------------------------------------------
// synthv1_sched_thread - worker/schedule thread decl.
//

class synthv1_sched_thread : public QThread
{
public:

	// ctor.
	synthv1_sched_thread(uint32_t nsize = 8);

	// dtor.
	~synthv1_sched_thread();

	// schedule processing and wake from wait condition.
	void schedule(synthv1_sched *sched);

protected:

	// main thread executive.
	void run();

private:

	// sync queue instance reference.
	uint32_t m_nsize;
	uint32_t m_nmask;

	synthv1_sched **m_items;

	volatile uint32_t m_iread;
	volatile uint32_t m_iwrite;

	// whether the thread is logically running.
	volatile bool m_running;

	// thread synchronization objects.
	QMutex m_mutex;
	QWaitCondition m_cond;
};


static synthv1_sched_thread *g_sched_thread = NULL;
static uint32_t g_sched_refcount = 0;

static QHash<synthv1 *, QList<synthv1_sched_notifier *> > g_sched_notifiers;


//-------------------------------------------------------------------------
// synthv1_sched_thread - worker/schedule thread impl.
//

// ctor.
synthv1_sched_thread::synthv1_sched_thread ( uint32_t nsize ) : QThread()
{
	m_nsize = (4 << 1);
	while (m_nsize < nsize)
		m_nsize <<= 1;
	m_nmask = (m_nsize - 1);
	m_items = new synthv1_sched * [m_nsize];

	m_iread  = 0;
	m_iwrite = 0;

	::memset(m_items, 0, m_nsize * sizeof(synthv1_sched *));

	m_running = false;
}


// dtor.
synthv1_sched_thread::~synthv1_sched_thread (void)
{
	// fake sync and wait
	if (m_running && isRunning()) do {
		if (m_mutex.tryLock()) {
			m_running = false;
			m_cond.wakeAll();
			m_mutex.unlock();
		}
	} while (!wait(100));

	delete [] m_items;
}


// schedule processing and wake from wait condition.
void synthv1_sched_thread::schedule ( synthv1_sched *sched )
{
	if (!sched->sync_wait()) {
		const uint32_t w = (m_iwrite + 1) & m_nmask;
		if (w != m_iread) {
			m_items[m_iwrite] = sched;
			m_iwrite = w;
		}
	}

	if (m_mutex.tryLock()) {
		m_cond.wakeAll();
		m_mutex.unlock();
	}
}


// main thread executive.
void synthv1_sched_thread::run (void)
{
	m_mutex.lock();

	m_running = true;

	while (m_running) {
		// do whatever we must...
		uint32_t r = m_iread;
		while (r != m_iwrite) {
			synthv1_sched *sched = m_items[r];
			if (sched) {
				sched->sync_process();
				m_items[r] = NULL;
			}
			++r &= m_nmask;
		}
		m_iread = r;
		// wait for sync...
		m_cond.wait(&m_mutex);
	}

	m_mutex.unlock();
}


//-------------------------------------------------------------------------
// synthv1_sched - worker/scheduled stuff (pure virtual).
//

// ctor.
synthv1_sched::synthv1_sched ( synthv1 *pSynth, Type stype, uint32_t nsize )
	: m_pSynth(pSynth), m_stype(stype), m_sync_wait(false)
{
	m_nsize = (4 << 1);
	while (m_nsize < nsize)
		m_nsize <<= 1;
	m_nmask = (m_nsize - 1);
	m_items = new int [m_nsize];

	m_iread  = 0;
	m_iwrite = 0;

	::memset(m_items, 0, m_nsize * sizeof(int));

	if (++g_sched_refcount == 1 && g_sched_thread == NULL) {
		g_sched_thread = new synthv1_sched_thread();
		g_sched_thread->start();
	}
}


// dtor (virtual).
synthv1_sched::~synthv1_sched (void)
{
	delete [] m_items;

	if (--g_sched_refcount == 0) {
		if (g_sched_thread) {
			delete g_sched_thread;
			g_sched_thread = NULL;
		}
	}
}


// instance access.
synthv1 *synthv1_sched::instance (void) const
{
	return m_pSynth;
}


// schedule process.
void synthv1_sched::schedule ( int sid )
{
	const uint32_t w = (m_iwrite + 1) & m_nmask;
	if (w != m_iread) {
		m_items[m_iwrite] = sid;
		m_iwrite = w;
	}

	if (g_sched_thread)
		g_sched_thread->schedule(this);
}


// test-and-set.
bool synthv1_sched::sync_wait (void)
{
	const bool sync_wait = m_sync_wait;

	if (!sync_wait)
		m_sync_wait = true;

	return sync_wait;
}


// scheduled processor.
void synthv1_sched::sync_process (void)
{
	// do whatever we must...
	uint32_t r = m_iread;
	while (r != m_iwrite) {
		const int sid = m_items[r];
		process(sid);
		sync_notify(m_pSynth, m_stype, sid);
		m_items[r] = 0;
		++r &= m_nmask;
	}
	m_iread = r;

	m_sync_wait = false;
}


// signal broadcast (static).
void synthv1_sched::sync_notify ( synthv1 *pSynth, Type stype, int sid )
{
	if (g_sched_notifiers.contains(pSynth)) {
		const QList<synthv1_sched_notifier *>& list
			= g_sched_notifiers.value(pSynth);
		QListIterator<synthv1_sched_notifier *> iter(list);
		while (iter.hasNext())
			iter.next()->notify(stype, sid);
	}
}


//-------------------------------------------------------------------------
// synthv1_sched_notifier - worker/schedule proxy decl.
//

// ctor.
synthv1_sched_notifier::synthv1_sched_notifier ( synthv1 *pSynth )
	: m_pSynth(pSynth)
{
	g_sched_notifiers[m_pSynth].append(this);
}


// dtor.
synthv1_sched_notifier::~synthv1_sched_notifier (void)
{
	if (g_sched_notifiers.contains(m_pSynth)) {
		QList<synthv1_sched_notifier *>& list = g_sched_notifiers[m_pSynth];
		list.removeAll(this);
		if (list.isEmpty())
			g_sched_notifiers.remove(m_pSynth);
	}
}


// end of synthv1_sched.cpp
