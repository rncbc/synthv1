// synthv1widget_lv2.cpp
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

#include "synthv1widget_lv2.h"

#include "synthv1_lv2.h"


//-------------------------------------------------------------------------
// synthv1widget_lv2 - impl.
//

synthv1widget_lv2::synthv1widget_lv2 (
	LV2UI_Controller controller, LV2UI_Write_Function write_function )
	: synthv1widget()
{
	m_controller = controller;
	m_write_function = write_function;

	clearPreset();
}


void synthv1widget_lv2::port_event ( uint32_t port_index,
	uint32_t buffer_size, uint32_t format, const void *buffer )
{
	if (format == 0 && buffer_size == sizeof(float)) {
		float fValue = *(float *) buffer;
		setParamValue(synthv1::ParamIndex(
			port_index - synthv1_lv2::ParamBase), fValue);
	}
}


// Param method.
void synthv1widget_lv2::updateParam (
	synthv1::ParamIndex index, float fValue ) const
{
	m_write_function(m_controller,
		synthv1_lv2::ParamBase + index, sizeof(float), 0, &fValue);
}


// end of synthv1widget_lv2.cpp
