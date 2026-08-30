// synthv1_config.h
//
/****************************************************************************
   Copyright (C) 2012-2026, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "synthv1_presets.h"

#include <QSettings>
#include <QStringList>

// forward decls.
class synthv1_programs;
class synthv1_controls;

class QDir;


//-------------------------------------------------------------------------
// synthv1_config - Prototype settings class (singleton).
//

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

	// Knob behavior modes.
	int iKnobDialMode;
	int iKnobEditMode;

	// Default randomize factor (percent).
	float fRandomizePercent;

	// Special persistent options.
	bool bControlsEnabled;
	bool bProgramsEnabled;
	bool bProgramsPreview;
	bool bPresetsPreview;
	bool bUseNativeDialogs;
	// Run-time special non-persistent options.
	bool bDontUseNativeDialogs;

	// Custom color palette/widget style themes.
	QString sCustomColorTheme;
	QString sCustomStyleTheme;

	// Micro-tuning options.
	bool    bTuningEnabled;
	float   fTuningRefPitch;
	int     iTuningRefNote;
	QString sTuningScaleDir;
	QString sTuningScaleFile;
	QString sTuningKeyMapDir;
	QString sTuningKeyMapFile;

	// Presets database.
	synthv1_presets presets;

	// Singleton instance accessor.
	static synthv1_config *getInstance();

	// Preset utility methods.
	QString presetFile(const QString& sPreset);
	void setPresetFile(const QString& sPreset, const QString& sPresetFile);
	void removePreset(const QString& sPreset);

	// Presets utility methods.
	void loadPresets();
	void savePresets();

	static void importPresets(
		const QString& sFilename,
		synthv1_presets *pPresets);
	static void exportPresets(
		const QString& sFilename,
		synthv1_presets *pPresets);

	// Programs utility methods.
	void loadPrograms(synthv1_programs *pPrograms);
	void savePrograms(synthv1_programs *pPrograms);

	// Controllers utility methods.
	void loadControls(synthv1_controls *pControls);
	void saveControls(synthv1_controls *pControls);

protected:

	// Absolute/relative path functors.
	struct MapPath
	{
		virtual QString operator()(const QString& sPath) const
			{ return sPath; }
	};

	static void loadPresets(
		QSettings *pSettings,
		synthv1_presets *pPresets,
		const MapPath& mapPath = MapPath());
	static void savePresets(
		QSettings *pSettings,
		synthv1_presets *pPresets,
		const MapPath& mapPath = MapPath());

	static int loadPresetsConf(
		QSettings *pSettings,
		synthv1_presets *pPresets);
	static int loadPresetsConfDir(
		synthv1_presets *pPresets,
		const QDir& dir, QStringList& confs);

	void clearPrograms();
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

