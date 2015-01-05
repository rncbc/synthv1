// synthv1widget.h
//
/****************************************************************************
   Copyright (C) 2012-2015, rncbc aka Rui Nuno Capela. All rights reserved.

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

#ifndef __synthv1widget_h
#define __synthv1widget_h

#include "ui_synthv1widget.h"

#include "synthv1_config.h"

#include "synthv1.h"


// forward decls.
class synthv1_sched_notifier;


//-------------------------------------------------------------------------
// synthv1widget - decl.
//

class synthv1widget : public QWidget
{
	Q_OBJECT

public:

	// Constructor
	synthv1widget(QWidget *pParent = 0, Qt::WindowFlags wflags = 0);

	// Param port accessors.
	void setParamValue(
		synthv1::ParamIndex index, float fValue, bool bDefault = false);
	float paramValue(synthv1::ParamIndex index) const;

	// Param kbob (widget) mapper.
	void setParamKnob(synthv1::ParamIndex index, synthv1widget_knob *pKnob);
	synthv1widget_knob *paramKnob(synthv1::ParamIndex index) const;

	// Preset init.
	void initPreset();
	// Preset clear.
	void clearPreset();

	// Dirty close prompt,
	bool queryClose();

public slots:

	// Preset file I/O.
	void loadPreset(const QString& sFilename);
	void savePreset(const QString& sFilename);

protected slots:

	// Preset renewal.
	void newPreset();

	// Param knob (widget) slots.
	void paramChanged(float fValue);

	// Reset param knobs to default value.
	void resetParams();

	// Swap params A/B.
	void swapParams(bool bOn);

	// Delay BPM change.
	void bpmSyncChanged();

	// Schedule notification updater.
	void updateSchedNotify(int stype);

	// Menu actions.
	void helpConfigure();

	void helpAbout();
	void helpAboutQt();

protected:

	// Synth engine accessor.
	virtual synthv1 *instance() const = 0;

	// Reset swap params A/B group.
	void resetSwapParams();

	// Initialize all param/knob values.
	void updateParamValues();

	// Reset all param/knob default values.
	void resetParamValues();
	void resetParamKnobs();

	// Param port methods.
	virtual void updateParam(synthv1::ParamIndex index, float fValue) const = 0;

	// Update local tied widgets.
	void updateParamEx(synthv1::ParamIndex index, float fValue);

	// Dirty flag (overridable virtual) methods.
	virtual void updateDirtyPreset(bool bDirtyPreset);

private:

	// Instance variables.
	Ui::synthv1widget m_ui;

	synthv1_sched_notifier *m_sched_notifier;

	QHash<synthv1::ParamIndex, synthv1widget_knob *> m_paramKnobs;
	QHash<synthv1widget_knob *, synthv1::ParamIndex> m_knobParams;

	float m_params_ab[synthv1::NUM_PARAMS];

	int m_iUpdate;
};


#endif	// __synthv1widget_h

// end of synthv1widget.h
