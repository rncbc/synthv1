// synthv1widget_config.h
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

#ifndef __synthv1widget_config_h
#define __synthv1widget_config_h

#include "synthv1_config.h"

#include <QSettings>
#include <QStringList>


//-------------------------------------------------------------------------
// synthv1widget_config - Prototype settings class (singleton).
//

class synthv1widget_config : public QSettings
{
public:

	// Constructor.
	synthv1widget_config();

	// Default destructor.
	~synthv1widget_config();

	// Default options...
	QString sPreset;
	QString sPresetDir;

	// Run-time special non-persistent options.
	bool bDontUseNativeDialog;

	// Singleton instance accessor.
	static synthv1widget_config *getInstance();

protected:

	// Explicit I/O methods.
	void load();
	void save();

private:

	// The singleton instance.
	static synthv1widget_config *g_pSettings;
};


#endif	// __synthv1widget_config_h

// end of synthv1widget_config.h
