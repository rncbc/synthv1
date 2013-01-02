// synthv1widget_config.h
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

#ifndef __synthv1widget_config_h
#define __synthv1widget_config_h

#include "config.h"

#define SYNTHV1_TITLE	PACKAGE_NAME
#define SYNTHV1_VERSION	PACKAGE_VERSION

#define SYNTHV1_SUBTITLE     "an old-school polyphonic synthesizer."
#define SYNTHV1_WEBSITE      "http://synthv1.sourceforge.net"
#define SYNTHV1_COPYRIGHT    "Copyright (C) 2012-2013, rncbc aka Rui Nuno Capela. All rights reserved."

#define SYNTHV1_DOMAIN	"rncbc.org"


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
