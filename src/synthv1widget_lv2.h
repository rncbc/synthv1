// synthv1widget_lv2.h
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

#ifndef __synthv1widget_lv2_h
#define __synthv1widget_lv2_h

#include "synthv1widget.h"

#include "lv2.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"


#define SYNTHV1_LV2UI_URI SYNTHV1_LV2_PREFIX "ui"


// Forward decls.
class synthv1_lv2;


#ifdef CONFIG_LV2_EXTERNAL_UI
#include "lv2_external_ui.h"
#define SYNTHV1_LV2UI_EXTERNAL_URI SYNTHV1_LV2_PREFIX "ui_external"
#endif


//-------------------------------------------------------------------------
// synthv1widget_lv2 - decl.
//

class synthv1widget_lv2 : public synthv1widget
{
public:

	synthv1widget_lv2(synthv1_lv2 *pSynth,
		LV2UI_Controller controller, LV2UI_Write_Function write_function);

	void port_event(uint32_t port_index,
		uint32_t buffer_size, uint32_t format, const void *buffer);

#ifdef CONFIG_LV2_EXTERNAL_UI
	void setExternalHost(LV2_External_UI_Host *external_host);
	const LV2_External_UI_Host *externalHost() const;
#endif

protected:

	// Synth engine accessor.
	synthv1 *instance() const;

	// Param methods.
	void updateParam(synthv1::ParamIndex index, float fValue) const;

#ifdef CONFIG_LV2_EXTERNAL_UI
	void closeEvent(QCloseEvent *pCloseEvent);
#endif

private:

	// Instance variables.
	synthv1_lv2 *m_pSynth;

	// Instance variables.
	LV2UI_Controller     m_controller;
	LV2UI_Write_Function m_write_function;

	bool m_params_def[synthv1::NUM_PARAMS];

#ifdef CONFIG_LV2_EXTERNAL_UI
	LV2_External_UI_Host *m_external_host;
#endif
};


#endif	// __synthv1widget_lv2_h

// end of synthv1widget_lv2.h
