// synthv1widget_lv2.cpp
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

#include "synthv1widget_lv2.h"

#include "synthv1_lv2.h"


#ifdef CONFIG_LV2_EXTERNAL_UI
#include <QCloseEvent>
#endif


//-------------------------------------------------------------------------
// synthv1widget_lv2 - impl.
//

synthv1widget_lv2::synthv1widget_lv2 (
	LV2UI_Controller controller, LV2UI_Write_Function write_function )
	: synthv1widget()
{
	m_controller = controller;
	m_write_function = write_function;

#ifdef CONFIG_LV2_EXTERNAL_UI
	m_external_host = NULL;
#endif
	
	clearPreset();
}


#ifdef CONFIG_LV2_EXTERNAL_UI

void synthv1widget_lv2::setExternalHost ( LV2_External_UI_Host *external_host )
{
	m_external_host = external_host;

	if (m_external_host && m_external_host->plugin_human_id)
		synthv1widget::setWindowTitle(m_external_host->plugin_human_id);
}

const LV2_External_UI_Host *synthv1widget_lv2::externalHost (void) const
{
	return m_external_host;
}

void synthv1widget_lv2::closeEvent ( QCloseEvent *pCloseEvent )
{
	synthv1widget::closeEvent(pCloseEvent);

	if (m_external_host && m_external_host->ui_closed) {
		if (pCloseEvent->isAccepted())
			m_external_host->ui_closed(m_controller);
	}
}

#endif	// CONFIG_LV2_EXTERNAL_UI



void synthv1widget_lv2::port_event ( uint32_t port_index,
	uint32_t buffer_size, uint32_t format, const void *buffer )
{
	if (format == 0 && buffer_size == sizeof(float)) {
		synthv1::ParamIndex index
			= synthv1::ParamIndex(port_index - synthv1_lv2::ParamBase);
		float fValue = *(float *) buffer;
	//--legacy support < 0.3.0.4 -- begin
		if (index == synthv1::DEL1_BPM && fValue < 3.6f)
			fValue *= 100.0f;
	//--legacy support < 0.3.0.4 -- end.
		setParamValue(index, fValue);
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
