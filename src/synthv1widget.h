// synthv1widget.h
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

#ifndef __synthv1widget_h
#define __synthv1widget_h

#include "synthv1_config.h"
#include "synthv1_sched.h"

#include "synthv1_ui.h"

#include <QWidget>


// forward decls.
namespace Ui { class synthv1widget; }

class synthv1widget_param;
class synthv1widget_sched;


//-------------------------------------------------------------------------
// synthv1widget - decl.
//

class synthv1widget : public QWidget
{
	Q_OBJECT

public:

	// Constructor
	synthv1widget(QWidget *pParent = nullptr);

	// Destructor.
	virtual ~synthv1widget();

	// Open/close the scheduler/work notifier.
	void openSchedNotifier();
	void closeSchedNotifier();

	// Param port accessors.
	void setParamValue(synthv1::ParamIndex index, float fValue);
	float paramValue(synthv1::ParamIndex index) const;

	// Param kbob (widget) mapper.
	void setParamKnob(synthv1::ParamIndex index, synthv1widget_param *pKnob);
	synthv1widget_param *paramKnob(synthv1::ParamIndex index) const;

	// Preset init.
	void initPreset();
	// Preset clear.
	void clearPreset();

	// Dirty close prompt,
	bool queryClose();

	// Update visual configuration.
	void updateConfig();

public slots:

	// Preset file I/O.
	bool loadPreset(const QString& sFilename);
	bool savePreset(const QString& sFilename);

	// Direct note-on/off slot.
	void directNoteOn(int iNote, int iVelocity);

protected slots:

	// Preset renewal.
	void newPreset();

	// Param knob (widget) slots.
	void paramChanged(float fValue);

	// Reset param knobs to default value.
	void resetParams();

	// Randomize params (partial).
	void randomParams();

	// Swap params A/B.
	void swapParams(bool bOn);

	// Panic: all-notes/sound-off (reset).
	void panic();
	
	// Schedule notification updater.
	void updateSchedNotify(int stype, int sid);

	// MIDI In LED timeout.
	void midiInLedTimeout();

	// Keyboard note range change.
	void noteRangeChanged();

	// Param knob context menu.
	void paramContextMenu(const QPoint& pos);

	// Menu actions.
	void helpConfigure();

	void helpAbout();
	void helpAboutQt();

protected:

	// Synth engine accessor.
	virtual synthv1_ui *ui_instance() const = 0;

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

	// Update scheduled controllers param/knob widgets.
	void updateSchedParam(synthv1::ParamIndex index, float fValue);

	// Preset status updater.
	void updateLoadPreset(const QString& sPreset);

	// Dirty flag (overridable virtual) methods.
	virtual void updateDirtyPreset(bool bDirtyPreset);

	// Show/hide dget handlers.
	void showEvent(QShowEvent *pShowEvent);
	void hideEvent(QHideEvent *pHideEvent);

private:

	// Instance variables.
	Ui::synthv1widget *p_ui;
	Ui::synthv1widget& m_ui;

	synthv1widget_sched *m_sched_notifier;

	QHash<synthv1::ParamIndex, synthv1widget_param *> m_paramKnobs;
	QHash<synthv1widget_param *, synthv1::ParamIndex> m_knobParams;

	float m_params_ab[synthv1::NUM_PARAMS];

	int m_iUpdate;
};


//-------------------------------------------------------------------------
// synthv1widget_sched - worker/schedule proxy decl.
//

class synthv1widget_sched : public QObject
{
	Q_OBJECT

public:

	// ctor.
	synthv1widget_sched(synthv1 *pSynth, QObject *pParent = nullptr)
		: QObject(pParent), m_notifier(pSynth, this) {}

signals:

	// Notification signal.
	void notify(int stype, int sid);

protected:

	// Notififier visitor.
	class Notifier : public synthv1_sched::Notifier
	{
	public:

		Notifier(synthv1 *pSynth, synthv1widget_sched *pSched)
			: synthv1_sched::Notifier(pSynth), m_pSched(pSched) {}

		void notify(synthv1_sched::Type stype, int sid) const
			{ m_pSched->emit_notify(stype, sid); }

	private:

		synthv1widget_sched *m_pSched;
	};

	// Notification method.
	void emit_notify(synthv1_sched::Type stype, int sid)
		{ emit notify(int(stype), sid); }

private:

	// Instance variables.
	Notifier m_notifier;
};


#endif	// __synthv1widget_h

// end of synthv1widget.h
