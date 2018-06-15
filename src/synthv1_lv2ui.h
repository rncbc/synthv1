// synthv1_lv2ui.h
//
/****************************************************************************
   Copyright (C) 2012-2018, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __synthv1_lv2ui_h
#define __synthv1_lv2ui_h

#include "synthv1_ui.h"

#include "lv2.h"
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"


#define SYNTHV1_LV2UI_URI SYNTHV1_LV2_PREFIX "ui"

#ifdef CONFIG_LV2_UI_X11
#include <QWindow>
#define SYNTHV1_LV2UI_X11_URI SYNTHV1_LV2_PREFIX "ui_x11"
#endif

#ifdef CONFIG_LV2_UI_EXTERNAL
#include "lv2_external_ui.h"
#define SYNTHV1_LV2UI_EXTERNAL_URI SYNTHV1_LV2_PREFIX "ui_external"
#endif


// Forward decls.
class synthv1_lv2;


//-------------------------------------------------------------------------
// synthv1_lv2ui - decl.
//

class synthv1_lv2ui : public synthv1_ui
{
public:

	// Constructor.
	synthv1_lv2ui(synthv1_lv2 *pSynth,
		LV2UI_Controller controller, LV2UI_Write_Function write_function);

	// Accessors.
	const LV2UI_Controller& controller() const;
	void write_function(synthv1::ParamIndex index, float fValue) const;

private:

	// Instance variables.
	LV2UI_Controller     m_controller;
	LV2UI_Write_Function m_write_function;
};


#endif	// __synthv1_lv2ui_h

// end of synthv1_lv2ui.h
