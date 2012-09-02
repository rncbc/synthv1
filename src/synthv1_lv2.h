// synthv1_lv2.h
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

#ifndef __synthv1_lv2_h
#define __synthv1_lv2_h

#include "synthv1.h"

#include "lv2.h"
#include "lv2/lv2plug.in/ns/ext/event/event.h"

#define SYNTHV1_LV2_URI "https://www.rncbc.org/svn/test/synthv1"
#define SYNTHV1_LV2_PREFIX SYNTHV1_LV2_URI "#"


//-------------------------------------------------------------------------
// synthv1_lv2 - decl.
//

class synthv1_lv2 : public synthv1
{
public:

	synthv1_lv2(double sample_rate, const LV2_Feature *const *host_features);

	~synthv1_lv2();

	enum PortIndex {

		MidiIn = 0,
		AudioInL,
		AudioInR,
		AudioOutL,
		AudioOutR,
		ParamBase
	};

	void connect_port(uint32_t port, void *data);

	void run(uint32_t nframes);

	void activate();
	void deactivate();

private:

	uint32_t m_event_id;

	LV2_Event_Buffer *m_event_buffer;

	float **m_ins;
	float **m_outs;
};


#endif// __synthv1_lv2_h

// end of synthv1_lv2.h
