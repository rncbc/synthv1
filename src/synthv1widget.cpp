// synthv1widget.cpp
//
/****************************************************************************
   Copyright (C) 2012, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include <QDomDocument>
#include <QTextStream>
#include <QFileInfo>

#include <QMessageBox>
#include <QDir>


//-------------------------------------------------------------------------
// default state (params)

static
struct {

	const char *name;
	float value;

} synthv1_default_params[synthv1::NUM_PARAMS] = {

	{ "DCO1_SHAPE1",    1.0f },
	{ "DCO1_WIDTH1",    1.0f },
	{ "DCO1_SHAPE2",    1.0f },
	{ "DCO1_WIDTH2",    1.0f },
	{ "DCO1_BALANCE",   0.0f },
	{ "DCO1_DETUNE",    0.1f },
	{ "DCO1_PHASE",     0.0f },
	{ "DCO1_OCTAVE",    0.0f },
	{ "DCO1_TUNING",    0.0f },
	{ "DCO1_GLIDE",     0.0f },
	{ "DCF1_CUTOFF",    0.5f },
	{ "DCF1_RESO",      0.0f },
	{ "DCF1_TYPE",      0.0f },
	{ "DCF1_SLOPE",     0.0f },
	{ "DCF1_ENVELOPE",  1.0f },
	{ "DCF1_ATTACK",    0.0f },
	{ "DCF1_DECAY",     0.2f },
	{ "DCF1_SUSTAIN",   0.5f },
	{ "DCF1_RELEASE",   0.5f },
	{ "LFO1_SHAPE",     1.0f },
	{ "LFO1_WIDTH",     1.0f },
	{ "LFO1_RATE",      0.5f },
	{ "LFO1_SWEEP",     0.0f },
	{ "LFO1_PITCH",     0.0f },
	{ "LFO1_CUTOFF",    0.0f },
	{ "LFO1_RESO",      0.0f },
	{ "LFO1_PANNING",   0.0f },
	{ "LFO1_VOLUME",    0.0f },
	{ "LFO1_ATTACK",    0.0f },
	{ "LFO1_DECAY",     0.1f },
	{ "LFO1_SUSTAIN",   1.0f },
	{ "LFO1_RELEASE",   0.5f },
	{ "DCA1_VOLUME",    0.5f },
	{ "DCA1_ATTACK",    0.0f },
	{ "DCA1_DECAY",     0.1f },
	{ "DCA1_SUSTAIN",   1.0f },
	{ "DCA1_RELEASE",   0.1f },
	{ "OUT1_WIDTH",     0.0f },
	{ "OUT1_PANNING",   0.0f },
	{ "OUT1_VOLUME",    0.5f },

	{ "DEF1_PITCHBEND", 0.2f },
	{ "DEF1_MODWHEEL",  0.2f },
	{ "DEF1_PRESSURE",  0.2f },
	{ "DEF1_VELOCITY",  0.2f },

	{ "DCO2_SHAPE1",    1.0f },
	{ "DCO2_WIDTH1",    1.0f },
	{ "DCO2_SHAPE2",    1.0f },
	{ "DCO2_WIDTH2",    1.0f },
	{ "DCO2_BALANCE",   0.0f },
	{ "DCO2_DETUNE",    0.1f },
	{ "DCO2_PHASE",     0.0f },
	{ "DCO2_OCTAVE",   -2.0f },
	{ "DCO2_TUNING",    0.0f },
	{ "DCO2_GLIDE",     0.0f },
	{ "DCF2_CUTOFF",    0.5f },
	{ "DCF2_RESO",      0.0f },
	{ "DCF2_TYPE",      0.0f },
	{ "DCF2_SLOPE",     0.0f },
	{ "DCF2_ENVELOPE",  1.0f },
	{ "DCF2_ATTACK",    0.0f },
	{ "DCF2_DECAY",     0.2f },
	{ "DCF2_SUSTAIN",   0.5f },
	{ "DCF2_RELEASE",   0.5f },
	{ "LFO2_SHAPE",     1.0f },
	{ "LFO2_WIDTH",     1.0f },
	{ "LFO2_RATE",      0.5f },
	{ "LFO2_SWEEP",     0.0f },
	{ "LFO2_PITCH",     0.0f },
	{ "LFO2_CUTOFF",    0.0f },
	{ "LFO2_RESO",      0.0f },
	{ "LFO2_PANNING",   0.0f },
	{ "LFO2_VOLUME",    0.0f },
	{ "LFO2_ATTACK",    0.0f },
	{ "LFO2_DECAY",     0.1f },
	{ "LFO2_SUSTAIN",   1.0f },
	{ "LFO2_RELEASE",   0.5f },
	{ "DCA2_VOLUME",    0.5f },
	{ "DCA2_ATTACK",    0.0f },
	{ "DCA2_DECAY",     0.1f },
	{ "DCA2_SUSTAIN",   1.0f },
	{ "DCA2_RELEASE",   0.1f },
	{ "OUT2_WIDTH",     0.0f },
	{ "OUT2_PANNING",   0.0f },
	{ "OUT2_VOLUME",    0.5f },

	{ "DEF2_PITCHBEND", 0.2f },
	{ "DEF2_MODWHEEL",  0.2f },
	{ "DEF2_PRESSURE",  0.2f },
	{ "DEF2_VELOCITY",  0.2f },

	{ "CHO1_WET",       0.0f },
	{ "CHO1_DELAY",     0.5f },
	{ "CHO1_FEEDB",     0.5f },
	{ "CHO1_RATE",      0.5f },
	{ "CHO1_MOD",       0.5f },
	{ "FLA1_WET",       0.0f },
	{ "FLA1_DELAY",     0.5f },
	{ "FLA1_FEEDB",     0.5f },
	{ "FLA1_DAFT",      0.0f },
	{ "PHA1_WET",       0.0f },
	{ "PHA1_RATE",      0.5f },
	{ "PHA1_FEEDB",     0.5f },
	{ "PHA1_DEPTH",     0.5f },
	{ "PHA1_DAFT",      0.0f },
	{ "DEL1_WET",       0.0f },
	{ "DEL1_DELAY",     0.5f },
	{ "DEL1_FEEDB",     0.5f },
	{ "DEL1_BPM",       1.8f },
	{ "DYN1_COMPRESS",  0.0f },
	{ "DYN1_LIMIT",     1.0f }
};


//-------------------------------------------------------------------------
// synthv1widget - impl.
//

// Constructor.
synthv1widget::synthv1widget ( QWidget *pParent, Qt::WindowFlags wflags )
	: QWidget(pParent, wflags)
{
	Q_INIT_RESOURCE(synthv1);

	m_ui.setupUi(this);

	// Init swapable params A/B to default.
	for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i)
		m_params_ab[i] = synthv1_default_params[i].value;

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

	m_ui.Dcf1SlopeKnob->insertItems(0, slopes);
	m_ui.Dcf2SlopeKnob->insertItems(0, slopes);

	// Dynamic states.
	QStringList states;
	states << tr("Off");
	states << tr("On");

	m_ui.Dyn1CompressKnob->insertItems(0, states);
	m_ui.Dyn1LimiterKnob->insertItems(0, states);

	// Special values
	const QString& sOff = states.first();
	m_ui.Cho1WetKnob->setSpecialValueText(sOff);
	m_ui.Fla1WetKnob->setSpecialValueText(sOff);
	m_ui.Pha1WetKnob->setSpecialValueText(sOff);
	m_ui.Del1WetKnob->setSpecialValueText(sOff);

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
	m_ui.Lfo1SweepKnob->setMinimum(-1.0f);
	m_ui.Lfo1SweepKnob->setMaximum(+1.0f);
	m_ui.Lfo1CutoffKnob->setMinimum(-1.0f);
	m_ui.Lfo1CutoffKnob->setMaximum(+1.0f);
	m_ui.Lfo1ResoKnob->setMinimum(-1.0f);
	m_ui.Lfo1ResoKnob->setMaximum(+1.0f);
	m_ui.Lfo1PitchKnob->setMinimum(-1.0f);
	m_ui.Lfo1PitchKnob->setMaximum(+1.0f);
	m_ui.Lfo1PanningKnob->setMinimum(-1.0f);
	m_ui.Lfo1PanningKnob->setMaximum(+1.0f);
	m_ui.Lfo1VolumeKnob->setMinimum(-1.0f);
	m_ui.Lfo1VolumeKnob->setMaximum(+1.0f);

	m_ui.Lfo2SweepKnob->setMinimum(-1.0f);
	m_ui.Lfo2SweepKnob->setMaximum(+1.0f);
	m_ui.Lfo2CutoffKnob->setMinimum(-1.0f);
	m_ui.Lfo2CutoffKnob->setMaximum(+1.0f);
	m_ui.Lfo2ResoKnob->setMinimum(-1.0f);
	m_ui.Lfo2ResoKnob->setMaximum(+1.0f);
	m_ui.Lfo2PitchKnob->setMinimum(-1.0f);
	m_ui.Lfo2PitchKnob->setMaximum(+1.0f);
	m_ui.Lfo2PanningKnob->setMinimum(-1.0f);
	m_ui.Lfo2PanningKnob->setMaximum(+1.0f);
	m_ui.Lfo2VolumeKnob->setMinimum(-1.0f);
	m_ui.Lfo2VolumeKnob->setMaximum(+1.0f);

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
	m_ui.Del1BpmKnob->setMinimum(0.0f);
	m_ui.Del1BpmKnob->setMaximum(3.6f);


	// DCO1
	setParamKnob(synthv1::DCO1_SHAPE1,  m_ui.Dco1Shape1Knob);
	setParamKnob(synthv1::DCO1_WIDTH1,  m_ui.Dco1Width1Knob);
	setParamKnob(synthv1::DCO1_SHAPE2,  m_ui.Dco1Shape2Knob);
	setParamKnob(synthv1::DCO1_WIDTH2,  m_ui.Dco1Width2Knob);
	setParamKnob(synthv1::DCO1_BALANCE, m_ui.Dco1BalanceKnob);
	setParamKnob(synthv1::DCO1_DETUNE,  m_ui.Dco1DetuneKnob);
	setParamKnob(synthv1::DCO1_PHASE,   m_ui.Dco1PhaseKnob);
	setParamKnob(synthv1::DCO1_OCTAVE,  m_ui.Dco1OctaveKnob);
	setParamKnob(synthv1::DCO1_TUNING,  m_ui.Dco1TuningKnob);
	setParamKnob(synthv1::DCO1_GLIDE,   m_ui.Dco1GlideKnob);

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
	setParamKnob(synthv1::LFO1_RATE,    m_ui.Lfo1RateKnob);
	setParamKnob(synthv1::LFO1_PANNING, m_ui.Lfo1PanningKnob);
	setParamKnob(synthv1::LFO1_VOLUME,  m_ui.Lfo1VolumeKnob);
	setParamKnob(synthv1::LFO1_CUTOFF,  m_ui.Lfo1CutoffKnob);
	setParamKnob(synthv1::LFO1_RESO,    m_ui.Lfo1ResoKnob);
	setParamKnob(synthv1::LFO1_PITCH,   m_ui.Lfo1PitchKnob);
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

	// OUT1
	setParamKnob(synthv1::OUT1_WIDTH,   m_ui.Out1WidthKnob);
	setParamKnob(synthv1::OUT1_PANNING, m_ui.Out1PanningKnob);
	setParamKnob(synthv1::OUT1_VOLUME,  m_ui.Out1VolumeKnob);


	// DCO2
	setParamKnob(synthv1::DCO2_SHAPE1,  m_ui.Dco2Shape1Knob);
	setParamKnob(synthv1::DCO2_WIDTH1,  m_ui.Dco2Width1Knob);
	setParamKnob(synthv1::DCO2_SHAPE2,  m_ui.Dco2Shape2Knob);
	setParamKnob(synthv1::DCO2_WIDTH2,  m_ui.Dco2Width2Knob);
	setParamKnob(synthv1::DCO2_BALANCE, m_ui.Dco2BalanceKnob);
	setParamKnob(synthv1::DCO2_DETUNE,  m_ui.Dco2DetuneKnob);
	setParamKnob(synthv1::DCO2_PHASE,   m_ui.Dco2PhaseKnob);
	setParamKnob(synthv1::DCO2_OCTAVE,  m_ui.Dco2OctaveKnob);
	setParamKnob(synthv1::DCO2_TUNING,  m_ui.Dco2TuningKnob);
	setParamKnob(synthv1::DCO2_GLIDE,   m_ui.Dco2GlideKnob);

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
	setParamKnob(synthv1::LFO2_RATE,    m_ui.Lfo2RateKnob);
	setParamKnob(synthv1::LFO2_PANNING, m_ui.Lfo2PanningKnob);
	setParamKnob(synthv1::LFO2_VOLUME,  m_ui.Lfo2VolumeKnob);
	setParamKnob(synthv1::LFO2_CUTOFF,  m_ui.Lfo2CutoffKnob);
	setParamKnob(synthv1::LFO2_RESO,    m_ui.Lfo2ResoKnob);
	setParamKnob(synthv1::LFO2_PITCH,   m_ui.Lfo2PitchKnob);
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

	// OUT2
	setParamKnob(synthv1::OUT2_WIDTH,   m_ui.Out2WidthKnob);
	setParamKnob(synthv1::OUT2_PANNING, m_ui.Out2PanningKnob);
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
	QObject::connect(m_ui.helpAboutAction,
		SIGNAL(triggered(bool)),
		SLOT(helpAbout()));
	QObject::connect(m_ui.helpAboutQtAction,
		SIGNAL(triggered(bool)),
		SLOT(helpAboutQt()));

	// Epilog.
	// QWidget::adjustSize();

	m_ui.StatusBar->showMessage(tr("Ready"), 5000);
	m_ui.StatusBar->setModified(false);
}


// Param kbob (widget) map accesors.
void synthv1widget::setParamKnob ( synthv1::ParamIndex index, synthv1widget_knob *pKnob )
{
	m_paramKnobs.insert(index, pKnob);
	m_knobParams.insert(pKnob, index);

	QObject::connect(pKnob,
		SIGNAL(valueChanged(float)),
		SLOT(paramChanged(float)));
}

synthv1widget_knob *synthv1widget::paramKnob ( synthv1::ParamIndex index ) const
{
	return m_paramKnobs.value(index, NULL);
}


// Param port accessors.
void synthv1widget::setParamValue ( synthv1::ParamIndex index, float fValue )
{
	++m_iUpdate;

	synthv1widget_knob *pKnob = paramKnob(index);
	if (pKnob)
		pKnob->setValue(fValue);

	--m_iUpdate;
}

float synthv1widget::paramValue ( synthv1::ParamIndex index ) const
{
	synthv1widget_knob *pKnob = paramKnob(index);
	return (pKnob ? pKnob->value() : 0.0f);
}


// Param knob (widget) slot.
void synthv1widget::paramChanged ( float fValue )
{
	if (m_iUpdate > 0)
		return;

	synthv1widget_knob *pKnob = qobject_cast<synthv1widget_knob *> (sender());
	if (pKnob) {
		updateParam(m_knobParams.value(pKnob), fValue);
		m_ui.StatusBar->showMessage(QString("%1 / %2: %3")
			.arg(m_ui.StackedWidget->currentWidget()->windowTitle())
			.arg(pKnob->toolTip())
			.arg(pKnob->valueText()), 5000);
		m_ui.StatusBar->setModified(true);
	}

	m_ui.Preset->dirtyPreset();
}


// Reset all param knobs to default values.
void synthv1widget::resetParams (void)
{
	resetSwapParams();

	for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i) {
		synthv1::ParamIndex index = synthv1::ParamIndex(i);
		float fValue = synthv1_default_params[i].value;
		synthv1widget_knob *pKnob = paramKnob(index);
		if (pKnob)
			fValue = pKnob->defaultValue();
		setParamValue(index, fValue);
		updateParam(index, fValue);
		m_params_ab[index] = fValue;
	}

	m_ui.StatusBar->showMessage(tr("Reset preset"), 5000);
	m_ui.StatusBar->setModified(false);
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
		synthv1::ParamIndex index = synthv1::ParamIndex(i);
		synthv1widget_knob *pKnob = paramKnob(index);
		if (pKnob) {
			const float fOldValue = pKnob->value();
			const float fNewValue = m_params_ab[index];
			setParamValue(index, fNewValue);
			updateParam(index, fNewValue);
			m_params_ab[index] = fOldValue;
		}
	}

	m_ui.Preset->dirtyPreset();

	const bool bSwapA = m_ui.SwapParamsAButton->isChecked();
	m_ui.StatusBar->showMessage(tr("Swap %1").arg(bSwapA ? 'A' : 'B'), 5000);
	m_ui.StatusBar->setModified(true);
}


// Reset swap params A/B group.
void synthv1widget::resetSwapParams (void)
{
	++m_iUpdate;
	m_ui.SwapParamsAButton->setChecked(true);
	--m_iUpdate;
}


// Reset all param default values.
void synthv1widget::resetParamValues (void)
{
	resetSwapParams();

	for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i) {
		synthv1::ParamIndex index = synthv1::ParamIndex(i);
		float fValue = synthv1_default_params[i].value;
		setParamValue(index, fValue);
		updateParam(index, fValue);
		m_params_ab[index] = fValue;
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
	m_ui.StatusBar->setModified(false);
}


// Preset file I/O slots.
void synthv1widget::loadPreset ( const QString& sFilename )
{
#ifdef CONFIG_DEBUG
	qDebug("synthv1widget::loadPreset(\"%s\")", sFilename.toUtf8().constData());
#endif

	QFile file(sFilename);
	if (!file.open(QIODevice::ReadOnly))
		return;

	static QHash<QString, synthv1::ParamIndex> s_hash;
	if (s_hash.isEmpty()) {
		for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i)
			s_hash.insert(synthv1_default_params[i].name, synthv1::ParamIndex(i));
	}

	resetParamValues();
	resetParamKnobs();

	const QFileInfo fi(sFilename);
	const QDir currentDir(QDir::current());
	QDir::setCurrent(fi.absolutePath());

	QDomDocument doc(SYNTHV1_TITLE);
	if (doc.setContent(&file)) {
		QDomElement ePreset = doc.documentElement();
		if (ePreset.tagName() == "preset"
			&& ePreset.attribute("name") == fi.completeBaseName()) {
			for (QDomNode nChild = ePreset.firstChild();
					!nChild.isNull();
						nChild = nChild.nextSibling()) {
				QDomElement eChild = nChild.toElement();
				if (eChild.isNull())
					continue;
				if (eChild.tagName() == "params") {
					for (QDomNode nParam = eChild.firstChild();
							!nParam.isNull();
								nParam = nParam.nextSibling()) {
						QDomElement eParam = nParam.toElement();
						if (eParam.isNull())
							continue;
						if (eParam.tagName() == "param") {
							synthv1::ParamIndex index = synthv1::ParamIndex(
								eParam.attribute("index").toULong());
							const QString& sName = eParam.attribute("name");
							if (!sName.isEmpty() && s_hash.contains(sName))
								index = s_hash.value(sName);
							float fValue = eParam.text().toFloat();
							setParamValue(index, fValue);
							updateParam(index, fValue);
							m_params_ab[index] = fValue;
						}
					}
				}
			}
		}
	}

	file.close();

	const QString& sPreset = fi.completeBaseName();
	m_ui.Preset->setPreset(sPreset);

	m_ui.StatusBar->showMessage(tr("Load preset: %1").arg(sPreset), 5000);
	m_ui.StatusBar->setModified(false);

	QDir::setCurrent(currentDir.absolutePath());
}


void synthv1widget::savePreset ( const QString& sFilename )
{
#ifdef CONFIG_DEBUG
	qDebug("synthv1widget::savePreset(\"%s\")", sFilename.toUtf8().constData());
#endif
	const QString& sPreset = QFileInfo(sFilename).completeBaseName();

	QDomDocument doc(SYNTHV1_TITLE);
	QDomElement ePreset = doc.createElement("preset");
	ePreset.setAttribute("name", sPreset);
	ePreset.setAttribute("version", SYNTHV1_VERSION);

	QDomElement eParams = doc.createElement("params");
	for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i) {
		QDomElement eParam = doc.createElement("param");
		eParam.setAttribute("index", QString::number(i));
		eParam.setAttribute("name", synthv1_default_params[i].name);
		eParam.appendChild(doc.createTextNode(QString::number(
			paramValue(synthv1::ParamIndex(i)))));
		eParams.appendChild(eParam);
	}
	ePreset.appendChild(eParams);
	doc.appendChild(ePreset);

	QFile file(sFilename);
	if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
		QTextStream(&file) << doc.toString();
		file.close();
	}

	m_ui.StatusBar->showMessage(tr("Save preset: %1").arg(sPreset), 5000);
	m_ui.StatusBar->setModified(false);
}


// Dirty close prompt,
bool synthv1widget::queryClose (void)
{
	return m_ui.Preset->queryPreset();
}


// Menu actions.
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
	sText += tr("Version") + ": <b>" SYNTHV1_VERSION "</b><br />\n";
	sText += "<small>" + tr("Build") + ": " __DATE__ " " __TIME__ "</small><br />\n";
	QStringListIterator iter(list);
	while (iter.hasNext()) {
		sText += "<small><font color=\"red\">";
		sText += iter.next();
		sText += "</font></small><br />";
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


// end of synthv1widget.cpp
