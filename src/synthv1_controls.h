// synthv1_controls.h
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

#ifndef __synthv1_controls_h
#define __synthv1_controls_h

#include "synthv1_param.h"
#include "synthv1_sched.h"

#include <QMap>


//-------------------------------------------------------------------------
// synthv1_controls - Controller processs class.
//

class synthv1_controls
{
public:

	// ctor.
	synthv1_controls(synthv1 *pSynth);

	// dtor.
	~synthv1_controls();

	// operational mode flags.
	enum Mode { Enabled = 1, Optional = 2 };

	void enabled(bool on)
		{ if (on) m_mode |= Enabled; else m_mode &= ~Enabled; }
	bool enabled() const
		{ return m_mode & Enabled; }

	void optional(bool on)
		{ if (on) m_mode |= Optional; else m_mode &= ~Optional; }
	bool optional() const
		{ return m_mode & Optional; }

	// controller types,
	enum Type { None = 0, CC = 0x100, RPN = 0x200, NRPN = 0x300, CC14 = 0x400 };

	// controller hash key.
	struct Key
	{
		Key () : status(0), param(0) {}
		Key (const Key& key)
			: status(key.status), param(key.param) {}

		Type type() const
			{ return Type(status & 0xf00); }
		unsigned short channel() const
			{ return (status & 0x1f); }

		// hash key comparator.
		bool operator< (const Key& key) const
		{
			if (status != key.status)
				return (status < key.status);
			else
				return (param < key.param);
		}

		unsigned short status;
		unsigned short param;
	};

	// controller flags,
	enum Flag { Logarithmic = 1, Invert = 2, Hook = 4 };

	// controller data.
	struct Data
	{
		Data () : index(-1), flags(0), val(0.0f), sync(false) {}
		Data (const Data& data)
			: index(data.index), flags(data.flags),
			  val(data.val), sync(data.sync) {}

		int index;
		int flags;
		float val;
		bool sync;
	};

	typedef QMap<Key, Data> Map;

	// controller events.
	struct Event
	{
		Key key;
		unsigned short value;
	};

	// controller map methods.
	const Map& map() const { return m_map; }

	int find_control(const Key& key) const
		{ return m_map.value(key).index; }
	void add_control(const Key& key, const Data& data)
		{ m_map.insert(key, data); }
	void remove_control(const Key& key)
		{ m_map.remove(key); }

	void clear() { m_map.clear(); }

	// reset all controllers.
	void reset();

	// controller queue methods.
	void process_enqueue(
		unsigned short channel,
		unsigned short param,
		unsigned short value);

	void process_dequeue();

	// process timer counter.
	void process(unsigned int nframes);

	// text utilities.
	static Type typeFromText(const QString& sText);
	static QString textFromType(Type ctype);

	// current/last controller accessor.
	const Key& current_key() const;

protected:

	// controller action.
	void process_event(const Event& event);

	// input controller scheduled events (learn)
	class SchedIn : public synthv1_sched
	{
	public:

		// ctor.
		SchedIn (synthv1 *pSynth)
			: synthv1_sched(pSynth, Controller) {}

		void schedule_key(const Key& key)
			{ m_key = key; schedule(); }

		// process (virtual stub).
		void process(int) {}

		// current controller accessor.
		const Key& current_key() const
			{ return m_key; }

	private:

		// instance variables,.
		Key m_key;
	};

	// output controller scheduled events (assignments)
	class SchedOut : public synthv1_sched
	{
	public:

		// ctor.
		SchedOut (synthv1 *pSynth)
			: synthv1_sched(pSynth, Controls) {}

		void schedule_event(synthv1::ParamIndex index, float fValue)
		{
			instance()->setParamValue(index, fValue);

			schedule(int(index));
		}

		// process (virtual stub).
		void process(int) {}
	};

private:

	// instance variables.
	class Impl;

	Impl *m_pImpl;

	// operational mode flags.
	unsigned int m_mode;

	// controller schedulers.
	SchedIn  m_sched_in;
	SchedOut m_sched_out;

	// controllers map.
	Map m_map;

	// frame timers.
	unsigned int m_timeout;
	unsigned int m_timein;
};


#endif	// __synthv1_controls_h

// end of synthv1_controls.h
