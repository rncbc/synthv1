// synthv1widget.cpp
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

#include "synthv1widget.h"
#include "synthv1_param.h"

#include "synthv1_wave.h"
#include "synthv1_sched.h"

#include "synthv1widget_config.h"
#include "synthv1widget_control.h"

#include <QMessageBox>
#include <QDir>
#include <QTimer>

#include <QShowEvent>
#include <QHideEvent>


//-------------------------------------------------------------------------
// synthv1widget - impl.
//

// Constructor.
synthv1widget::synthv1widget ( QWidget *pParent, Qt::WindowFlags wflags )
	: QWidget(pParent, wflags)
{
	Q_INIT_RESOURCE(synthv1);

#if QT_VERSION >= 0x050000
	// HACK: Dark themes grayed/disabled color group fix...
	QPalette pal;
	if (pal.base().color().value() < 0x7f) {
		const QColor& color = pal.window().color();
		const int iGroups = int(QPalette::Active | QPalette::Inactive) + 1;
		for (int i = 0; i < iGroups; ++i) {
			const QPalette::ColorGroup group = QPalette::ColorGroup(i);
			pal.setBrush(group, QPalette::Light,    color.lighter(150));
			pal.setBrush(group, QPalette::Midlight, color.lighter(120));
			pal.setBrush(group, QPalette::Dark,     color.darker(150));
			pal.setBrush(group, QPalette::Mid,      color.darker(120));
			pal.setBrush(group, QPalette::Shadow,   color.darker(200));
		}
		pal.setColor(QPalette::Disabled, QPalette::ButtonText, pal.mid().color());
		QWidget::setPalette(pal);
	}
#endif

	m_ui.setupUi(this);

	// Init sched notifier.
	m_sched_notifier = NULL;

	// Init swapable params A/B to default.
	for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i)
		m_params_ab[i] = synthv1_param::paramDefaultValue(synthv1::ParamIndex(i));

	// Start clean.
	m_iUpdate = 0;

	// Replicate the stacked/pages
	for (int iTab = 0; iTab < m_ui.StackedWidget->count(); ++iTab)
		m_ui.TabBar->addTab(m_ui.StackedWidget->widget(iTab)->windowTitle());

	// Swappable params A/B group.
	QButtonGroup *pSwapParamsGroup = new QButtonGroup(this);
	pSwapParamsGroup->addButton(m_ui.SwapParamsAButton);
	pSwapParamsGroup->addButton(m_ui.SwapParamsBButton);
	pSwapParamsGroup->setExclusive(true);
	m_ui.SwapParamsAButton->setChecked(true);

	// Wave shapes.
	QStringList shapes;
	shapes << tr("Pulse");
	shapes << tr("Saw");
	shapes << tr("Sine");
	shapes << tr("Rand");
	shapes << tr("Noise");

	m_ui.Dco1Shape1Knob->insertItems(0, shapes);
	m_ui.Dco1Shape2Knob->insertItems(0, shapes);
	m_ui.Dco2Shape1Knob->insertItems(0, shapes);
	m_ui.Dco2Shape2Knob->insertItems(0, shapes);

	m_ui.Lfo1ShapeKnob->insertItems(0, shapes);
	m_ui.Lfo2ShapeKnob->insertItems(0, shapes);

	// Filter types.
	QStringList types;
	types << tr("LPF");
	types << tr("BPF");
	types << tr("HPF");
	types << tr("BRF");

	m_ui.Dcf1TypeKnob->insertItems(0, types);
	m_ui.Dcf2TypeKnob->insertItems(0, types);

	// Filter slopes.
	QStringList slopes;
	slopes << tr("12dB/oct");
	slopes << tr("24dB/oct");
	slopes << tr("Biquad");
	slopes << tr("Formant");

	m_ui.Dcf1SlopeKnob->insertItems(0, slopes);
	m_ui.Dcf2SlopeKnob->insertItems(0, slopes);

	// Dynamic states.
	QStringList states;
	states << tr("Off");
	states << tr("On");

	m_ui.Dco1Bandl1Knob->insertItems(0, states);
	m_ui.Dco1Bandl2Knob->insertItems(0, states);
	m_ui.Dco2Bandl1Knob->insertItems(0, states);
	m_ui.Dco2Bandl2Knob->insertItems(0, states);

	m_ui.Lfo1SyncKnob->insertItems(0, states);
	m_ui.Lfo2SyncKnob->insertItems(0, states);

	m_ui.Dyn1CompressKnob->insertItems(0, states);
	m_ui.Dyn1LimiterKnob->insertItems(0, states);

	// Special values
	const QString& sOff = states.first();
	m_ui.Dco1RingModKnob->setSpecialValueText(sOff);
	m_ui.Dco2RingModKnob->setSpecialValueText(sOff);
	m_ui.Dco1GlideKnob->setSpecialValueText(sOff);
	m_ui.Dco2GlideKnob->setSpecialValueText(sOff);
	m_ui.Cho1WetKnob->setSpecialValueText(sOff);
	m_ui.Fla1WetKnob->setSpecialValueText(sOff);
	m_ui.Pha1WetKnob->setSpecialValueText(sOff);
	m_ui.Del1WetKnob->setSpecialValueText(sOff);
	m_ui.Rev1WetKnob->setSpecialValueText(sOff);

	const QString& sAuto = tr("Auto");
	m_ui.Lfo1BpmKnob->setSpecialValueText(sAuto);
	m_ui.Lfo2BpmKnob->setSpecialValueText(sAuto);
	m_ui.Del1BpmKnob->setSpecialValueText(sAuto);

	// Wave integer widths.
	m_ui.Dco1Width1Knob->setDecimals(0);
	m_ui.Dco1Width2Knob->setDecimals(0);
	m_ui.Dco2Width1Knob->setDecimals(0);
	m_ui.Dco2Width2Knob->setDecimals(0);

	m_ui.Lfo1WidthKnob->setDecimals(0);
	m_ui.Lfo2WidthKnob->setDecimals(0);

	// DCO octave limits.
	m_ui.Dco1OctaveKnob->setMinimum(-4.0f);
	m_ui.Dco1OctaveKnob->setMaximum(+4.0f);

	m_ui.Dco2OctaveKnob->setMinimum(-4.0f);
	m_ui.Dco2OctaveKnob->setMaximum(+4.0f);

	// DCO balance limits.
	m_ui.Dco1BalanceKnob->setMinimum(-1.0f);
	m_ui.Dco1BalanceKnob->setMaximum(+1.0f);

	m_ui.Dco2BalanceKnob->setMinimum(-1.0f);
	m_ui.Dco2BalanceKnob->setMaximum(+1.0f);

	// DCO tune limits.
	m_ui.Dco1TuningKnob->setMinimum(-1.0f);
	m_ui.Dco1TuningKnob->setMaximum(+1.0f);

	m_ui.Dco2TuningKnob->setMinimum(-1.0f);
	m_ui.Dco2TuningKnob->setMaximum(+1.0f);

	// DCF volume (env.amount) limits.
	m_ui.Dcf1EnvelopeKnob->setMinimum(-1.0f);
	m_ui.Dcf1EnvelopeKnob->setMaximum(+1.0f);

	m_ui.Dcf2EnvelopeKnob->setMinimum(-1.0f);
	m_ui.Dcf2EnvelopeKnob->setMaximum(+1.0f);

	// LFO parameter limits.
	m_ui.Lfo1BpmKnob->setScale(1.0f);
	m_ui.Lfo1BpmKnob->setMinimum(0.0f);
	m_ui.Lfo1BpmKnob->setMaximum(360.0f);
//	m_ui.Lfo1BpmKnob->setSingleStep(1.0f);
	m_ui.Lfo1SweepKnob->setMinimum(-1.0f);
	m_ui.Lfo1SweepKnob->setMaximum(+1.0f);
	m_ui.Lfo1CutoffKnob->setMinimum(-1.0f);
	m_ui.Lfo1CutoffKnob->setMaximum(+1.0f);
	m_ui.Lfo1ResoKnob->setMinimum(-1.0f);
	m_ui.Lfo1ResoKnob->setMaximum(+1.0f);
	m_ui.Lfo1PitchKnob->setMinimum(-1.0f);
	m_ui.Lfo1PitchKnob->setMaximum(+1.0f);
	m_ui.Lfo1RingModKnob->setMinimum(-1.0f);
	m_ui.Lfo1RingModKnob->setMaximum(+1.0f);
	m_ui.Lfo1PanningKnob->setMinimum(-1.0f);
	m_ui.Lfo1PanningKnob->setMaximum(+1.0f);
	m_ui.Lfo1VolumeKnob->setMinimum(-1.0f);
	m_ui.Lfo1VolumeKnob->setMaximum(+1.0f);

	m_ui.Lfo2BpmKnob->setScale(1.0f);
	m_ui.Lfo2BpmKnob->setMinimum(0.0f);
	m_ui.Lfo2BpmKnob->setMaximum(360.0f);
//	m_ui.Lfo2BpmKnob->setSingleStep(1.0f);
	m_ui.Lfo2SweepKnob->setMinimum(-1.0f);
	m_ui.Lfo2SweepKnob->setMaximum(+1.0f);
	m_ui.Lfo2CutoffKnob->setMinimum(-1.0f);
	m_ui.Lfo2CutoffKnob->setMaximum(+1.0f);
	m_ui.Lfo2ResoKnob->setMinimum(-1.0f);
	m_ui.Lfo2ResoKnob->setMaximum(+1.0f);
	m_ui.Lfo2PitchKnob->setMinimum(-1.0f);
	m_ui.Lfo2PitchKnob->setMaximum(+1.0f);
	m_ui.Lfo2RingModKnob->setMinimum(-1.0f);
	m_ui.Lfo2RingModKnob->setMaximum(+1.0f);
	m_ui.Lfo2PanningKnob->setMinimum(-1.0f);
	m_ui.Lfo2PanningKnob->setMaximum(+1.0f);
	m_ui.Lfo2VolumeKnob->setMinimum(-1.0f);
	m_ui.Lfo2VolumeKnob->setMaximum(+1.0f);


	// Channel filters
	QStringList channels;
	channels << tr("Omni");
	for (int iChannel = 0; iChannel < 16; ++iChannel)
		channels << QString::number(iChannel + 1);

	m_ui.Def1ChannelKnob->insertItems(0, channels);
	m_ui.Def2ChannelKnob->insertItems(0, channels);

	// Mono switches
	m_ui.Def1MonoKnob->insertItems(0, states);
	m_ui.Def2MonoKnob->insertItems(0, states);

	// Output (stereo-)width limits.
	m_ui.Out1WidthKnob->setMinimum(-1.0f);
	m_ui.Out1WidthKnob->setMaximum(+1.0f);

	m_ui.Out2WidthKnob->setMinimum(-1.0f);
	m_ui.Out2WidthKnob->setMaximum(+1.0f);

	// Output (stereo-)panning limits.
	m_ui.Out1PanningKnob->setMinimum(-1.0f);
	m_ui.Out1PanningKnob->setMaximum(+1.0f);

	m_ui.Out2PanningKnob->setMinimum(-1.0f);
	m_ui.Out2PanningKnob->setMaximum(+1.0f);

	// Effects (delay BPM)
	m_ui.Del1BpmKnob->setScale(1.0f);
	m_ui.Del1BpmKnob->setMinimum(0.0f);
	m_ui.Del1BpmKnob->setMaximum(360.0f);
//	m_ui.Del1BpmKnob->setSingleStep(1.0f);

	// Reverb (stereo-)width limits.
	m_ui.Rev1WidthKnob->setMinimum(-1.0f);
	m_ui.Rev1WidthKnob->setMaximum(+1.0f);

	// DCO1
	setParamKnob(synthv1::DCO1_SHAPE1,  m_ui.Dco1Shape1Knob);
	setParamKnob(synthv1::DCO1_WIDTH1,  m_ui.Dco1Width1Knob);
	setParamKnob(synthv1::DCO1_BANDL1,  m_ui.Dco1Bandl1Knob);
	setParamKnob(synthv1::DCO1_SHAPE2,  m_ui.Dco1Shape2Knob);
	setParamKnob(synthv1::DCO1_WIDTH2,  m_ui.Dco1Width2Knob);
	setParamKnob(synthv1::DCO1_BANDL2,  m_ui.Dco1Bandl2Knob);
	setParamKnob(synthv1::DCO1_BALANCE, m_ui.Dco1BalanceKnob);
	setParamKnob(synthv1::DCO1_DETUNE,  m_ui.Dco1DetuneKnob);
	setParamKnob(synthv1::DCO1_PHASE,   m_ui.Dco1PhaseKnob);
	setParamKnob(synthv1::DCO1_RINGMOD, m_ui.Dco1RingModKnob);
	setParamKnob(synthv1::DCO1_OCTAVE,  m_ui.Dco1OctaveKnob);
	setParamKnob(synthv1::DCO1_TUNING,  m_ui.Dco1TuningKnob);
	setParamKnob(synthv1::DCO1_GLIDE,   m_ui.Dco1GlideKnob);
	setParamKnob(synthv1::DCO1_ENVTIME, m_ui.Dco1EnvTimeKnob);

	QObject::connect(
		m_ui.Dco1Shape1Knob, SIGNAL(valueChanged(float)),
		m_ui.Dco1Wave1, SLOT(setWaveShape(float)));
	QObject::connect(
		m_ui.Dco1Wave1, SIGNAL(waveShapeChanged(float)),
		m_ui.Dco1Shape1Knob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dco1Width1Knob, SIGNAL(valueChanged(float)),
		m_ui.Dco1Wave1, SLOT(setWaveWidth(float)));
	QObject::connect(
		m_ui.Dco1Wave1, SIGNAL(waveWidthChanged(float)),
		m_ui.Dco1Width1Knob, SLOT(setValue(float)));

	QObject::connect(
		m_ui.Dco1Shape2Knob, SIGNAL(valueChanged(float)),
		m_ui.Dco1Wave2, SLOT(setWaveShape(float)));
	QObject::connect(
		m_ui.Dco1Wave2, SIGNAL(waveShapeChanged(float)),
		m_ui.Dco1Shape2Knob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dco1Width2Knob, SIGNAL(valueChanged(float)),
		m_ui.Dco1Wave2, SLOT(setWaveWidth(float)));
	QObject::connect(
		m_ui.Dco1Wave2, SIGNAL(waveWidthChanged(float)),
		m_ui.Dco1Width2Knob, SLOT(setValue(float)));

	// DCF1
	setParamKnob(synthv1::DCF1_CUTOFF,   m_ui.Dcf1CutoffKnob);
	setParamKnob(synthv1::DCF1_RESO,     m_ui.Dcf1ResoKnob);
	setParamKnob(synthv1::DCF1_TYPE,     m_ui.Dcf1TypeKnob);
	setParamKnob(synthv1::DCF1_SLOPE,    m_ui.Dcf1SlopeKnob);
	setParamKnob(synthv1::DCF1_ENVELOPE, m_ui.Dcf1EnvelopeKnob);
	setParamKnob(synthv1::DCF1_ATTACK,   m_ui.Dcf1AttackKnob);
	setParamKnob(synthv1::DCF1_DECAY,    m_ui.Dcf1DecayKnob);
	setParamKnob(synthv1::DCF1_SUSTAIN,  m_ui.Dcf1SustainKnob);
	setParamKnob(synthv1::DCF1_RELEASE,  m_ui.Dcf1ReleaseKnob);

	QObject::connect(
		m_ui.Dcf1Filt, SIGNAL(cutoffChanged(float)),
		m_ui.Dcf1CutoffKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dcf1CutoffKnob, SIGNAL(valueChanged(float)),
		m_ui.Dcf1Filt, SLOT(setCutoff(float)));

	QObject::connect(
		m_ui.Dcf1Filt, SIGNAL(resoChanged(float)),
		m_ui.Dcf1ResoKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dcf1ResoKnob, SIGNAL(valueChanged(float)),
		m_ui.Dcf1Filt, SLOT(setReso(float)));

	QObject::connect(
		m_ui.Dcf1TypeKnob, SIGNAL(valueChanged(float)),
		m_ui.Dcf1Filt, SLOT(setType(float)));
	QObject::connect(
		m_ui.Dcf1SlopeKnob, SIGNAL(valueChanged(float)),
		m_ui.Dcf1Filt, SLOT(setSlope(float)));

	QObject::connect(
		m_ui.Dcf1Env, SIGNAL(attackChanged(float)),
		m_ui.Dcf1AttackKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dcf1AttackKnob, SIGNAL(valueChanged(float)),
		m_ui.Dcf1Env, SLOT(setAttack(float)));

	QObject::connect(
		m_ui.Dcf1Env, SIGNAL(decayChanged(float)),
		m_ui.Dcf1DecayKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dcf1DecayKnob, SIGNAL(valueChanged(float)),
		m_ui.Dcf1Env, SLOT(setDecay(float)));

	QObject::connect(
		m_ui.Dcf1Env, SIGNAL(sustainChanged(float)),
		m_ui.Dcf1SustainKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dcf1SustainKnob, SIGNAL(valueChanged(float)),
		m_ui.Dcf1Env, SLOT(setSustain(float)));

	QObject::connect(
		m_ui.Dcf1Env, SIGNAL(releaseChanged(float)),
		m_ui.Dcf1ReleaseKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dcf1ReleaseKnob, SIGNAL(valueChanged(float)),
		m_ui.Dcf1Env, SLOT(setRelease(float)));

	// LFO1
	setParamKnob(synthv1::LFO1_SHAPE,   m_ui.Lfo1ShapeKnob);
	setParamKnob(synthv1::LFO1_WIDTH,   m_ui.Lfo1WidthKnob);
	setParamKnob(synthv1::LFO1_BPM,     m_ui.Lfo1BpmKnob);
	setParamKnob(synthv1::LFO1_RATE,    m_ui.Lfo1RateKnob);
	setParamKnob(synthv1::LFO1_SYNC,    m_ui.Lfo1SyncKnob);
	setParamKnob(synthv1::LFO1_PANNING, m_ui.Lfo1PanningKnob);
	setParamKnob(synthv1::LFO1_VOLUME,  m_ui.Lfo1VolumeKnob);
	setParamKnob(synthv1::LFO1_CUTOFF,  m_ui.Lfo1CutoffKnob);
	setParamKnob(synthv1::LFO1_RESO,    m_ui.Lfo1ResoKnob);
	setParamKnob(synthv1::LFO1_PITCH,   m_ui.Lfo1PitchKnob);
	setParamKnob(synthv1::LFO1_RINGMOD, m_ui.Lfo1RingModKnob);
	setParamKnob(synthv1::LFO1_SWEEP,   m_ui.Lfo1SweepKnob);
	setParamKnob(synthv1::LFO1_ATTACK,  m_ui.Lfo1AttackKnob);
	setParamKnob(synthv1::LFO1_DECAY,   m_ui.Lfo1DecayKnob);
	setParamKnob(synthv1::LFO1_SUSTAIN, m_ui.Lfo1SustainKnob);
	setParamKnob(synthv1::LFO1_RELEASE, m_ui.Lfo1ReleaseKnob);

	QObject::connect(
		m_ui.Lfo1ShapeKnob, SIGNAL(valueChanged(float)),
		m_ui.Lfo1Wave, SLOT(setWaveShape(float)));
	QObject::connect(
		m_ui.Lfo1Wave, SIGNAL(waveShapeChanged(float)),
		m_ui.Lfo1ShapeKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Lfo1WidthKnob, SIGNAL(valueChanged(float)),
		m_ui.Lfo1Wave, SLOT(setWaveWidth(float)));
	QObject::connect(
		m_ui.Lfo1Wave, SIGNAL(waveWidthChanged(float)),
		m_ui.Lfo1WidthKnob, SLOT(setValue(float)));

	QObject::connect(
		m_ui.Lfo1Env, SIGNAL(attackChanged(float)),
		m_ui.Lfo1AttackKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Lfo1AttackKnob, SIGNAL(valueChanged(float)),
		m_ui.Lfo1Env, SLOT(setAttack(float)));

	QObject::connect(
		m_ui.Lfo1Env, SIGNAL(decayChanged(float)),
		m_ui.Lfo1DecayKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Lfo1DecayKnob, SIGNAL(valueChanged(float)),
		m_ui.Lfo1Env, SLOT(setDecay(float)));

	QObject::connect(
		m_ui.Lfo1Env, SIGNAL(sustainChanged(float)),
		m_ui.Lfo1SustainKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Lfo1SustainKnob, SIGNAL(valueChanged(float)),
		m_ui.Lfo1Env, SLOT(setSustain(float)));

	QObject::connect(
		m_ui.Lfo1Env, SIGNAL(releaseChanged(float)),
		m_ui.Lfo1ReleaseKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Lfo1ReleaseKnob, SIGNAL(valueChanged(float)),
		m_ui.Lfo1Env, SLOT(setRelease(float)));

	// DCA1
	setParamKnob(synthv1::DCA1_VOLUME,  m_ui.Dca1VolumeKnob);
	setParamKnob(synthv1::DCA1_ATTACK,  m_ui.Dca1AttackKnob);
	setParamKnob(synthv1::DCA1_DECAY,   m_ui.Dca1DecayKnob);
	setParamKnob(synthv1::DCA1_SUSTAIN, m_ui.Dca1SustainKnob);
	setParamKnob(synthv1::DCA1_RELEASE, m_ui.Dca1ReleaseKnob);

	QObject::connect(
		m_ui.Dca1Env, SIGNAL(attackChanged(float)),
		m_ui.Dca1AttackKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dca1AttackKnob, SIGNAL(valueChanged(float)),
		m_ui.Dca1Env, SLOT(setAttack(float)));

	QObject::connect(
		m_ui.Dca1Env, SIGNAL(decayChanged(float)),
		m_ui.Dca1DecayKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dca1DecayKnob, SIGNAL(valueChanged(float)),
		m_ui.Dca1Env, SLOT(setDecay(float)));

	QObject::connect(
		m_ui.Dca1Env, SIGNAL(sustainChanged(float)),
		m_ui.Dca1SustainKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dca1SustainKnob, SIGNAL(valueChanged(float)),
		m_ui.Dca1Env, SLOT(setSustain(float)));

	QObject::connect(
		m_ui.Dca1Env, SIGNAL(releaseChanged(float)),
		m_ui.Dca1ReleaseKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dca1ReleaseKnob, SIGNAL(valueChanged(float)),
		m_ui.Dca1Env, SLOT(setRelease(float)));

	// DEF1
	setParamKnob(synthv1::DEF1_PITCHBEND, m_ui.Def1PitchbendKnob);
	setParamKnob(synthv1::DEF1_MODWHEEL,  m_ui.Def1ModwheelKnob);
	setParamKnob(synthv1::DEF1_PRESSURE,  m_ui.Def1PressureKnob);
	setParamKnob(synthv1::DEF1_VELOCITY,  m_ui.Def1VelocityKnob);
	setParamKnob(synthv1::DEF1_CHANNEL,   m_ui.Def1ChannelKnob);
	setParamKnob(synthv1::DEF1_MONO,      m_ui.Def1MonoKnob);

	// OUT1
	setParamKnob(synthv1::OUT1_WIDTH,   m_ui.Out1WidthKnob);
	setParamKnob(synthv1::OUT1_PANNING, m_ui.Out1PanningKnob);
	setParamKnob(synthv1::OUT1_FXSEND,  m_ui.Out1FxSendKnob);
	setParamKnob(synthv1::OUT1_VOLUME,  m_ui.Out1VolumeKnob);


	// DCO2
	setParamKnob(synthv1::DCO2_SHAPE1,  m_ui.Dco2Shape1Knob);
	setParamKnob(synthv1::DCO2_WIDTH1,  m_ui.Dco2Width1Knob);
	setParamKnob(synthv1::DCO2_BANDL1,  m_ui.Dco2Bandl1Knob);
	setParamKnob(synthv1::DCO2_SHAPE2,  m_ui.Dco2Shape2Knob);
	setParamKnob(synthv1::DCO2_WIDTH2,  m_ui.Dco2Width2Knob);
	setParamKnob(synthv1::DCO2_BANDL2,  m_ui.Dco2Bandl2Knob);
	setParamKnob(synthv1::DCO2_BALANCE, m_ui.Dco2BalanceKnob);
	setParamKnob(synthv1::DCO2_DETUNE,  m_ui.Dco2DetuneKnob);
	setParamKnob(synthv1::DCO2_PHASE,   m_ui.Dco2PhaseKnob);
	setParamKnob(synthv1::DCO2_RINGMOD, m_ui.Dco2RingModKnob);
	setParamKnob(synthv1::DCO2_OCTAVE,  m_ui.Dco2OctaveKnob);
	setParamKnob(synthv1::DCO2_TUNING,  m_ui.Dco2TuningKnob);
	setParamKnob(synthv1::DCO2_GLIDE,   m_ui.Dco2GlideKnob);
	setParamKnob(synthv1::DCO2_ENVTIME, m_ui.Dco2EnvTimeKnob);

	QObject::connect(
		m_ui.Dco2Shape1Knob, SIGNAL(valueChanged(float)),
		m_ui.Dco2Wave1, SLOT(setWaveShape(float)));
	QObject::connect(
		m_ui.Dco2Wave1, SIGNAL(waveShapeChanged(float)),
		m_ui.Dco2Shape1Knob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dco2Width1Knob, SIGNAL(valueChanged(float)),
		m_ui.Dco2Wave1, SLOT(setWaveWidth(float)));
	QObject::connect(
		m_ui.Dco2Wave1, SIGNAL(waveWidthChanged(float)),
		m_ui.Dco2Width1Knob, SLOT(setValue(float)));

	QObject::connect(
		m_ui.Dco2Shape2Knob, SIGNAL(valueChanged(float)),
		m_ui.Dco2Wave2, SLOT(setWaveShape(float)));
	QObject::connect(
		m_ui.Dco2Wave2, SIGNAL(waveShapeChanged(float)),
		m_ui.Dco2Shape2Knob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dco2Width2Knob, SIGNAL(valueChanged(float)),
		m_ui.Dco2Wave2, SLOT(setWaveWidth(float)));
	QObject::connect(
		m_ui.Dco2Wave2, SIGNAL(waveWidthChanged(float)),
		m_ui.Dco2Width2Knob, SLOT(setValue(float)));

	// DCF2
	setParamKnob(synthv1::DCF2_CUTOFF,   m_ui.Dcf2CutoffKnob);
	setParamKnob(synthv1::DCF2_RESO,     m_ui.Dcf2ResoKnob);
	setParamKnob(synthv1::DCF2_TYPE,     m_ui.Dcf2TypeKnob);
	setParamKnob(synthv1::DCF2_SLOPE,    m_ui.Dcf2SlopeKnob);
	setParamKnob(synthv1::DCF2_ENVELOPE, m_ui.Dcf2EnvelopeKnob);
	setParamKnob(synthv1::DCF2_ATTACK,   m_ui.Dcf2AttackKnob);
	setParamKnob(synthv1::DCF2_DECAY,    m_ui.Dcf2DecayKnob);
	setParamKnob(synthv1::DCF2_SUSTAIN,  m_ui.Dcf2SustainKnob);
	setParamKnob(synthv1::DCF2_RELEASE,  m_ui.Dcf2ReleaseKnob);

	QObject::connect(
		m_ui.Dcf2Filt, SIGNAL(cutoffChanged(float)),
		m_ui.Dcf2CutoffKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dcf2CutoffKnob, SIGNAL(valueChanged(float)),
		m_ui.Dcf2Filt, SLOT(setCutoff(float)));

	QObject::connect(
		m_ui.Dcf2Filt, SIGNAL(resoChanged(float)),
		m_ui.Dcf2ResoKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dcf2ResoKnob, SIGNAL(valueChanged(float)),
		m_ui.Dcf2Filt, SLOT(setReso(float)));

	QObject::connect(
		m_ui.Dcf2TypeKnob, SIGNAL(valueChanged(float)),
		m_ui.Dcf2Filt, SLOT(setType(float)));
	QObject::connect(
		m_ui.Dcf2SlopeKnob, SIGNAL(valueChanged(float)),
		m_ui.Dcf2Filt, SLOT(setSlope(float)));

	QObject::connect(
		m_ui.Dcf2Env, SIGNAL(attackChanged(float)),
		m_ui.Dcf2AttackKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dcf2AttackKnob, SIGNAL(valueChanged(float)),
		m_ui.Dcf2Env, SLOT(setAttack(float)));

	QObject::connect(
		m_ui.Dcf2Env, SIGNAL(decayChanged(float)),
		m_ui.Dcf2DecayKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dcf2DecayKnob, SIGNAL(valueChanged(float)),
		m_ui.Dcf2Env, SLOT(setDecay(float)));

	QObject::connect(
		m_ui.Dcf2Env, SIGNAL(sustainChanged(float)),
		m_ui.Dcf2SustainKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dcf2SustainKnob, SIGNAL(valueChanged(float)),
		m_ui.Dcf2Env, SLOT(setSustain(float)));

	QObject::connect(
		m_ui.Dcf2Env, SIGNAL(releaseChanged(float)),
		m_ui.Dcf2ReleaseKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dcf2ReleaseKnob, SIGNAL(valueChanged(float)),
		m_ui.Dcf2Env, SLOT(setRelease(float)));

	// LFO2
	setParamKnob(synthv1::LFO2_SHAPE,   m_ui.Lfo2ShapeKnob);
	setParamKnob(synthv1::LFO2_WIDTH,   m_ui.Lfo2WidthKnob);
	setParamKnob(synthv1::LFO2_BPM,     m_ui.Lfo2BpmKnob);
	setParamKnob(synthv1::LFO2_RATE,    m_ui.Lfo2RateKnob);
	setParamKnob(synthv1::LFO2_SYNC,    m_ui.Lfo2SyncKnob);
	setParamKnob(synthv1::LFO2_PANNING, m_ui.Lfo2PanningKnob);
	setParamKnob(synthv1::LFO2_VOLUME,  m_ui.Lfo2VolumeKnob);
	setParamKnob(synthv1::LFO2_CUTOFF,  m_ui.Lfo2CutoffKnob);
	setParamKnob(synthv1::LFO2_RESO,    m_ui.Lfo2ResoKnob);
	setParamKnob(synthv1::LFO2_PITCH,   m_ui.Lfo2PitchKnob);
	setParamKnob(synthv1::LFO2_RINGMOD, m_ui.Lfo2RingModKnob);
	setParamKnob(synthv1::LFO2_SWEEP,   m_ui.Lfo2SweepKnob);
	setParamKnob(synthv1::LFO2_ATTACK,  m_ui.Lfo2AttackKnob);
	setParamKnob(synthv1::LFO2_DECAY,   m_ui.Lfo2DecayKnob);
	setParamKnob(synthv1::LFO2_SUSTAIN, m_ui.Lfo2SustainKnob);
	setParamKnob(synthv1::LFO2_RELEASE, m_ui.Lfo2ReleaseKnob);

	QObject::connect(
		m_ui.Lfo2ShapeKnob, SIGNAL(valueChanged(float)),
		m_ui.Lfo2Wave, SLOT(setWaveShape(float)));
	QObject::connect(
		m_ui.Lfo2Wave, SIGNAL(waveShapeChanged(float)),
		m_ui.Lfo2ShapeKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Lfo2WidthKnob, SIGNAL(valueChanged(float)),
		m_ui.Lfo2Wave, SLOT(setWaveWidth(float)));
	QObject::connect(
		m_ui.Lfo2Wave, SIGNAL(waveWidthChanged(float)),
		m_ui.Lfo2WidthKnob, SLOT(setValue(float)));

	QObject::connect(
		m_ui.Lfo2Env, SIGNAL(attackChanged(float)),
		m_ui.Lfo2AttackKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Lfo2AttackKnob, SIGNAL(valueChanged(float)),
		m_ui.Lfo2Env, SLOT(setAttack(float)));

	QObject::connect(
		m_ui.Lfo2Env, SIGNAL(decayChanged(float)),
		m_ui.Lfo2DecayKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Lfo2DecayKnob, SIGNAL(valueChanged(float)),
		m_ui.Lfo2Env, SLOT(setDecay(float)));

	QObject::connect(
		m_ui.Lfo2Env, SIGNAL(sustainChanged(float)),
		m_ui.Lfo2SustainKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Lfo2SustainKnob, SIGNAL(valueChanged(float)),
		m_ui.Lfo2Env, SLOT(setSustain(float)));

	QObject::connect(
		m_ui.Lfo2Env, SIGNAL(releaseChanged(float)),
		m_ui.Lfo2ReleaseKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Lfo2ReleaseKnob, SIGNAL(valueChanged(float)),
		m_ui.Lfo2Env, SLOT(setRelease(float)));

	// DCA2
	setParamKnob(synthv1::DCA2_VOLUME,  m_ui.Dca2VolumeKnob);
	setParamKnob(synthv1::DCA2_ATTACK,  m_ui.Dca2AttackKnob);
	setParamKnob(synthv1::DCA2_DECAY,   m_ui.Dca2DecayKnob);
	setParamKnob(synthv1::DCA2_SUSTAIN, m_ui.Dca2SustainKnob);
	setParamKnob(synthv1::DCA2_RELEASE, m_ui.Dca2ReleaseKnob);

	QObject::connect(
		m_ui.Dca2Env, SIGNAL(attackChanged(float)),
		m_ui.Dca2AttackKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dca2AttackKnob, SIGNAL(valueChanged(float)),
		m_ui.Dca2Env, SLOT(setAttack(float)));

	QObject::connect(
		m_ui.Dca2Env, SIGNAL(decayChanged(float)),
		m_ui.Dca2DecayKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dca2DecayKnob, SIGNAL(valueChanged(float)),
		m_ui.Dca2Env, SLOT(setDecay(float)));

	QObject::connect(
		m_ui.Dca2Env, SIGNAL(sustainChanged(float)),
		m_ui.Dca2SustainKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dca2SustainKnob, SIGNAL(valueChanged(float)),
		m_ui.Dca2Env, SLOT(setSustain(float)));

	QObject::connect(
		m_ui.Dca2Env, SIGNAL(releaseChanged(float)),
		m_ui.Dca2ReleaseKnob, SLOT(setValue(float)));
	QObject::connect(
		m_ui.Dca2ReleaseKnob, SIGNAL(valueChanged(float)),
		m_ui.Dca2Env, SLOT(setRelease(float)));

	// DEF2
	setParamKnob(synthv1::DEF2_PITCHBEND, m_ui.Def2PitchbendKnob);
	setParamKnob(synthv1::DEF2_MODWHEEL,  m_ui.Def2ModwheelKnob);
	setParamKnob(synthv1::DEF2_PRESSURE,  m_ui.Def2PressureKnob);
	setParamKnob(synthv1::DEF2_VELOCITY,  m_ui.Def2VelocityKnob);
	setParamKnob(synthv1::DEF2_CHANNEL,   m_ui.Def2ChannelKnob);
	setParamKnob(synthv1::DEF2_MONO,      m_ui.Def2MonoKnob);

	// OUT2
	setParamKnob(synthv1::OUT2_WIDTH,   m_ui.Out2WidthKnob);
	setParamKnob(synthv1::OUT2_PANNING, m_ui.Out2PanningKnob);
	setParamKnob(synthv1::OUT2_FXSEND,  m_ui.Out2FxSendKnob);
	setParamKnob(synthv1::OUT2_VOLUME,  m_ui.Out2VolumeKnob);


	// Effects
	setParamKnob(synthv1::CHO1_WET,   m_ui.Cho1WetKnob);
	setParamKnob(synthv1::CHO1_DELAY, m_ui.Cho1DelayKnob);
	setParamKnob(synthv1::CHO1_FEEDB, m_ui.Cho1FeedbKnob);
	setParamKnob(synthv1::CHO1_RATE,  m_ui.Cho1RateKnob);
	setParamKnob(synthv1::CHO1_MOD,   m_ui.Cho1ModKnob);

	setParamKnob(synthv1::FLA1_WET,   m_ui.Fla1WetKnob);
	setParamKnob(synthv1::FLA1_DELAY, m_ui.Fla1DelayKnob);
	setParamKnob(synthv1::FLA1_FEEDB, m_ui.Fla1FeedbKnob);
	setParamKnob(synthv1::FLA1_DAFT,  m_ui.Fla1DaftKnob);

	setParamKnob(synthv1::PHA1_WET,   m_ui.Pha1WetKnob);
	setParamKnob(synthv1::PHA1_RATE,  m_ui.Pha1RateKnob);
	setParamKnob(synthv1::PHA1_FEEDB, m_ui.Pha1FeedbKnob);
	setParamKnob(synthv1::PHA1_DEPTH, m_ui.Pha1DepthKnob);
	setParamKnob(synthv1::PHA1_DAFT,  m_ui.Pha1DaftKnob);

	setParamKnob(synthv1::DEL1_WET,   m_ui.Del1WetKnob);
	setParamKnob(synthv1::DEL1_DELAY, m_ui.Del1DelayKnob);
	setParamKnob(synthv1::DEL1_FEEDB, m_ui.Del1FeedbKnob);
	setParamKnob(synthv1::DEL1_BPM,   m_ui.Del1BpmKnob);

	// Reverb
	setParamKnob(synthv1::REV1_WET,   m_ui.Rev1WetKnob);
	setParamKnob(synthv1::REV1_ROOM,  m_ui.Rev1RoomKnob);
	setParamKnob(synthv1::REV1_DAMP,  m_ui.Rev1DampKnob);
	setParamKnob(synthv1::REV1_FEEDB, m_ui.Rev1FeedbKnob);
	setParamKnob(synthv1::REV1_WIDTH, m_ui.Rev1WidthKnob);

	// Dynamics
	setParamKnob(synthv1::DYN1_COMPRESS, m_ui.Dyn1CompressKnob);
	setParamKnob(synthv1::DYN1_LIMITER,  m_ui.Dyn1LimiterKnob);


	// Preset management
	QObject::connect(m_ui.Preset,
		SIGNAL(newPresetFile()),
		SLOT(newPreset()));
	QObject::connect(m_ui.Preset,
		SIGNAL(loadPresetFile(const QString&)),
		SLOT(loadPreset(const QString&)));
	QObject::connect(m_ui.Preset,
		SIGNAL(savePresetFile(const QString&)),
		SLOT(savePreset(const QString&)));
	QObject::connect(m_ui.Preset,
		SIGNAL(resetPresetFile()),
		SLOT(resetParams()));


	// Swap params A/B
	QObject::connect(m_ui.SwapParamsAButton,
		SIGNAL(toggled(bool)),
		SLOT(swapParams(bool)));
	QObject::connect(m_ui.SwapParamsBButton,
		SIGNAL(toggled(bool)),
		SLOT(swapParams(bool)));

	// Direct stacked-page signal/slot
	QObject::connect(m_ui.TabBar, SIGNAL(currentChanged(int)),
		m_ui.StackedWidget, SLOT(setCurrentIndex(int)));

	// Menu actions
	QObject::connect(m_ui.helpConfigureAction,
		SIGNAL(triggered(bool)),
		SLOT(helpConfigure()));
	QObject::connect(m_ui.helpAboutAction,
		SIGNAL(triggered(bool)),
		SLOT(helpAbout()));
	QObject::connect(m_ui.helpAboutQtAction,
		SIGNAL(triggered(bool)),
		SLOT(helpAboutQt()));

	// General knob/dial  behavior init...
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig) {
		synthv1widget_dial::setDialMode(
			synthv1widget_dial::DialMode(pConfig->iKnobDialMode));
	}

	// Epilog.
	// QWidget::adjustSize();

	m_ui.StatusBar->showMessage(tr("Ready"), 5000);
	m_ui.StatusBar->modified(false);
	m_ui.Preset->setDirtyPreset(false);
}


// Destructor.
synthv1widget::~synthv1widget (void)
{
	if (m_sched_notifier)
		delete m_sched_notifier;
}


// Open/close the scheduler/work notifier.
void synthv1widget::openSchedNotifier (void)
{
	if (m_sched_notifier)
		return;

	synthv1_ui *pSynthUi = ui_instance();
	if (pSynthUi == NULL)
		return;

	m_sched_notifier = new synthv1widget_sched(pSynthUi->instance(), this);

	QObject::connect(m_sched_notifier,
		SIGNAL(notify(int, int)),
		SLOT(updateSchedNotify(int, int)));

	pSynthUi->midiInEnabled(true);
}


void synthv1widget::closeSchedNotifier (void)
{
	if (m_sched_notifier) {
		delete m_sched_notifier;
		m_sched_notifier = NULL;
	}

	synthv1_ui *pSynthUi = ui_instance();
	if (pSynthUi)
		pSynthUi->midiInEnabled(false);
}


// Show/hide widget handlers.
void synthv1widget::showEvent ( QShowEvent *pShowEvent )
{
	QWidget::showEvent(pShowEvent);

	openSchedNotifier();
}


void synthv1widget::hideEvent ( QHideEvent *pHideEvent )
{
	closeSchedNotifier();

	QWidget::hideEvent(pHideEvent);
}


// Param kbob (widget) map accesors.
void synthv1widget::setParamKnob ( synthv1::ParamIndex index, synthv1widget_knob *pKnob )
{
	pKnob->setDefaultValue(synthv1_param::paramDefaultValue(index));

	m_paramKnobs.insert(index, pKnob);
	m_knobParams.insert(pKnob, index);

	QObject::connect(pKnob,
		SIGNAL(valueChanged(float)),
		SLOT(paramChanged(float)));

	pKnob->setContextMenuPolicy(Qt::CustomContextMenu);

	QObject::connect(pKnob,
		SIGNAL(customContextMenuRequested(const QPoint&)),
		SLOT(paramContextMenu(const QPoint&)));
}

synthv1widget_knob *synthv1widget::paramKnob ( synthv1::ParamIndex index ) const
{
	return m_paramKnobs.value(index, NULL);
}


// Param port accessors.
void synthv1widget::setParamValue (
	synthv1::ParamIndex index, float fValue, bool bDefault )
{
	++m_iUpdate;

	synthv1widget_knob *pKnob = paramKnob(index);
	if (pKnob)
		pKnob->setValue(fValue, bDefault);

	updateParamEx(index, fValue);

	--m_iUpdate;
}

float synthv1widget::paramValue ( synthv1::ParamIndex index ) const
{
	float fValue = 0.0f;

	synthv1widget_knob *pKnob = paramKnob(index);
	if (pKnob) {
		fValue = pKnob->value();
	} else {
		synthv1_ui *pSynthUi = ui_instance();
		if (pSynthUi)
			fValue = pSynthUi->paramValue(index);
	}

	return fValue;
}


// Param knob (widget) slot.
void synthv1widget::paramChanged ( float fValue )
{
	if (m_iUpdate > 0)
		return;

	synthv1widget_knob *pKnob = qobject_cast<synthv1widget_knob *> (sender());
	if (pKnob) {
		const synthv1::ParamIndex index = m_knobParams.value(pKnob);
		updateParam(index, fValue);
		updateParamEx(index, fValue);
		m_ui.StatusBar->showMessage(QString("%1: %2")
			.arg(pKnob->toolTip())
			.arg(pKnob->valueText()), 5000);
		updateDirtyPreset(true);
	}
}


// Update local tied widgets.
void synthv1widget::updateParamEx ( synthv1::ParamIndex index, float fValue )
{
	++m_iUpdate;

	switch (index) {
	case synthv1::DCO1_SHAPE1:
		m_ui.Dco1Bandl1Knob->setEnabled(
			synthv1_wave::Shape(int(fValue)) != synthv1_wave::Noise);
		break;
	case synthv1::DCO1_SHAPE2:
		m_ui.Dco1Bandl2Knob->setEnabled(
			synthv1_wave::Shape(int(fValue)) != synthv1_wave::Noise);
		break;
	case synthv1::DCO2_SHAPE1:
		m_ui.Dco2Bandl1Knob->setEnabled(
			synthv1_wave::Shape(int(fValue)) != synthv1_wave::Noise);
		break;
	case synthv1::DCO2_SHAPE2:
		m_ui.Dco2Bandl2Knob->setEnabled(
			synthv1_wave::Shape(int(fValue)) != synthv1_wave::Noise);
		break;
	case synthv1::DCF1_SLOPE:
		m_ui.Dcf1TypeKnob->setEnabled(int(fValue) != 3); // !Formant
		break;
	case synthv1::DCF2_SLOPE:
		m_ui.Dcf2TypeKnob->setEnabled(int(fValue) != 3); // !Formant
		// Fall thru...
	default:
		break;
	}

	--m_iUpdate;
}


// Update scheduled controllers param/knob widgets.
void synthv1widget::updateSchedParam ( synthv1::ParamIndex index, float fValue )
{
	++m_iUpdate;

	synthv1widget_knob *pKnob = paramKnob(index);
	if (pKnob) {
		pKnob->setValue(fValue, false);
		updateParam(index, fValue);
		updateParamEx(index, fValue);
		m_ui.StatusBar->showMessage(QString("%1: %2")
			.arg(pKnob->toolTip())
			.arg(pKnob->valueText()), 5000);
		updateDirtyPreset(true);
	}

	--m_iUpdate;
}


// Reset all param knobs to default values.
void synthv1widget::resetParams (void)
{
	synthv1_ui *pSynthUi = ui_instance();
	if (pSynthUi == NULL)
		return;

	pSynthUi->reset();

	resetSwapParams();

	for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i) {
		const synthv1::ParamIndex index = synthv1::ParamIndex(i);
		float fValue = synthv1_param::paramDefaultValue(index);
		synthv1widget_knob *pKnob = paramKnob(index);
		if (pKnob && pKnob->isDefaultValue())
			fValue = pKnob->defaultValue();
		setParamValue(index, fValue);
		updateParam(index, fValue);
		m_params_ab[i] = fValue;
	}

	m_ui.StatusBar->showMessage(tr("Reset preset"), 5000);
	updateDirtyPreset(false);
}


// Swap params A/B.
void synthv1widget::swapParams ( bool bOn )
{
	if (m_iUpdate > 0 || !bOn)
		return;

#ifdef CONFIG_DEBUG
	qDebug("synthv1widget::swapParams(%d)", int(bOn));
#endif
//	resetParamKnobs();

	for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i) {
		const synthv1::ParamIndex index = synthv1::ParamIndex(i);
		synthv1widget_knob *pKnob = paramKnob(index);
		if (pKnob) {
			const float fOldValue = pKnob->value();
			const float fNewValue = m_params_ab[i];
			setParamValue(index, fNewValue);
			updateParam(index, fNewValue);
			m_params_ab[i] = fOldValue;
		}
	}

	const bool bSwapA = m_ui.SwapParamsAButton->isChecked();
	m_ui.StatusBar->showMessage(tr("Swap %1").arg(bSwapA ? 'A' : 'B'), 5000);
	updateDirtyPreset(true);
}


// Reset swap params A/B group.
void synthv1widget::resetSwapParams (void)
{
	++m_iUpdate;
	m_ui.SwapParamsAButton->setChecked(true);
	--m_iUpdate;
}


// Initialize param values.
void synthv1widget::updateParamValues (void)
{
	resetSwapParams();

	synthv1_ui *pSynthUi = ui_instance();

	for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i) {
		const synthv1::ParamIndex index = synthv1::ParamIndex(i);
		const float fValue = (pSynthUi
			? pSynthUi->paramValue(index)
			: synthv1_param::paramDefaultValue(index));
		setParamValue(index, fValue, true);
		updateParam(index, fValue);
	//	updateParamEx(index, fValue);
		m_params_ab[i] = fValue;
	}
}


// Reset all param default values.
void synthv1widget::resetParamValues (void)
{
	resetSwapParams();

	for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i) {
		const synthv1::ParamIndex index = synthv1::ParamIndex(i);
		const float fValue = synthv1_param::paramDefaultValue(index);
		setParamValue(index, fValue, true);
		updateParam(index, fValue);
	//	updateParamEx(index, fValue);
		m_params_ab[i] = fValue;
	}
}


// Reset all knob default values.
void synthv1widget::resetParamKnobs (void)
{
	for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i) {
		synthv1widget_knob *pKnob = paramKnob(synthv1::ParamIndex(i));
		if (pKnob)
			pKnob->resetDefaultValue();
	}
}


// Preset init.
void synthv1widget::initPreset (void)
{
	m_ui.Preset->initPreset();
}


// Preset clear.
void synthv1widget::clearPreset (void)
{
	m_ui.Preset->clearPreset();
}


// Preset renewal.
void synthv1widget::newPreset (void)
{
#ifdef CONFIG_DEBUG
	qDebug("synthv1widget::newPreset()");
#endif

	resetParamKnobs();
	resetParamValues();

	m_ui.StatusBar->showMessage(tr("New preset"), 5000);
	updateDirtyPreset(false);
}


// Preset file I/O slots.
void synthv1widget::loadPreset ( const QString& sFilename )
{
#ifdef CONFIG_DEBUG
	qDebug("synthv1widget::loadPreset(\"%s\")", sFilename.toUtf8().constData());
#endif

	resetParamKnobs();
	resetParamValues();

	synthv1_ui *pSynthUi = ui_instance();
	if (pSynthUi)
		synthv1_param::loadPreset(pSynthUi->instance(), sFilename);

	updateLoadPreset(QFileInfo(sFilename).completeBaseName());
}


void synthv1widget::savePreset ( const QString& sFilename )
{
#ifdef CONFIG_DEBUG
	qDebug("synthv1widget::savePreset(\"%s\")", sFilename.toUtf8().constData());
#endif

	synthv1_ui *pSynthUi = ui_instance();
	if (pSynthUi)
		synthv1_param::savePreset(pSynthUi->instance(), sFilename);

	const QString& sPreset
		= QFileInfo(sFilename).completeBaseName();

	m_ui.StatusBar->showMessage(tr("Save preset: %1").arg(sPreset), 5000);
	updateDirtyPreset(false);
}


// Dirty close prompt,
bool synthv1widget::queryClose (void)
{
	return m_ui.Preset->queryPreset();
}


// Preset status updater.
void synthv1widget::updateLoadPreset ( const QString& sPreset )
{
	updateParamValues();

	m_ui.Preset->setPreset(sPreset);
	m_ui.StatusBar->showMessage(tr("Load preset: %1").arg(sPreset), 5000);
	updateDirtyPreset(false);
}


// Notification updater.
void synthv1widget::updateSchedNotify ( int stype, int sid )
{
	synthv1_ui *pSynthUi = ui_instance();
	if (pSynthUi == NULL)
		return;

#ifdef CONFIG_DEBUG_0
	qDebug("samplv1widget::updateSchedNotify(%d, %d)", stype, sid);
#endif

	switch (synthv1_sched::Type(stype)) {
	case synthv1_sched::MidiIn:
		if (pSynthUi->midiInCount() > 0) {
			m_ui.StatusBar->midiInLed(true);
			QTimer::singleShot(200, this, SLOT(midiInLedTimeout()));
		}
		break;
	case synthv1_sched::Controller: {
		synthv1widget_control *pInstance
			= synthv1widget_control::getInstance();
		if (pInstance) {
			synthv1_controls *pControls = pSynthUi->controls();
			pInstance->setControlKey(pControls->current_key());
		}
		break;
	}
	case synthv1_sched::Controls: {
		const synthv1::ParamIndex index = synthv1::ParamIndex(sid);
		updateSchedParam(index, pSynthUi->paramValue(index));
		break;
	}
	case synthv1_sched::Programs: {
		synthv1_programs *pPrograms = pSynthUi->programs();
		synthv1_programs::Prog *pProg = pPrograms->current_prog();
		if (pProg) updateLoadPreset(pProg->name());
		break;
	}
	case synthv1_sched::Wave:
	default:
		break;
	}
}


// MIDI In LED timeout.
void synthv1widget::midiInLedTimeout (void)
{
	m_ui.StatusBar->midiInLed(false);
}


// Menu actions.
void synthv1widget::helpConfigure (void)
{
	synthv1_ui *pSynthUi = ui_instance();
	if (pSynthUi == NULL)
		return;

	synthv1widget_config form(this);

	// Set controllers&&programs database...
	form.setControls(pSynthUi->controls());
	form.setPrograms(pSynthUi->programs());

	form.exec();
}


void synthv1widget::helpAbout (void)
{
	// About...
	QStringList list;
#ifdef CONFIG_DEBUG
	list << tr("Debugging option enabled.");
#endif
#ifndef CONFIG_JACK
	list << tr("JACK stand-alone build disabled.");
#endif
#ifndef CONFIG_JACK_SESSION
	list << tr("JACK session support disabled.");
#endif
#ifndef CONFIG_JACK_MIDI
	list << tr("JACK MIDI support disabled.");
#endif
#ifndef CONFIG_ALSA_MIDI
	list << tr("ALSA MIDI support disabled.");
#endif
#ifndef CONFIG_LV2
	list << tr("LV2 plug-in build disabled.");
#endif

	QString sText = "<p>\n";
	sText += "<b>" SYNTHV1_TITLE "</b> - " + tr(SYNTHV1_SUBTITLE) + "<br />\n";
	sText += "<br />\n";
	sText += tr("Version") + ": <b>" CONFIG_BUILD_VERSION "</b><br />\n";
//	sText += "<small>" + tr("Build") + ": " CONFIG_BUILD_DATE "</small><br />\n";
	if (!list.isEmpty()) {
		sText += "<small><font color=\"red\">";
		sText += list.join("<br />\n");
		sText += "</font></small><br />\n";
	}
	sText += "<br />\n";
	sText += tr("Website") + ": <a href=\"" SYNTHV1_WEBSITE "\">" SYNTHV1_WEBSITE "</a><br />\n";
	sText += "<br />\n";
	sText += "<small>";
	sText += SYNTHV1_COPYRIGHT "<br />\n";
	sText += "<br />\n";
	sText += tr("This program is free software; you can redistribute it and/or modify it") + "<br />\n";
	sText += tr("under the terms of the GNU General Public License version 2 or later.");
	sText += "</small>";
	sText += "</p>\n";

	QMessageBox::about(this, tr("About") + " " SYNTHV1_TITLE, sText);
}


void synthv1widget::helpAboutQt (void)
{
	// About Qt...
	QMessageBox::aboutQt(this);
}


// Dirty flag (overridable virtual) methods.
void synthv1widget::updateDirtyPreset ( bool bDirtyPreset )
{
	synthv1_ui *pSynthUi = ui_instance();
	if (pSynthUi)
		pSynthUi->updatePreset(bDirtyPreset);

	m_ui.StatusBar->modified(bDirtyPreset);
	m_ui.Preset->setDirtyPreset(bDirtyPreset);
}


// Param knob context menu.
void synthv1widget::paramContextMenu ( const QPoint& pos )
{
	synthv1widget_knob *pKnob
		= qobject_cast<synthv1widget_knob *> (sender());
	if (pKnob == NULL)
		return;

	synthv1_ui *pSynthUi = ui_instance();
	if (pSynthUi == NULL)
		return;

	synthv1_controls *pControls = pSynthUi->controls();
	if (pControls == NULL)
		return;

	if (!pControls->enabled())
		return;

	QMenu menu(this);

	QAction *pAction = menu.addAction(
		QIcon(":/images/synthv1_control.png"),
		tr("MIDI &Controller..."));

	if (menu.exec(pKnob->mapToGlobal(pos)) == pAction) {
		const synthv1::ParamIndex index = m_knobParams.value(pKnob);
		const QString& sTitle = pKnob->toolTip();
		synthv1widget_control::showInstance(pControls, index, sTitle, this);
	}
}


// end of synthv1widget.cpp
