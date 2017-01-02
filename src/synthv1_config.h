// synthv1_config.h
//
/****************************************************************************
   Copyright (C) 2012-2017, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __synthv1_config_h
#define __synthv1_config_h

#include "config.h"

#define SYNTHV1_TITLE       PACKAGE_NAME

#define SYNTHV1_SUBTITLE    "an old-school polyphonic synthesizer."
#define SYNTHV1_WEBSITE     "http://synthv1.sourceforge.net"
#define SYNTHV1_COPYRIGHT   "Copyright (C) 2012-2017, rncbc aka Rui Nuno Capela. All rights reserved."

#define SYNTHV1_DOMAIN      "rncbc.org"


//-------------------------------------------------------------------------
// synthv1_config - Prototype settings class (singleton).
//

#include <QSettings>
#include <QStringList>

// forward decls.
class synthv1_programs;
class synthv1_controls;


class synthv1_config : public QSettings
{
public:

	// Constructor.
	synthv1_config();

	// Default destructor.
	~synthv1_config();

	// Default options...
	QString sPreset;
	QString sPresetDir;

	// Dial knob behavior mode.
	int iKnobDialMode;

	// Special persistent options.
	bool bControlsEnabled;
	bool bProgramsEnabled;
	bool bProgramsPreview;
	bool bUseNativeDialogs;
	// Run-time special non-persistent options.
	bool bDontUseNativeDialogs;

	// Custom widget style theme.
	QString sCustomStyleTheme;

	// Singleton instance accessor.
	static synthv1_config *getInstance();

	// Preset utility methods.
	QString presetFile(const QString& sPreset);
	void setPresetFile(const QString& sPreset, const QString& sPresetFile);
	void removePreset(const QString& sPreset);
	QStringList presetList();

	// Programs utility methods.
	void loadPrograms(synthv1_programs *pPrograms);
	void savePrograms(synthv1_programs *pPrograms);

	// Controllers utility methods.
	void loadControls(synthv1_controls *pControls);
	void saveControls(synthv1_controls *pControls);

protected:

	// Preset group path.
	QString presetGroup() const;

	// Banks programs group path.
	QString programsGroup() const;
	QString bankPrefix() const;

	void clearPrograms();

	// Controllers group path.
	QString controlsGroup() const;
	QString controlPrefix() const;

	void clearControls();

	// Explicit I/O methods.
	void load();
	void save();

private:

	// The singleton instance.
	static synthv1_config *g_pSettings;
};


#endif	// __synthv1_config_h

// end of synthv1_config.h

