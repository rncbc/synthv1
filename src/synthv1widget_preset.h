// synthv1widget_preset.h
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

#ifndef __synthv1widget_preset_h
#define __synthv1widget_preset_h

#include "synthv1widget_presets.h"

#include <QWidget>


// Forward declarations.
class synthv1_presets;

class QToolButton;
class QComboBox;

class QTreeWidgetItem;


//-------------------------------------------------------------------------
// synthv1widget_preset - Custom preset-box widget.

class synthv1widget_preset : public QWidget
{
	Q_OBJECT

public:

	// Constructor.
	synthv1widget_preset(QWidget *pParent = nullptr);

	void setPreset(const QString& sPreset);
	QString preset() const;

	void setDirtyPreset(bool bDirtyPreset);
	bool isDirtyPreset() const;

	void initPreset();

	void clearPreset();
	bool queryPreset();

	void reloadPresets(synthv1_presets *pPresets = nullptr);

signals:

	void newPresetFile();

	void loadPresetFile(const QString&);
	void savePresetFile(const QString&);

	void resetPresetFile();

	void presetActivated(const QString&);

	void refreshPresets();

protected slots:

	void newPreset();
	void openPreset();
	void activatePreset(const QString&);
	void savePreset();
	void deletePreset();
	void resetPreset();

protected:

	void loadPreset(const QString&);
	void savePreset(const QString&);

	void setPresetItem(const QString& sPreset);
	QTreeWidgetItem *presetItem(const QString& sPreset) const;

	void setBankItem(const QString& sBank, int iPreset = 0);
	QTreeWidgetItem *bankItem(const QString& sBank) const;

	void stabilizePreset();

private:

	// Widget members.
	QToolButton *m_pNewButton;
	QToolButton *m_pOpenButton;

	synthv1widget_presets::ComboBox *m_pComboBox;

	QToolButton *m_pSaveButton;
	QToolButton *m_pDeleteButton;
	QToolButton *m_pResetButton;

	synthv1widget_presets *m_pPresetsView;

	int m_iInitPreset;
	int m_iDirtyPreset;
};


#endif  // __synthv1widget_preset_h

// end of synthv1widget_preset.h
