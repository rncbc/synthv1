// synthv1widget_jack.h
//
/****************************************************************************
   Copyright (C) 2012-2024, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __synthv1widget_jack_h
#define __synthv1widget_jack_h

#include "synthv1widget.h"


// Forward decls.
class synthv1_jack;

#ifdef CONFIG_NSM
class synthv1_nsm;
#endif


//-------------------------------------------------------------------------
// synthv1widget_jack - decl.
//

class synthv1widget_jack : public synthv1widget
{
public:

	// Constructor.
	synthv1widget_jack(synthv1_jack *pSynth);

	// Destructor.
	~synthv1widget_jack();

#ifdef CONFIG_NSM
	// NSM client accessors.
	void setNsmClient(synthv1_nsm *pNsmClient);
	synthv1_nsm *nsmClient() const;
#endif	// CONFIG_NSM

	// Dirty flag method.
	void updateDirtyPreset(bool bDirtyPreset);

protected:

	// Synth engine accessor.
	synthv1_ui *ui_instance() const;

	// Param port method.
	void updateParam(synthv1::ParamIndex index, float fValue) const;

	// Application close.
	void closeEvent(QCloseEvent *pCloseEvent);

#ifdef CONFIG_NSM
	// Optional GUI handlers.
	void showEvent(QShowEvent *pShowEvent);
	void hideEvent(QHideEvent *pHideEvent);
#endif	// CONFIG_NSM

private:

	// Instance variables.
	synthv1     *m_pSynth;
	synthv1_ui  *m_pSynthUi;

#ifdef CONFIG_NSM
	synthv1_nsm *m_pNsmClient;
#endif
};


#endif	// __synthv1widget_jack_h

// end of synthv1widget_jack.h
