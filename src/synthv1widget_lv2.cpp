// synthv1widget_lv2.cpp
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

#include "synthv1widget_lv2.h"

#include "synthv1_lv2.h"


#include <QCloseEvent>


//-------------------------------------------------------------------------
// synthv1widget_lv2 - impl.
//

synthv1widget_lv2::synthv1widget_lv2 ( synthv1_lv2 *pSynth,
	LV2UI_Controller controller, LV2UI_Write_Function write_function )
	: synthv1widget()
{
	m_pSynthUi = new synthv1_lv2ui(pSynth, controller, write_function);

#ifdef CONFIG_LV2_EXTERNAL_UI
	m_external_host = NULL;
#endif
#ifdef CONFIG_LV2_UI_IDLE
	m_bIdleClosed = false;
#endif

	for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i)
		m_params_def[i] = true;

	// May initialize the scheduler/work notifier.
	initSchedNotifier();

	clearPreset();
}


// Destructor.
synthv1widget_lv2::~synthv1widget_lv2 (void)
{
	delete m_pSynthUi;
}


// Synth engine accessor.
synthv1_ui *synthv1widget_lv2::ui_instance (void) const
{
	return m_pSynthUi;
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

#endif	// CONFIG_LV2_EXTERNAL_UI


#ifdef CONFIG_LV2_UI_IDLE

bool synthv1widget_lv2::isIdleClosed (void) const
{
	return m_bIdleClosed;
}

#endif	// CONFIG_LV2_UI_IDLE


// Close event handler.
void synthv1widget_lv2::closeEvent ( QCloseEvent *pCloseEvent )
{
	synthv1widget::closeEvent(pCloseEvent);

#ifdef CONFIG_LV2_UI_IDLE
	if (pCloseEvent->isAccepted())
		m_bIdleClosed = true;
#endif
#ifdef CONFIG_LV2_EXTERNAL_UI
	if (m_external_host && m_external_host->ui_closed) {
		if (pCloseEvent->isAccepted())
			m_external_host->ui_closed(m_pSynthUi->controller());
	}
#endif
}


// LV2 port event dispatcher.
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
		setParamValue(index, fValue, m_params_def[index]);
		m_params_def[index] = false;
	}
}


// Param method.
void synthv1widget_lv2::updateParam (
	synthv1::ParamIndex index, float fValue ) const
{
	m_pSynthUi->write_function(index, fValue);
}


// end of synthv1widget_lv2.cpp
