// synthv1_list.h
//
/****************************************************************************
   Copyright (C) 2012, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __synthv1_list_h
#define __synthv1_list_h


//-------------------------------------------------------------------------
// synthv1_list - generic double-linked list node.

template<typename T>
class synthv1_list
{
public:

	synthv1_list() : m_prev(0), m_next(0) {}

	void append(T *p)
	{
		p->m_prev = m_prev;
		p->m_next = 0;

		if (m_prev)
			m_prev->m_next = p;
		else
			m_next = p;

		m_prev = p;
	}

	void remove(T *p)
	{
		if (p->m_prev)
			p->m_prev->m_next = p->m_next;
		else
			m_next = p->m_next;

		if (p->m_next)
			p->m_next->m_prev = p->m_prev;
		else
			m_prev = p->m_prev;
	}

	T *prev() const { return m_prev; }
	T *next() const { return m_next; }

private:

	T *m_prev;
	T *m_next;
};


#endif	// __synthv1_list_h

// end of synthv1_list.h
