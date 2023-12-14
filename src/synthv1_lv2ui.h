// synthv1_lv2ui.h
//
/****************************************************************************
   Copyright (C) 2012-2021, rncbc aka Rui Nuno Capela. All rights reserved.

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

#if __has_include (<lv2/core/lv2.h>)
// new versions of LV2 use different location for headers
#include "lv2/ui/ui.h"
#else
#include "lv2/lv2plug.in/ns/extensions/ui/ui.h"
#endif


#define SYNTHV1_LV2UI_URI SYNTHV1_LV2_PREFIX "ui"

#if defined(CONFIG_LV2_UI_X11) || defined(CONFIG_LV2_UI_WINDOWS)
#include <QWindow>
#endif

#ifdef CONFIG_LV2_UI_X11
#define SYNTHV1_LV2UI_X11_URI SYNTHV1_LV2_PREFIX "ui_x11"
#endif

#ifdef CONFIG_LV2_UI_WINDOWS
#include <windows.h>
#define SYNTHV1_LV2UI_WINDOWS_URI SYNTHV1_LV2_PREFIX "ui_windows"

// Polyfill for windows size (minimal suitable size)
// Qt cannot determine the right window size on Windows.
#define UI_WINDOWS_RECOMMENDED_WIDTH 1380
#define UI_WINDOWS_RECOMMENDED_HEIGHT 650
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
