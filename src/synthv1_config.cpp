// synthv1_config.cpp
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

#include "synthv1_config.h"

#include "synthv1_programs.h"
#include "synthv1_controls.h"

#include <QFileInfo>
#include <QDir>

#include <QCoreApplication>

#ifndef CONFIG_BINDIR
#define CONFIG_BINDIR	CONFIG_PREFIX "/bin"
#endif

#ifndef CONFIG_DATADIR
#define CONFIG_DATADIR	CONFIG_PREFIX "/share"
#endif

// Local static consts.
static const char *PresetsGroup       = "/Presets";
static const char *PresetsListKey     = "/PresetList";

static const char *PresetsBanksGroup  = "/Banks";
static const char *PresetsBankListKey = "/BankList";

static const char *PresetsConfGroup   = "/PresetsConf";
static const char *PresetsConfListKey = "/ConfList";

static const char *ProgramsGroup      = "/Programs";
static const char *BankPrefix         = "/Bank_";

static const char *ControlsGroup      = "/Controllers";
static const char *ControlPrefix      = "/Control";


//-------------------------------------------------------------------------
// synthv1_config - Prototype settings structure (pseudo-singleton).
//

// Singleton instance accessor (static).
synthv1_config *synthv1_config::g_pSettings = nullptr;

synthv1_config *synthv1_config::getInstance (void)
{
	return g_pSettings;
}


// Constructor.
synthv1_config::synthv1_config (void)
	: QSettings(PROJECT_DOMAIN, PROJECT_NAME)
{
	g_pSettings = this;

	load();
}


// Default destructor.
synthv1_config::~synthv1_config (void)
{
	save();

	g_pSettings = nullptr;
}


// Preset utility methods.
QString synthv1_config::presetFile ( const QString& sPreset )
{
	QSettings::beginGroup(PresetsGroup);
	const QString sPresetFile(QSettings::value(sPreset).toString());
	QSettings::endGroup();
	return sPresetFile;
}


void synthv1_config::setPresetFile (
	const QString& sPreset, const QString& sPresetFile )
{
	QSettings::beginGroup(PresetsGroup);
	QSettings::setValue(sPreset, sPresetFile);
	QSettings::endGroup();
}


void synthv1_config::removePreset ( const QString& sPreset )
{
	QSettings::beginGroup(PresetsGroup);
	const QString& sPresetFile = QSettings::value(sPreset).toString();
	if (QFileInfo::exists(sPresetFile))
		QFile(sPresetFile).remove();
	QSettings::remove(sPreset);
	QSettings::endGroup();
}


// Presets utility methods.
void synthv1_config::importPresets (
	const QString& sFilename, synthv1_presets *pPresets )
{
	QSettings settings(sFilename, QSettings::IniFormat);

	struct AbsPath : MapPath
	{
		AbsPath(const QString& sFilename)
			: dir(QFileInfo(sFilename).absolutePath()) {}

		QString operator()(const QString& sPath) const override
			{ return QFileInfo(dir.filePath(sPath)).canonicalFilePath(); }

		QDir dir;
	};

	loadPresets(&settings, pPresets, AbsPath(sFilename));
}


void synthv1_config::loadPresets ( QSettings *pSettings,
	synthv1_presets *pPresets,	const MapPath& mapPath )
{
	QStringList bank_list;
	pSettings->beginGroup(PresetsBankListKey);
	bank_list = pSettings->value(PresetsBankListKey).toStringList();
	pSettings->endGroup();

	pSettings->beginGroup(PresetsBanksGroup);
	QStringListIterator bank_iter(bank_list);
	while (bank_iter.hasNext()) {
		const QString& sBank = bank_iter.next();
		synthv1_presets::Bank *pBank = pPresets->add_bank(sBank);
		const QStringList& preset_list
			= pSettings->value(sBank).toStringList();
		QStringListIterator preset_iter(preset_list);
		while (preset_iter.hasNext()) {
			const QString& sPreset = preset_iter.next();
			pBank->add_preset(sPreset);
		}
	}
	pSettings->endGroup();

	QStringList preset_list;
	pSettings->beginGroup(PresetsListKey);
	preset_list = pSettings->value(PresetsListKey).toStringList();
	pSettings->endGroup();

	pSettings->beginGroup(PresetsGroup);
	const QStringList& preset_keys
		= pSettings->childKeys();
	QStringListIterator preset_key(preset_keys);
	while (preset_key.hasNext()) {
		const QString& sPresetKey = preset_key.next();
		if (!preset_list.contains(sPresetKey))
			preset_list.append(sPresetKey);
	}
	QStringListIterator preset_iter(preset_list);
	while (preset_iter.hasNext()) {
		const QString& sPreset = preset_iter.next();
		synthv1_presets::Preset *pPreset = pPresets->find_preset(sPreset);
		if (pPreset == nullptr)
			pPreset = pPresets->add_preset(sPreset);
		const QString& sPresetFile
			= mapPath(pSettings->value(sPreset).toString());
		if (!sPresetFile.isEmpty()
			&& QFileInfo::exists(sPresetFile)) {
			pPreset->set_file(sPresetFile);
		} else {
			pPresets->remove_preset(sPreset);
		}
	}
	pSettings->endGroup();

	// cleanup database from dangling banks/presets...
	//
	bank_iter.toFront();
	while (bank_iter.hasNext()) {
		const QString& sBank = bank_iter.next();
		synthv1_presets::Bank *pBank = pPresets->find_bank(sBank);
		if (pBank == nullptr) {
			pPresets->remove_bank(sBank);
		} else {
			const QStringList& preset_list = pBank->preset_list();
			QStringListIterator preset_iter(preset_list);
			while (preset_iter.hasNext()) {
				const QString& sPreset = preset_iter.next();
				if (pPresets->find_preset(sPreset) == nullptr)
					pBank->remove_preset(sPreset);
			}
		}
	}
}


void synthv1_config::loadPresets (void)
{
	presets.clear_banks();
	presets.clear_presets();

	loadPresets(this, &presets);

	if (loadPresetsConf(this, &presets) > 0)
		savePresets();
}


void synthv1_config::exportPresets (
	const QString& sFilename, synthv1_presets *pPresets )
{
	QSettings settings(sFilename, QSettings::IniFormat);

	struct RelPath : MapPath
	{
		RelPath(const QString& sFilename)
			: dir(QFileInfo(sFilename).absolutePath()) {}

		QString operator()(const QString& sPath) const override
			{ return dir.relativeFilePath(sPath); }

		QDir dir;
	};

	savePresets(&settings, pPresets, RelPath(sFilename));
}


void synthv1_config::savePresets (
	QSettings *pSettings, synthv1_presets *pPresets, const MapPath& mapPath )
{
	const QStringList& bank_list = pPresets->bank_list();
	pSettings->beginGroup(PresetsBankListKey);
	if (bank_list.isEmpty())
		pSettings->remove(PresetsBankListKey);
	else
		pSettings->setValue(PresetsBankListKey, bank_list);
	pSettings->endGroup();

	pSettings->beginGroup(PresetsBanksGroup);
	const QStringList& bank_keys
		= pSettings->childKeys();
	QStringListIterator bank_key(bank_keys);
	while (bank_key.hasNext())
		pSettings->remove(bank_key.next());
	QStringListIterator bank_iter(bank_list);
	while (bank_iter.hasNext()) {
		const QString& sBank = bank_iter.next();
		synthv1_presets::Bank *pBank = pPresets->find_bank(sBank);
		if (pBank == nullptr)
			continue;
		QStringList preset_list;
		QStringListIterator bank_preset_iter(pBank->preset_list());
		while (bank_preset_iter.hasNext()) {
			const QString& sPreset
				= bank_preset_iter.next();
			preset_list.append(sPreset);
		}
		if (preset_list.isEmpty())
			pSettings->remove(sBank);
		else
			pSettings->setValue(sBank, preset_list);
	}
	pSettings->endGroup();

	const QStringList& preset_list = pPresets->preset_list();
	pSettings->beginGroup(PresetsListKey);
	if (preset_list.isEmpty())
		pSettings->remove(PresetsListKey);
	else
		pSettings->setValue(PresetsListKey, preset_list);
	pSettings->endGroup();

	pSettings->beginGroup(PresetsGroup);
	const QStringList& preset_keys
		= pSettings->childKeys();
	QStringListIterator preset_key(preset_keys);
	while (preset_key.hasNext())
		pSettings->remove(preset_key.next());
	const synthv1_presets::Presets& presets_map
		= pPresets->presets();
	synthv1_presets::Presets::ConstIterator presets_iter
		= presets_map.constBegin();
	const synthv1_presets::Presets::ConstIterator& presets_end
		= presets_map.constEnd();
	for ( ; presets_iter != presets_end; ++presets_iter) {
		const QString& sPreset = presets_iter.key();
		synthv1_presets::Preset *pPreset = presets_iter.value();
		const QString& sPresetFile = mapPath(pPreset->file());
		if (!sPresetFile.isEmpty()) {
			pSettings->setValue(sPreset, sPresetFile);
		} else {
			pPresets->remove_preset(sPreset);
		}
	}
	pSettings->endGroup();
	pSettings->sync();
}


void synthv1_config::savePresets (void)
{
	savePresets(this, &presets);
}


// Factory presets(.conf) loading...
//
int synthv1_config::loadPresetsConf (
	QSettings *pSettings, synthv1_presets *pPresets )
{
	int nconfs = 0;

	pSettings->beginGroup(PresetsConfGroup);
	QStringList confs = pSettings->value(PresetsConfListKey).toStringList();
	if (confs.isEmpty() || pPresets->isEmpty()) {
		const QChar sep = QDir::separator();
		QString sPresetsPath = QCoreApplication::applicationDirPath();
		sPresetsPath.remove(CONFIG_BINDIR);
		sPresetsPath.append(CONFIG_DATADIR);
		sPresetsPath.append(sep);
		sPresetsPath.append(PROJECT_NAME);
		sPresetsPath.append(sep);
		sPresetsPath.append("preset");
		QDir dir(sPresetsPath);
		if (dir.exists() && dir.isReadable()) {
			pSettings->remove(PresetsConfListKey);
			nconfs += loadPresetsConfDir(pPresets, dir, confs);
			if (nconfs > 0)
				pSettings->setValue(PresetsConfListKey, confs);
		}
	}
	pSettings->endGroup();

	return nconfs;
}


int synthv1_config::loadPresetsConfDir (
	synthv1_presets *pPresets, const QDir& dir, QStringList& confs )
{
	int nconfs = 0;

	const QDir::Filters filters
		= QDir::AllDirs | QDir::NoDotAndDotDot;
	QStringListIterator dir_iter(dir.entryList(filters));
	while (dir_iter.hasNext()) {
		nconfs += loadPresetsConfDir(pPresets,
			QDir(QFileInfo(dir, dir_iter.next()).filePath()), confs);
	}

	const QStringList filter("*." PROJECT_NAME ".conf");
	QStringListIterator iter(dir.entryList(filter, QDir::Files));
	while (iter.hasNext()) {
		const QFileInfo fi(dir, iter.next());
		const QString& sFilename = fi.absoluteFilePath();
		if (!confs.contains(sFilename)) {
			synthv1_config::importPresets(sFilename, pPresets);
			confs.append(sFilename);
			++nconfs;
		}
	}

	return nconfs;
}


// Programs utility methods.
void synthv1_config::loadPrograms ( synthv1_programs *pPrograms )
{
	pPrograms->clear_banks();

	QSettings::beginGroup(ProgramsGroup);

	const QStringList& bank_keys = QSettings::childKeys();
	QStringListIterator bank_iter(bank_keys);
	while (bank_iter.hasNext()) {
		const QString& bank_key = bank_iter.next();
		uint16_t bank_id = bank_key.toInt();
		const QString& bank_name
			= QSettings::value(bank_key).toString();
		synthv1_programs::Bank *pBank = pPrograms->add_bank(bank_id, bank_name);
		QSettings::beginGroup(BankPrefix + bank_key);
		const QStringList& prog_keys = QSettings::childKeys();
		QStringListIterator prog_iter(prog_keys);
		while (prog_iter.hasNext()) {
			const QString& prog_key = prog_iter.next();
			uint16_t prog_id = prog_key.toInt();
			const QString& prog_name
				= QSettings::value(prog_key).toString();
			pBank->add_prog(prog_id, prog_name);
		}
		QSettings::endGroup();
	}

	QSettings::endGroup();

	pPrograms->enabled(bProgramsEnabled);
}


void synthv1_config::savePrograms ( synthv1_programs *pPrograms )
{
	bProgramsEnabled = pPrograms->enabled();

	clearPrograms();

	QSettings::beginGroup(ProgramsGroup);

	const synthv1_programs::Banks& banks = pPrograms->banks();
	synthv1_programs::Banks::ConstIterator bank_iter = banks.constBegin();
	const synthv1_programs::Banks::ConstIterator& bank_end = banks.constEnd();
	for ( ; bank_iter != bank_end; ++bank_iter) {
		synthv1_programs::Bank *pBank = bank_iter.value();
		const QString& bank_key = QString::number(pBank->id());
		const QString& bank_name = pBank->name();
		QSettings::setValue(bank_key, bank_name);
		QSettings::beginGroup(BankPrefix + bank_key);
		const synthv1_programs::Progs& progs = pBank->progs();
		synthv1_programs::Progs::ConstIterator prog_iter = progs.constBegin();
		const synthv1_programs::Progs::ConstIterator& prog_end = progs.constEnd();
		for ( ; prog_iter != prog_end; ++prog_iter) {
			synthv1_programs::Prog *pProg = prog_iter.value();
			const QString& prog_key = QString::number(pProg->id());
			const QString& prog_name = pProg->name();
			QSettings::setValue(prog_key, prog_name);
		}
		QSettings::endGroup();
	}

	QSettings::endGroup();
	QSettings::sync();
}


void synthv1_config::clearPrograms (void)
{
	QSettings::beginGroup(ProgramsGroup);

	const QStringList& bank_keys = QSettings::childKeys();
	QStringListIterator bank_iter(bank_keys);
	while (bank_iter.hasNext()) {
		const QString& bank_key = bank_iter.next();
		QSettings::beginGroup(BankPrefix + bank_key);
		const QStringList& prog_keys = QSettings::childKeys();
		QStringListIterator prog_iter(prog_keys);
		while (prog_iter.hasNext()) {
			const QString& prog_key = prog_iter.next();
			QSettings::remove(prog_key);
		}
		QSettings::endGroup();
		QSettings::remove(bank_key);
	}

	QSettings::endGroup();
}


// Controllers utility methods.
void synthv1_config::loadControls ( synthv1_controls *pControls )
{
	pControls->clear();

	QSettings::beginGroup(ControlsGroup);

	const QStringList& keys = QSettings::childKeys();
	QStringListIterator iter(keys);
	while (iter.hasNext()) {
		const QString& sKey = '/' + iter.next();
		const QStringList& clist = sKey.split('_');
		if (clist.at(0) == ControlPrefix) {
			const unsigned short channel
				= clist.at(1).toInt();
			const synthv1_controls::Type ctype
				= synthv1_controls::typeFromText(clist.at(2));
			synthv1_controls::Key key;
			key.status = ctype | (channel & 0x1f);
			key.param = clist.at(3).toInt();
			const QStringList& vlist
				= QSettings::value(sKey).toStringList();
			synthv1_controls::Data data;
			data.index = vlist.at(0).toInt();
			if (vlist.count() > 1)
				data.flags = vlist.at(1).toInt();
			pControls->add_control(key, data);
		}
	}

	QSettings::endGroup();

	pControls->enabled(bControlsEnabled);
}


void synthv1_config::saveControls ( synthv1_controls *pControls )
{
	bControlsEnabled = pControls->enabled();

	clearControls();

	QSettings::beginGroup(ControlsGroup);

	const synthv1_controls::Map& map = pControls->map();
	synthv1_controls::Map::ConstIterator iter = map.constBegin();
	const synthv1_controls::Map::ConstIterator& iter_end = map.constEnd();
	for ( ; iter != iter_end; ++iter) {
		const synthv1_controls::Key& key = iter.key();
		QString sKey = ControlPrefix;
		sKey += '_' + QString::number(key.channel());
		sKey += '_' + synthv1_controls::textFromType(key.type());
		sKey += '_' + QString::number(key.param);
		const synthv1_controls::Data& data = iter.value();
		QStringList vlist;
		vlist.append(QString::number(data.index));
		vlist.append(QString::number(data.flags));
		QSettings::setValue(sKey, vlist);
	}

	QSettings::endGroup();
	QSettings::sync();
}


void synthv1_config::clearControls (void)
{
	QSettings::beginGroup(ControlsGroup);

	const QStringList& keys = QSettings::childKeys();
	QStringListIterator iter(keys);
	while (iter.hasNext()) {
		const QString& key = iter.next();
		QSettings::remove(key);
	}

	QSettings::endGroup();
}


// Explicit I/O methods.
void synthv1_config::load (void)
{
	QSettings::beginGroup("/Default");
	sPreset = QSettings::value("/Preset").toString();
	sPresetDir = QSettings::value("/PresetDir").toString();
	iKnobDialMode = QSettings::value("/KnobDialMode", 0).toInt();
	iKnobEditMode = QSettings::value("/KnobEditMode", 0).toInt();
	fRandomizePercent = QSettings::value("/RandomizePercent", 20.0f).toFloat();
	bControlsEnabled = QSettings::value("/ControlsEnabled", false).toBool();
	bProgramsEnabled = QSettings::value("/ProgramsEnabled", false).toBool();
	QSettings::endGroup();

	QSettings::beginGroup("/Dialogs");
	bPresetsPreview = QSettings::value("/PresetsPreview", false).toBool();
	bProgramsPreview = QSettings::value("/ProgramsPreview", false).toBool();
	bUseNativeDialogs = QSettings::value("/UseNativeDialogs", false).toBool();
	// Run-time special non-persistent options.
	bDontUseNativeDialogs = !bUseNativeDialogs;
	QSettings::endGroup();

	QSettings::beginGroup("/Custom");
	sCustomColorTheme = QSettings::value("/ColorTheme").toString();
	sCustomStyleTheme = QSettings::value("/StyleTheme").toString();
	QSettings::endGroup();

	// Micro-tuning options.
	QSettings::beginGroup("/Tuning");
	bTuningEnabled = QSettings::value("/Enabled", false).toBool();
	fTuningRefPitch = float(QSettings::value("/RefPitch", 440.0).toDouble());
	iTuningRefNote = QSettings::value("/RefNote", 69).toInt();
	sTuningScaleDir = QSettings::value("/ScaleDir").toString();
	sTuningScaleFile = QSettings::value("/ScaleFile").toString();
	sTuningKeyMapDir = QSettings::value("/KeyMapDir").toString();
	sTuningKeyMapFile = QSettings::value("/KeyMapFile").toString();
	QSettings::endGroup();

	// Presets database.
	loadPresets();
}


void synthv1_config::save (void)
{
	QSettings::beginGroup("/Program");
	QSettings::setValue("/Version", PROJECT_VERSION);
	QSettings::endGroup();

	QSettings::beginGroup("/Default");
	QSettings::setValue("/Preset", sPreset);
	QSettings::setValue("/PresetDir", sPresetDir);
	QSettings::setValue("/KnobDialMode", iKnobDialMode);
	QSettings::setValue("/KnobEditMode", iKnobEditMode);
	QSettings::setValue("/RandomizePercent", fRandomizePercent);
	QSettings::setValue("/ControlsEnabled", bControlsEnabled);
	QSettings::setValue("/ProgramsEnabled", bProgramsEnabled);
	QSettings::endGroup();

	QSettings::beginGroup("/Dialogs");
	QSettings::setValue("/PresetsPreview", bPresetsPreview);
	QSettings::setValue("/ProgramsPreview", bProgramsPreview);
	QSettings::setValue("/UseNativeDialogs", bUseNativeDialogs);
	QSettings::endGroup();

	QSettings::beginGroup("/Custom");
	QSettings::setValue("/ColorTheme", sCustomColorTheme);
	QSettings::setValue("/StyleTheme", sCustomStyleTheme);
	QSettings::endGroup();

	// Micro-tuning options.
	QSettings::beginGroup("/Tuning");
	QSettings::setValue("/Enabled", bTuningEnabled);
	QSettings::setValue("/RefPitch", double(fTuningRefPitch));
	QSettings::setValue("/RefNote", iTuningRefNote);
	QSettings::setValue("/ScaleDir", sTuningScaleDir);
	QSettings::setValue("/ScaleFile", sTuningScaleFile);
	QSettings::setValue("/KeyMapDir", sTuningKeyMapDir);
	QSettings::setValue("/KeyMapFile", sTuningKeyMapFile);
	QSettings::endGroup();

	// Presets database.
	savePresets();
}


// end of synthv1_config.cpp

