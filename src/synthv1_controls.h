// synthv1_controls.h
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

#ifndef __synthv1_controls_h
#define __synthv1_controls_h

#include "synthv1_param.h"

#include <QHash>


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

	// controller types,
	enum Type { None = 0, CC = 0xb0, RPN = 0x10, NRPN = 0x20, CC14 = 0x30 };

	// controller events.
	struct Event
	{
		unsigned char  status;
		unsigned short param;
		unsigned short value;
	};

	// controller hash key.
	struct Key
	{
		Key () : status(0), param(0) {}
		Key (const Event& event)
			: status(event.status), param(event.param) {}

		// hash key comparator.
		bool operator== (const Key& key) const
			{ return (key.status == status) && (key.param == param); }

		unsigned char  status;
		unsigned short param;
	};

	typedef QHash<Key, int> Map;

	// controller map methods.
	const Map& map() const { return m_map; }

	int find_control(const Key& key) const
		{ return m_map.value(key, -1); }
	void add_control(const Key& key, int index)
		{ m_map.insert(key, index); }
	void remove__control(const Key& key)
		{ m_map.remove(key); }

	void clear() { m_map.clear(); }

	// controller queue methods.
	void process_enqueue(
		unsigned short channel,
		unsigned short param,
		unsigned short value);

	void process_dequeue();

	void flush();

	// text utilities.
	static Type typeFromText(const QString& sText);
	static QString textFromType(Type ctype);

protected:

	void process_event(const Event& event);

private:

	// instance variables.
	class Impl;

	Impl *m_pImpl;

	synthv1 *m_pSynth;

	// Controllers map.
	Map m_map;
};


// hash key function.
inline uint qHash ( const synthv1_controls::Key& key )
{
	return qHash(key.status ^ key.param);
}


#endif	// __synthv1_controls_h

// end of synthv1_controls.h
