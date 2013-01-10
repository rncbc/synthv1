// synthv1widget_lv2.h
//
/****************************************************************************
   Copyright (C) 2012-2013, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __synthv1widget_lv2_h
#define __synthv1widget_lv2_h

#include "synthv1widget.h"

#include "lv2.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"


#define SYNTHV1_LV2UI_URI SYNTHV1_LV2_PREFIX "ui"


//-------------------------------------------------------------------------
// synthv1widget_lv2 - decl.
//

class synthv1widget_lv2 : public synthv1widget
{
public:

	synthv1widget_lv2(
		LV2UI_Controller controller, LV2UI_Write_Function write_function);

	void port_event(uint32_t port_index,
		uint32_t buffer_size, uint32_t format, const void *buffer);

protected:

	// Param methods.
	void updateParam(synthv1::ParamIndex index, float fValue) const;

private:

	// Instance variables.
	LV2UI_Controller     m_controller;
	LV2UI_Write_Function m_write_function;
};


#endif	// __synthv1widget_lv2_h

// end of synthv1widget_lv2.h
