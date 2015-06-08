// synthv1widget_controls.cpp
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

#include "synthv1widget_controls.h"

#include "synthv1_controls.h"
#include "synthv1_config.h"

#include <QHeaderView>

#include <QSpinBox>
#include <QLineEdit>
#include <QComboBox>


//----------------------------------------------------------------------------
// MIDI Controller Names - Default controller names hash map.

const synthv1widget_controls::Names& synthv1widget_controls::controllerNames (void)
{
	static struct
	{
		unsigned short param;
		const char *name;

	} s_controllers[] = {

		{  0, QT_TR_NOOP("Bank Select (coarse)") },
		{  1, QT_TR_NOOP("Modulation Wheel (coarse)") },
		{  2, QT_TR_NOOP("Breath Controller (coarse)") },
		{  4, QT_TR_NOOP("Foot Pedal (coarse)") },
		{  5, QT_TR_NOOP("Portamento Time (coarse)") },
		{  6, QT_TR_NOOP("Data Entry (coarse)") },
		{  7, QT_TR_NOOP("Volume (coarse)") },
		{  8, QT_TR_NOOP("Balance (coarse)") },
		{ 10, QT_TR_NOOP("Pan Position (coarse)") },
		{ 11, QT_TR_NOOP("Expression (coarse)") },
		{ 12, QT_TR_NOOP("Effect Control 1 (coarse)") },
		{ 13, QT_TR_NOOP("Effect Control 2 (coarse)") },
		{ 16, QT_TR_NOOP("General Purpose Slider 1") },
		{ 17, QT_TR_NOOP("General Purpose Slider 2") },
		{ 18, QT_TR_NOOP("General Purpose Slider 3") },
		{ 19, QT_TR_NOOP("General Purpose Slider 4") },
		{ 32, QT_TR_NOOP("Bank Select (fine)") },
		{ 33, QT_TR_NOOP("Modulation Wheel (fine)") },
		{ 34, QT_TR_NOOP("Breath Controller (fine)") },
		{ 36, QT_TR_NOOP("Foot Pedal (fine)") },
		{ 37, QT_TR_NOOP("Portamento Time (fine)") },
		{ 38, QT_TR_NOOP("Data Entry (fine)") },
		{ 39, QT_TR_NOOP("Volume (fine)") },
		{ 40, QT_TR_NOOP("Balance (fine)") },
		{ 42, QT_TR_NOOP("Pan Position (fine)") },
		{ 43, QT_TR_NOOP("Expression (fine)") },
		{ 44, QT_TR_NOOP("Effect Control 1 (fine)") },
		{ 45, QT_TR_NOOP("Effect Control 2 (fine)") },
		{ 64, QT_TR_NOOP("Hold Pedal (on/off)") },
		{ 65, QT_TR_NOOP("Portamento (on/off)") },
		{ 66, QT_TR_NOOP("Sustenuto Pedal (on/off)") },
		{ 67, QT_TR_NOOP("Soft Pedal (on/off)") },
		{ 68, QT_TR_NOOP("Legato Pedal (on/off)") },
		{ 69, QT_TR_NOOP("Hold 2 Pedal (on/off)") },
		{ 70, QT_TR_NOOP("Sound Variation") },
		{ 71, QT_TR_NOOP("Sound Timbre") },
		{ 72, QT_TR_NOOP("Sound Release Time") },
		{ 73, QT_TR_NOOP("Sound Attack Time") },
		{ 74, QT_TR_NOOP("Sound Brightness") },
		{ 75, QT_TR_NOOP("Sound Control 6") },
		{ 76, QT_TR_NOOP("Sound Control 7") },
		{ 77, QT_TR_NOOP("Sound Control 8") },
		{ 78, QT_TR_NOOP("Sound Control 9") },
		{ 79, QT_TR_NOOP("Sound Control 10") },
		{ 80, QT_TR_NOOP("General Purpose Button 1 (on/off)") },
		{ 81, QT_TR_NOOP("General Purpose Button 2 (on/off)") },
		{ 82, QT_TR_NOOP("General Purpose Button 3 (on/off)") },
		{ 83, QT_TR_NOOP("General Purpose Button 4 (on/off)") },
		{ 91, QT_TR_NOOP("Effects Level") },
		{ 92, QT_TR_NOOP("Tremulo Level") },
		{ 93, QT_TR_NOOP("Chorus Level") },
		{ 94, QT_TR_NOOP("Celeste Level") },
		{ 95, QT_TR_NOOP("Phaser Level") },
		{ 96, QT_TR_NOOP("Data Button Increment") },
		{ 97, QT_TR_NOOP("Data Button Decrement") },
		{ 98, QT_TR_NOOP("Non-Registered Parameter (fine)") },
		{ 99, QT_TR_NOOP("Non-Registered Parameter (coarse)") },
		{100, QT_TR_NOOP("Registered Parameter (fine)") },
		{101, QT_TR_NOOP("Registered Parameter (coarse)") },
		{120, QT_TR_NOOP("All Sound Off") },
		{121, QT_TR_NOOP("All Controllers Off") },
		{122, QT_TR_NOOP("Local Keyboard (on/off)") },
		{123, QT_TR_NOOP("All Notes Off") },
		{124, QT_TR_NOOP("Omni Mode Off") },
		{125, QT_TR_NOOP("Omni Mode On") },
		{126, QT_TR_NOOP("Mono Operation") },
		{127, QT_TR_NOOP("Poly Operation") },

		{  0, NULL }
	};

	static Names s_controllerNames;

	// Pre-load controller-names hash table...
	if (s_controllerNames.isEmpty()) {
		for (int i = 0; s_controllers[i].name; ++i) {
			s_controllerNames.insert(s_controllers[i].param,
				QObject::tr(s_controllers[i].name, "controllerName"));
		}
	}

	return s_controllerNames;
}


//----------------------------------------------------------------------------
// MIDI RPN Names - Default RPN names hash map.

const synthv1widget_controls::Names& synthv1widget_controls::rpnNames (void)
{
	static struct
	{
		unsigned short param;
		const char *name;

	} s_rpns[] = {

		{  0, QT_TR_NOOP("Pitch Bend Sensitivity") },
		{  1, QT_TR_NOOP("Fine Tune") },
		{  2, QT_TR_NOOP("Coarse Tune") },
		{  3, QT_TR_NOOP("Tuning Program") },
		{  4, QT_TR_NOOP("Tuning Bank") },

		{  0, NULL }
	};

	static Names s_rpnNames;

	if (s_rpnNames.isEmpty()) {
		// Pre-load RPN-names hash table...
		for (int i = 0; s_rpns[i].name; ++i) {
			s_rpnNames.insert(s_rpns[i].param,
				QObject::tr(s_rpns[i].name, "rpnName"));
		}
	}

	return s_rpnNames;
}


//----------------------------------------------------------------------------
// MIDI NRPN Names - Default NRPN names hash map.

const synthv1widget_controls::Names& synthv1widget_controls::nrpnNames (void)
{
	static struct
	{
		unsigned short param;
		const char *name;

	} s_nrpns[] = {

		{  136, QT_TR_NOOP("Vibrato Rate") },
		{  137, QT_TR_NOOP("Vibrato Depth") },
		{  138, QT_TR_NOOP("Vibrato Delay") },
		{  160, QT_TR_NOOP("Filter Cutoff") },
		{  161, QT_TR_NOOP("Filter Resonance") },
		{  227, QT_TR_NOOP("EG Attack") },
		{  228, QT_TR_NOOP("EG Decay") },
		{  230, QT_TR_NOOP("EG Release") },

		// GS Drum NRPN map...
		{ 2560, QT_TR_NOOP("Drum Filter Cutoff") },
		{ 2688, QT_TR_NOOP("Drum Filter Resonance") },
		{ 2816, QT_TR_NOOP("Drum EG Attack") },
		{ 2944, QT_TR_NOOP("Drum EG Decay") },
		{ 3072, QT_TR_NOOP("Drum Pitch Coarse") },
		{ 3200, QT_TR_NOOP("Drum Pitch Fine") },
		{ 3328, QT_TR_NOOP("Drum Level") },
		{ 3584, QT_TR_NOOP("Drum Pan") },
		{ 3712, QT_TR_NOOP("Drum Reverb Send") },
		{ 3840, QT_TR_NOOP("Drum Chorus Send") },
		{ 3968, QT_TR_NOOP("Drum Variation Send") },

		{    0, NULL }
	};

	static struct
	{
		unsigned char note;
		const char *name;

	} s_drums[] = {

		// GM Drum note map...
		{ 35, QT_TR_NOOP("Acoustic Bass Drum") },
		{ 36, QT_TR_NOOP("Bass Drum 1") },
		{ 37, QT_TR_NOOP("Side Stick") },
		{ 38, QT_TR_NOOP("Acoustic Snare") },
		{ 39, QT_TR_NOOP("Hand Clap") },
		{ 40, QT_TR_NOOP("Electric Snare") },
		{ 41, QT_TR_NOOP("Low Floor Tom") },
		{ 42, QT_TR_NOOP("Closed Hi-Hat") },
		{ 43, QT_TR_NOOP("High Floor Tom") },
		{ 44, QT_TR_NOOP("Pedal Hi-Hat") },
		{ 45, QT_TR_NOOP("Low Tom") },
		{ 46, QT_TR_NOOP("Open Hi-Hat") },
		{ 47, QT_TR_NOOP("Low-Mid Tom") },
		{ 48, QT_TR_NOOP("Hi-Mid Tom") },
		{ 49, QT_TR_NOOP("Crash Cymbal 1") },
		{ 50, QT_TR_NOOP("High Tom") },
		{ 51, QT_TR_NOOP("Ride Cymbal 1") },
		{ 52, QT_TR_NOOP("Chinese Cymbal") },
		{ 53, QT_TR_NOOP("Ride Bell") },
		{ 54, QT_TR_NOOP("Tambourine") },
		{ 55, QT_TR_NOOP("Splash Cymbal") },
		{ 56, QT_TR_NOOP("Cowbell") },
		{ 57, QT_TR_NOOP("Crash Cymbal 2") },
		{ 58, QT_TR_NOOP("Vibraslap") },
		{ 59, QT_TR_NOOP("Ride Cymbal 2") },
		{ 60, QT_TR_NOOP("Hi Bongo") },
		{ 61, QT_TR_NOOP("Low Bongo") },
		{ 62, QT_TR_NOOP("Mute Hi Conga") },
		{ 63, QT_TR_NOOP("Open Hi Conga") },
		{ 64, QT_TR_NOOP("Low Conga") },
		{ 65, QT_TR_NOOP("High Timbale") },
		{ 66, QT_TR_NOOP("Low Timbale") },
		{ 67, QT_TR_NOOP("High Agogo") },
		{ 68, QT_TR_NOOP("Low Agogo") },
		{ 69, QT_TR_NOOP("Cabasa") },
		{ 70, QT_TR_NOOP("Maracas") },
		{ 71, QT_TR_NOOP("Short Whistle") },
		{ 72, QT_TR_NOOP("Long Whistle") },
		{ 73, QT_TR_NOOP("Short Guiro") },
		{ 74, QT_TR_NOOP("Long Guiro") },
		{ 75, QT_TR_NOOP("Claves") },
		{ 76, QT_TR_NOOP("Hi Wood Block") },
		{ 77, QT_TR_NOOP("Low Wood Block") },
		{ 78, QT_TR_NOOP("Mute Cuica") },
		{ 79, QT_TR_NOOP("Open Cuica") },
		{ 80, QT_TR_NOOP("Mute Triangle") },
		{ 81, QT_TR_NOOP("Open Triangle") },

		{  0, NULL }
	};

	static Names s_nrpnNames;

	if (s_nrpnNames.isEmpty()) {
		// Pre-load NRPN-names hash table...
		const QString sDrumNrpnName("%1 (%2)");
		for (int i = 0; s_nrpns[i].name; ++i) {
			const unsigned short param = s_nrpns[i].param;
			const QString& sName = QObject::tr(s_nrpns[i].name, "nrpnName");
			if (param < 2560) {
				s_nrpnNames.insert(param, sName);
			} else {
				for (int j = 0; s_drums[j].name; ++j) {
					const unsigned char note = s_drums[j].note;
					s_nrpnNames.insert(param + note,
						sDrumNrpnName.arg(sName).arg(note));
				}
			}
		}
	}

	return s_nrpnNames;
}


//----------------------------------------------------------------------------
// MIDI Control-14 Names - Default controller names hash map.

const synthv1widget_controls::Names& synthv1widget_controls::control14Names (void)
{
	static struct
	{
		unsigned short param;
		const char *name;

	} s_control14s[] = {

		{  1, QT_TR_NOOP("Modulation Wheel (14bit)") },
		{  2, QT_TR_NOOP("Breath Controller (14bit)") },
		{  4, QT_TR_NOOP("Foot Pedal (14bit)") },
		{  5, QT_TR_NOOP("Portamento Time (14bit)") },
		{  7, QT_TR_NOOP("Volume (14bit)") },
		{  8, QT_TR_NOOP("Balance (14bit)") },
		{ 10, QT_TR_NOOP("Pan Position (14bit)") },
		{ 11, QT_TR_NOOP("Expression (14bit)") },
		{ 12, QT_TR_NOOP("Effect Control 1 (14bit)") },
		{ 13, QT_TR_NOOP("Effect Control 2 (14bit)") },
		{ 16, QT_TR_NOOP("General Purpose Slider 1 (14bit)") },
		{ 17, QT_TR_NOOP("General Purpose Slider 2 (14bit)") },
		{ 18, QT_TR_NOOP("General Purpose Slider 3 (14bit)") },
		{ 19, QT_TR_NOOP("General Purpose Slider 4 (14bit)") },

		{  0, NULL }
	};

	static Names s_control14Names;

	if (s_control14Names.isEmpty()) {
		// Pre-load controller-names hash table...
		for (int i = 0; s_control14s[i].name; ++i) {
			s_control14Names.insert(s_control14s[i].param,
				QObject::tr(s_control14s[i].name, "control14Name"));
		}
	}

	return s_control14Names;
}


//----------------------------------------------------------------------------
// MIDI Controller Names general helpers.

static
QComboBox *controlParamComboBox (
	synthv1_controls::Type ctype, QWidget *pParent )
{
	QComboBox *pComboBox = new QComboBox(pParent);

	synthv1widget_controls::Names map;

	int iParamMin = 0;
	int iParamMax = iParamMin;

	switch(ctype) {
	case synthv1_controls::CC:
		iParamMin = 0;
		iParamMax = 128;
		map = synthv1widget_controls::controllerNames();
		break;
	case synthv1_controls::RPN:
		map = synthv1widget_controls::rpnNames();
		break;
	case synthv1_controls::NRPN:
		map = synthv1widget_controls::nrpnNames();
		break;
	case synthv1_controls::CC14:
		iParamMin = 1;
		iParamMax = 32;
		map = synthv1widget_controls::control14Names();
		// Fall thru...
	default:
		break;
	}

	const bool bEditable = (iParamMin >= iParamMax);
	pComboBox->setEditable(bEditable);
	pComboBox->setInsertPolicy(QComboBox::NoInsert);

	const QString sMask("%1 - %2");
	if (bEditable) {
		synthv1widget_controls::Names::ConstIterator iter
			= map.constBegin();
		const synthv1widget_controls::Names::ConstIterator& iter_end
			= map.constEnd();
		for ( ; iter != iter_end; ++iter) {
			const unsigned short param = iter.key();
			pComboBox->addItem(sMask.arg(param).arg(iter.value()), int(param));
		}
	} else {
		for (int iParam = iParamMin; iParam < iParamMax; ++iParam) {
			const unsigned short param = iParam;
			pComboBox->addItem(sMask.arg(param).arg(map.value(param)), iParam);
		}
	}

	return pComboBox;
}


static
QString controlParamName (
	synthv1_controls::Type ctype, unsigned short param )
{
	synthv1widget_controls::Names map;

	switch(ctype) {
	case synthv1_controls::CC:
		map = synthv1widget_controls::controllerNames();
		break;
	case synthv1_controls::RPN:
		map = synthv1widget_controls::rpnNames();
		break;
	case synthv1_controls::NRPN:
		map = synthv1widget_controls::nrpnNames();
		break;
	case synthv1_controls::CC14:
		map = synthv1widget_controls::control14Names();
		// Fall thru...
	default:
		break;
	}

	const QString sMask("%1 - %2");
	synthv1widget_controls::Names::ConstIterator iter = map.constFind(param);
	if (iter == map.constEnd())
		return QString::number(param);
	else
		return sMask.arg(param).arg(iter.value());
}


//----------------------------------------------------------------------------
// synthv1widget_controls_item_delegate -- Custom (tree) list item delegate.

// ctor.
synthv1widget_controls_item_delegate::synthv1widget_controls_item_delegate (
	QObject *pParent ) : QItemDelegate(pParent)
{
}


// QItemDelegate interface...
QSize synthv1widget_controls_item_delegate::sizeHint (
	const QStyleOptionViewItem& option, const QModelIndex& index ) const
{
	const int x = (index.column() == 1 ? 32 : 4); // Type is special.
	return QItemDelegate::sizeHint(option, index) + QSize(x, 4);
}


QWidget *synthv1widget_controls_item_delegate::createEditor ( QWidget *pParent,
	const QStyleOptionViewItem& /*option*/, const QModelIndex& index ) const
{
	QWidget *pEditor = NULL;

	switch (index.column()) {
	case 0: // Channel.
	{
		QSpinBox *pSpinBox = new QSpinBox(pParent);
		pSpinBox->setMinimum(0);
		pSpinBox->setMaximum(16);
		pSpinBox->setSpecialValueText(tr("Auto"));
		pEditor = pSpinBox;
		break;
	}

	case 1: // Type.
	{
		QComboBox *pComboBox = new QComboBox(pParent);
		pComboBox->setEditable(false);
		pComboBox->addItem(
			synthv1_controls::textFromType(synthv1_controls::CC));
		pComboBox->addItem(
			synthv1_controls::textFromType(synthv1_controls::RPN));
		pComboBox->addItem(
			synthv1_controls::textFromType(synthv1_controls::NRPN));
		pComboBox->addItem(
			synthv1_controls::textFromType(synthv1_controls::CC14));
		pEditor = pComboBox;
		break;
	}

	case 2: // Parameter.
	{
		const QModelIndex& ctype_index = index.sibling(index.row(), 1);
		const QString& sType = ctype_index.data().toString();
		const synthv1_controls::Type ctype
			= synthv1_controls::typeFromText(sType);
		pEditor = controlParamComboBox(ctype, pParent);
		break;
	}

	case 3: // Subject.
	{
		QComboBox *pComboBox = new QComboBox(pParent);
		pComboBox->setEditable(false);
		for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i)
			pComboBox->addItem(
				synthv1_param::paramName(synthv1::ParamIndex(i)));
		pEditor = pComboBox;
		break;
	}

	default:
		break;
	}

#ifdef CONFIG_DEBUG_0
	qDebug("synthv1widget_controls_item_delegate::createEditor(%p, %d, %d) = %p",
		pParent, index.row(), index.column(), pEditor);
#endif

	return pEditor;
}


void synthv1widget_controls_item_delegate::setEditorData (
	QWidget *pEditor, const QModelIndex& index ) const
{
#ifdef CONFIG_DEBUG_0
	qDebug("synthv1widget_controls_item_delegate::setEditorData(%p, %d, %d)",
		pEditor, index.row(), index.column());
#endif

	switch (index.column()) {
	case 0: // Channel.
	{
		const int iChannel = index.data().toInt();
		//	= index.model()->data(index, Qt::DisplayRole).toInt();
		QSpinBox *pSpinBox = qobject_cast<QSpinBox *> (pEditor);
		if (pSpinBox) pSpinBox->setValue(iChannel);
		break;
	}

	case 1: // Type.
	{
		const QString& sText = index.data().toString();
		//	= index.model()->data(index, Qt::DisplayRole).toString();
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) {
			const int iIndex = pComboBox->findText(sText);
			if (iIndex >= 0)
				pComboBox->setCurrentIndex(iIndex);
			else
				pComboBox->setCurrentIndex(0);
		}
		break;
	}

	case 2: // Parameter.
	{
		const int iParam = index.data(Qt::UserRole).toInt();
		//	= index.model()->data(index, Qt::DisplayRole).toString();
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) {
			const int iIndex = pComboBox->findData(iParam);
			if (iIndex >= 0)
				pComboBox->setCurrentIndex(iIndex);
			else
				pComboBox->setEditText(index.data().toString());
		}
		break;
	}

	case 3: // Subject.
	{
		const int iIndex = index.data(Qt::UserRole).toInt();
		//	= index.model()->data(index, Qt::DisplayRole).toInt();
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) pComboBox->setCurrentIndex(iIndex);
		break;
	}

	default:
		break;
	}
}


void synthv1widget_controls_item_delegate::setModelData ( QWidget *pEditor,
	QAbstractItemModel *pModel,	const QModelIndex& index ) const
{
#ifdef CONFIG_DEBUG_0
	qDebug("synthv1widget_controls_item_delegate::setModelData(%p, %d, %d)",
		pEditor, index.row(), index.column());
#endif

	switch (index.column()) {
	case 0: // Channel.
	{
		QSpinBox *pSpinBox = qobject_cast<QSpinBox *> (pEditor);
		if (pSpinBox) {
			const int iChannel = pSpinBox->value();
			const QString& sText
				= (iChannel > 0 ? QString::number(iChannel) : tr("Auto"));
			pModel->setData(index, sText);
		}
		break;
	}

	case 1: // Type.
	{
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) {
			const QString& sType = pComboBox->currentText();
			pModel->setData(index, sType);
		}
		break;
	}

	case 2: // Parameter.
	{
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) {
			const int iIndex = pComboBox->currentIndex();
			QString sText;
			int iParam;
			if (iIndex >= 0) {
				sText = pComboBox->itemText(iIndex);
				iParam = pComboBox->itemData(iIndex).toInt();
			} else {
				sText = pComboBox->currentText();
				iParam = sText.toInt();
			}
			pModel->setData(index, sText);
			pModel->setData(index, iParam, Qt::UserRole);
		}
		break;
	}

	case 3: // Subject.
	{
		QComboBox *pComboBox = qobject_cast<QComboBox *> (pEditor);
		if (pComboBox) {
			const int iIndex = pComboBox->currentIndex();
			pModel->setData(index,
				synthv1_param::paramName(synthv1::ParamIndex(iIndex)));
			pModel->setData(index, iIndex, Qt::UserRole);
		}
		break;
	}

	default:
		break;
	}

	// Done.
}


//----------------------------------------------------------------------------
// synthv1widget_controls -- UI wrapper form.

// ctor.
synthv1widget_controls::synthv1widget_controls ( QWidget *pParent )
	: QTreeWidget(pParent)
{
	QTreeWidget::setColumnCount(4);

	QTreeWidget::setRootIsDecorated(false);
	QTreeWidget::setAlternatingRowColors(true);
	QTreeWidget::setUniformRowHeights(true);
	QTreeWidget::setAllColumnsShowFocus(false);

	QTreeWidget::setSelectionBehavior(QAbstractItemView::SelectRows);
	QTreeWidget::setSelectionMode(QAbstractItemView::SingleSelection);

	QHeaderView *pHeaderView = QTreeWidget::header();
#if QT_VERSION < 0x050000
	pHeaderView->setResizeMode(QHeaderView::ResizeToContents);
#else
	pHeaderView->setSectionResizeMode(QHeaderView::ResizeToContents);
#endif
//	pHeaderView->hide();

	QTreeWidget::setItemDelegate(new synthv1widget_controls_item_delegate(this));

	QObject::connect(this,
		SIGNAL(itemChanged(QTreeWidgetItem *, int)),
		SLOT(itemChangedSlot(QTreeWidgetItem *, int)));
}


// dtor.
synthv1widget_controls::~synthv1widget_controls (void)
{
}


// utilities.
void synthv1widget_controls::loadControls ( synthv1_controls *pControls )
{
	QTreeWidget::clear();

	const QIcon icon(":/images/synthv1_control.png");
	QList<QTreeWidgetItem *> items;
	const synthv1_controls::Map& map = pControls->map();
	synthv1_controls::Map::ConstIterator iter = map.constBegin();
	const synthv1_controls::Map::ConstIterator& iter_end = map.constEnd();
	for ( ; iter != iter_end; ++iter) {
		const synthv1_controls::Key& key = iter.key();
		const synthv1_controls::Type ctype = key.type();
		const unsigned short channel = key.channel();
		const synthv1::ParamIndex index = synthv1::ParamIndex(iter.value());
		QTreeWidgetItem *pItem = new QTreeWidgetItem(this);
	//	pItem->setIcon(0, icon);
		pItem->setText(0, (channel > 0 ? QString::number(channel) : tr("Auto")));
		pItem->setText(1, synthv1_controls::textFromType(ctype));
		pItem->setText(2, controlParamName(ctype, key.param));
		pItem->setData(2, Qt::UserRole, int(key.param));
		pItem->setIcon(3, icon);
		pItem->setText(3, synthv1_param::paramName(index));
		pItem->setData(3, Qt::UserRole, int(index));
		pItem->setFlags(
			Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
		items.append(pItem);
	}
	QTreeWidget::addTopLevelItems(items);
	QTreeWidget::expandAll();
}


void synthv1widget_controls::saveControls ( synthv1_controls *pControls )
{
	pControls->clear();

	const int iItemCount = QTreeWidget::topLevelItemCount();
	for (int iItem = 0 ; iItem < iItemCount; ++iItem) {
		QTreeWidgetItem *pItem = QTreeWidget::topLevelItem(iItem);
		const unsigned short channel
			= pItem->text(0).toInt();
		const synthv1_controls::Type ctype
			= synthv1_controls::typeFromText(pItem->text(1));
		synthv1_controls::Key key;
		key.status = ctype | (channel & 0x1f);
		key.param = pItem->data(2, Qt::UserRole).toInt();
		pControls->add_control(key, pItem->data(3, Qt::UserRole).toInt());
	}
}


// slots.
void synthv1widget_controls::addControlItem (void)
{
	QTreeWidget::setFocus();

	QTreeWidgetItem *pItem = newControlItem();
	if (pItem) {
		QTreeWidget::setCurrentItem(pItem);
		QTreeWidget::editItem(pItem, 0);
	}
}


// factory methods.
QTreeWidgetItem *synthv1widget_controls::newControlItem (void)
{
	QTreeWidgetItem *pItem = new QTreeWidgetItem();
	const QIcon icon(":/images/synthv1_control.png");
	const synthv1_controls::Type ctype = synthv1_controls::CC;
//	pItem->setIcon(0, icon);
	pItem->setText(0, tr("Auto"));
	pItem->setText(1, synthv1_controls::textFromType(ctype));
	pItem->setText(2, controlParamName(ctype, 0));
	pItem->setData(2, Qt::UserRole, 0);
	pItem->setIcon(3, icon);
	pItem->setText(3, synthv1_param::paramName(synthv1::ParamIndex(0)));
	pItem->setData(3, Qt::UserRole, 0);
	pItem->setFlags(
		Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable);
	QTreeWidget::addTopLevelItem(pItem);

	return pItem;
}


void synthv1widget_controls::itemChangedSlot (
	QTreeWidgetItem *pItem, int column )
{
	if (column == 1) {
		const bool bBlockSignals = QTreeWidget::blockSignals(true);
		const QString& sType = pItem->text(1);
		const synthv1_controls::Type ctype
			= synthv1_controls::typeFromText(sType);
		const int iParam = pItem->data(2, Qt::UserRole).toInt();
		pItem->setText(2, controlParamName(ctype, iParam));
		QTreeWidget::blockSignals(bBlockSignals);
	}
}


// end of synthv1widget_controls.cpp
