// synthv1widget_config.cpp
//
/****************************************************************************
   Copyright (C) 2012-2022, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include "synthv1widget_config.h"
#include "synthv1widget_param.h"

#include "synthv1widget_palette.h"

#include "synthv1_ui.h"

#include "synthv1_controls.h"
#include "synthv1_programs.h"

#include "ui_synthv1widget_config.h"

#include <QPushButton>
#include <QComboBox>

#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QUrl>

#include <QMenu>

#include <QStyleFactory>


// Default (empty/blank) name.
static const char *g_pszDefName = QT_TRANSLATE_NOOP("synthv1widget_config", "(default)");


//----------------------------------------------------------------------------
// synthv1widget_config -- UI wrapper form.

// ctor.
synthv1widget_config::synthv1widget_config (
	synthv1_ui *pSynthUi, QWidget *pParent )
	: QDialog(pParent), p_ui(new Ui::synthv1widget_config), m_ui(*p_ui),
		m_pSynthUi(pSynthUi)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// Note names.
	QStringList notes;
	for (int note = 0; note < 128; ++note)
		notes << synthv1_ui::noteName(note);
	m_ui.TuningRefNoteComboBox->insertItems(0, notes);

	// Tuning specifics setup...
	m_ui.TuningTabBar->addTab(tr("&Global"));
	m_ui.TuningTabBar->addTab(tr("&Instance"));

	// Dialog dirty flags.
	m_iDirtyTuning   = 0;
	m_iDirtyControls = 0;
	m_iDirtyPrograms = 0;
	m_iDirtyOptions  = 0;

	// Setup options...
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig && m_pSynthUi) {
		const bool bPlugin = m_pSynthUi->isPlugin();
		m_ui.ProgramsPreviewCheckBox->setChecked(pConfig->bProgramsPreview);
		m_ui.UseNativeDialogsCheckBox->setChecked(pConfig->bUseNativeDialogs);
		m_ui.KnobDialModeComboBox->setCurrentIndex(pConfig->iKnobDialMode);
		m_ui.KnobEditModeComboBox->setCurrentIndex(pConfig->iKnobEditMode);
		m_ui.RandomizePercentSpinBox->setValue(pConfig->fRandomizePercent);
		// Custom display options (only for no-plugin forms)...
		resetCustomColorThemes(pConfig->sCustomColorTheme);
		resetCustomStyleThemes(pConfig->sCustomStyleTheme);
		// Load controllers database...
		synthv1_controls *pControls = m_pSynthUi->controls();
		if (pControls) {
			m_ui.ControlsTreeWidget->loadControls(pControls);
			m_ui.ControlsEnabledCheckBox->setEnabled(bPlugin);
			m_ui.ControlsEnabledCheckBox->setChecked(pControls->enabled());
		}
		// Load programs database...
		synthv1_programs *pPrograms = m_pSynthUi->programs();
		if (pPrograms) {
			m_ui.ProgramsTreeWidget->loadPrograms(pPrograms);
			m_ui.ProgramsEnabledCheckBox->setEnabled(bPlugin);
			m_ui.ProgramsPreviewCheckBox->setEnabled(!bPlugin);
			m_ui.ProgramsEnabledCheckBox->setChecked(pPrograms->enabled());
		}
		// Initialize conveniency options...
		loadComboBoxHistory(m_ui.TuningScaleFileComboBox);
		loadComboBoxHistory(m_ui.TuningKeyMapFileComboBox);
		// Micro-tonal tuning settings...
		const int iTuningTab = (m_pSynthUi->isTuningEnabled() ? 1 : 0);
		m_ui.TuningTabBar->setCurrentIndex(iTuningTab);
		tuningTabChanged(iTuningTab);
	}

	// Signal/slots connections...
	QObject::connect(m_ui.ControlsAddItemToolButton,
		SIGNAL(clicked()),
		SLOT(controlsAddItem()));
	QObject::connect(m_ui.ControlsEditToolButton,
		SIGNAL(clicked()),
		SLOT(controlsEditItem()));
	QObject::connect(m_ui.ControlsDeleteToolButton,
		SIGNAL(clicked()),
		SLOT(controlsDeleteItem()));

	QObject::connect(m_ui.ControlsTreeWidget,
		SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
		SLOT(controlsCurrentChanged()));
	QObject::connect(m_ui.ControlsTreeWidget,
		SIGNAL(itemChanged(QTreeWidgetItem *, int)),
		SLOT(controlsChanged()));

	QObject::connect(m_ui.ControlsEnabledCheckBox,
		SIGNAL(toggled(bool)),
		SLOT(controlsEnabled(bool)));

	QObject::connect(m_ui.ProgramsAddBankToolButton,
		SIGNAL(clicked()),
		SLOT(programsAddBankItem()));
	QObject::connect(m_ui.ProgramsAddItemToolButton,
		SIGNAL(clicked()),
		SLOT(programsAddItem()));
	QObject::connect(m_ui.ProgramsEditToolButton,
		SIGNAL(clicked()),
		SLOT(programsEditItem()));
	QObject::connect(m_ui.ProgramsDeleteToolButton,
		SIGNAL(clicked()),
		SLOT(programsDeleteItem()));

	QObject::connect(m_ui.ProgramsTreeWidget,
		SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)),
		SLOT(programsCurrentChanged()));
	QObject::connect(m_ui.ProgramsTreeWidget,
		SIGNAL(itemChanged(QTreeWidgetItem *, int)),
		SLOT(programsChanged()));
	QObject::connect(m_ui.ProgramsTreeWidget,
		SIGNAL(itemActivated(QTreeWidgetItem *, int)),
		SLOT(programsActivated()));

	QObject::connect(m_ui.ProgramsEnabledCheckBox,
		SIGNAL(toggled(bool)),
		SLOT(programsEnabled(bool)));

	// Custom context menu...
	m_ui.ControlsTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);
	m_ui.ProgramsTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

	QObject::connect(m_ui.ControlsTreeWidget,
		SIGNAL(customContextMenuRequested(const QPoint&)),
		SLOT(controlsContextMenuRequested(const QPoint&)));
	QObject::connect(m_ui.ProgramsTreeWidget,
		SIGNAL(customContextMenuRequested(const QPoint&)),
		SLOT(programsContextMenuRequested(const QPoint&)));

	// Tuning slots...
	QObject::connect(m_ui.TuningTabBar,
		SIGNAL(currentChanged(int)),
		SLOT(tuningTabChanged(int)));
	QObject::connect(m_ui.TuningEnabledCheckBox,
		SIGNAL(toggled(bool)),
		SLOT(tuningChanged()));
	QObject::connect(m_ui.TuningRefNoteComboBox,
		SIGNAL(activated(int)),
		SLOT(tuningChanged()));
	QObject::connect(m_ui.TuningRefPitchSpinBox,
		SIGNAL(valueChanged(double)),
		SLOT(tuningChanged()));
	QObject::connect(m_ui.TuningRefNotePushButton,
		SIGNAL(clicked()),
		SLOT(tuningRefNoteClicked()));
	QObject::connect(m_ui.TuningScaleFileComboBox,
		SIGNAL(activated(int)),
		SLOT(tuningChanged()));
	QObject::connect(m_ui.TuningScaleFileToolButton,
		SIGNAL(clicked()),
		SLOT(tuningScaleFileClicked()));
	QObject::connect(m_ui.TuningKeyMapFileToolButton,
		SIGNAL(clicked()),
		SLOT(tuningKeyMapFileClicked()));
	QObject::connect(m_ui.TuningKeyMapFileComboBox,
		SIGNAL(activated(int)),
		SLOT(tuningChanged()));

	// Options slots...
	QObject::connect(m_ui.ProgramsPreviewCheckBox,
		SIGNAL(toggled(bool)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.UseNativeDialogsCheckBox,
		SIGNAL(toggled(bool)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.KnobDialModeComboBox,
		SIGNAL(activated(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.KnobEditModeComboBox,
		SIGNAL(activated(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.CustomColorThemeComboBox,
		SIGNAL(activated(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.CustomColorThemeToolButton,
		SIGNAL(clicked()),
		SLOT(editCustomColorThemes()));
	QObject::connect(m_ui.CustomStyleThemeComboBox,
		SIGNAL(activated(int)),
		SLOT(optionsChanged()));
	QObject::connect(m_ui.RandomizePercentSpinBox,
		SIGNAL(valueChanged(double)),
		SLOT(optionsChanged()));

	// Dialog commands...
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(accepted()),
		SLOT(accept()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(rejected()),
		SLOT(reject()));

	// Done.
	stabilize();
}


// dtor.
synthv1widget_config::~synthv1widget_config (void)
{
	delete p_ui;
}


synthv1_ui *synthv1widget_config::ui_instance (void) const
{
	return m_pSynthUi;
}


// controllers command slots.
void synthv1widget_config::controlsAddItem (void)
{
	m_ui.ControlsTreeWidget->addControlItem();

	controlsChanged();
}


void synthv1widget_config::controlsEditItem (void)
{
	QTreeWidgetItem *pItem = m_ui.ControlsTreeWidget->currentItem();
	if (pItem)
		m_ui.ControlsTreeWidget->editItem(pItem, 0);

	controlsChanged();
}


void synthv1widget_config::controlsDeleteItem (void)
{
	QTreeWidgetItem *pItem = m_ui.ControlsTreeWidget->currentItem();
	if (pItem)
		delete pItem;

	controlsChanged();
}


// controllers janitorial slots.
void synthv1widget_config::controlsCurrentChanged (void)
{
	stabilize();
}


void synthv1widget_config::controlsContextMenuRequested ( const QPoint& pos )
{
	QTreeWidgetItem *pItem = m_ui.ControlsTreeWidget->currentItem();

	QMenu menu(this);
	QAction *pAction;

	bool bEnabled = (m_pSynthUi && m_pSynthUi->controls() != nullptr);

	pAction = menu.addAction(QIcon(":/images/synthv1_preset.png"),
		tr("&Add Controller"), this, SLOT(controlsAddItem()));
	pAction->setEnabled(bEnabled);

	menu.addSeparator();

	bEnabled = bEnabled && (pItem != nullptr);

	pAction = menu.addAction(QIcon(":/images/presetEdit.png"),
		tr("&Edit"), this, SLOT(controlsEditItem()));
	pAction->setEnabled(bEnabled);

	menu.addSeparator();

	pAction = menu.addAction(QIcon(":/images/presetDelete.png"),
		tr("&Delete"), this, SLOT(controlsDeleteItem()));
	pAction->setEnabled(bEnabled);

	menu.exec(m_ui.ControlsTreeWidget->mapToGlobal(pos));
}


void synthv1widget_config::controlsEnabled ( bool bOn )
{
	if (m_pSynthUi) {
		synthv1_controls *pControls = m_pSynthUi->controls();
		if (pControls && m_pSynthUi->isPlugin())
			pControls->enabled(bOn);
	}

	controlsChanged();
}


void synthv1widget_config::controlsChanged (void)
{
	++m_iDirtyControls;

	stabilize();
}


// programs command slots.
void synthv1widget_config::programsAddBankItem (void)
{
	m_ui.ProgramsTreeWidget->addBankItem();

	programsChanged();
}


void synthv1widget_config::programsAddItem (void)
{
	m_ui.ProgramsTreeWidget->addProgramItem();

	programsChanged();
}


void synthv1widget_config::programsEditItem (void)
{
	QTreeWidgetItem *pItem = m_ui.ProgramsTreeWidget->currentItem();
	if (pItem)
		m_ui.ProgramsTreeWidget->editItem(pItem, 1);

	programsChanged();
}


void synthv1widget_config::programsDeleteItem (void)
{
	QTreeWidgetItem *pItem = m_ui.ProgramsTreeWidget->currentItem();
	if (pItem)
		delete pItem;

	programsChanged();
}


// programs janitor slots.
void synthv1widget_config::programsCurrentChanged (void)
{
	stabilize();
}


void synthv1widget_config::programsContextMenuRequested ( const QPoint& pos )
{
	QTreeWidgetItem *pItem = m_ui.ProgramsTreeWidget->currentItem();

	QMenu menu(this);
	QAction *pAction;

	bool bEnabled = (m_pSynthUi && m_pSynthUi->programs() != nullptr);

	pAction = menu.addAction(QIcon(":/images/presetBank.png"),
		tr("Add &Bank"), this, SLOT(programsAddBankItem()));
	pAction->setEnabled(bEnabled);

	pAction = menu.addAction(QIcon(":/images/synthv1_preset.png"),
		tr("&Add Program"), this, SLOT(programsAddItem()));
	pAction->setEnabled(bEnabled);

	menu.addSeparator();

	bEnabled = bEnabled && (pItem != nullptr);

	pAction = menu.addAction(QIcon(":/images/presetEdit.png"),
		tr("&Edit"), this, SLOT(programsEditItem()));
	pAction->setEnabled(bEnabled);

	menu.addSeparator();

	pAction = menu.addAction(QIcon(":/images/presetDelete.png"),
		tr("&Delete"), this, SLOT(programsDeleteItem()));
	pAction->setEnabled(bEnabled);

	menu.exec(m_ui.ProgramsTreeWidget->mapToGlobal(pos));
}


void synthv1widget_config::programsEnabled ( bool bOn )
{
	if (m_pSynthUi) {
		synthv1_programs *pPrograms = m_pSynthUi->programs();
		if (pPrograms && m_pSynthUi->isPlugin())
			pPrograms->enabled(bOn);
	}

	programsChanged();
}


void synthv1widget_config::programsChanged (void)
{
	++m_iDirtyPrograms;

	stabilize();
}


void synthv1widget_config::programsActivated (void)
{
	if (m_pSynthUi) {
		synthv1_programs *pPrograms = m_pSynthUi->programs();
		if (m_ui.ProgramsPreviewCheckBox->isChecked() && pPrograms)
			m_ui.ProgramsTreeWidget->selectProgram(pPrograms);
	}

	stabilize();
}


// tuning command slots
void synthv1widget_config::tuningTabChanged ( int iTuningTab )
{
	// Prevent loss of some tuning changes here...
	if (m_iDirtyTuning > 0 &&
		QMessageBox::warning(this,
			tr("Warning"),
			tr("%1 tuning settings have been changed.\n\n"
			"Do you want to discard the changes?")
			.arg(m_ui.TuningTabBar->tabText(1 - iTuningTab).remove('&')),
			QMessageBox::Discard | QMessageBox::Cancel)
			== QMessageBox::Cancel) {
		const bool bBlockSignals = m_ui.TuningTabBar->blockSignals(true);
		m_ui.TuningTabBar->setCurrentIndex(1 - iTuningTab);
		m_ui.TuningTabBar->blockSignals(bBlockSignals);
		return;
	}

	if (iTuningTab == 0) {
		// Global (default) scope...
		synthv1_config *pConfig = synthv1_config::getInstance();
		if (pConfig) {
			m_ui.TuningEnabledCheckBox->setChecked(pConfig->bTuningEnabled);
			m_ui.TuningRefNoteComboBox->setCurrentIndex(pConfig->iTuningRefNote);
			m_ui.TuningRefPitchSpinBox->setValue(double(pConfig->fTuningRefPitch));
			setComboBoxCurrentItem(
				m_ui.TuningScaleFileComboBox,
				QFileInfo(pConfig->sTuningScaleFile));
			setComboBoxCurrentItem(
				m_ui.TuningKeyMapFileComboBox,
				QFileInfo(pConfig->sTuningKeyMapFile));
		}
	}
	else
	if (m_pSynthUi) {
		// Instance scope...
		m_ui.TuningEnabledCheckBox->setChecked(m_pSynthUi->isTuningEnabled());
		m_ui.TuningRefNoteComboBox->setCurrentIndex(m_pSynthUi->tuningRefNote());
		m_ui.TuningRefPitchSpinBox->setValue(double(m_pSynthUi->tuningRefPitch()));
		setComboBoxCurrentItem(
			m_ui.TuningScaleFileComboBox,
			QFileInfo(QString::fromUtf8(m_pSynthUi->tuningScaleFile())));
		setComboBoxCurrentItem(
			m_ui.TuningKeyMapFileComboBox,
			QFileInfo(QString::fromUtf8(m_pSynthUi->tuningKeyMapFile())));
	}

	// Reset tuning dirty flag...
	m_iDirtyTuning = 0;
}


void synthv1widget_config::tuningRefNoteClicked (void)
{
	m_ui.TuningRefNoteComboBox->setCurrentIndex(69);
	m_ui.TuningRefPitchSpinBox->setValue(double(440.0f));

	tuningChanged();
}


void synthv1widget_config::tuningScaleFileClicked (void)
{
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig == nullptr)
		return;

	QString sTuningScaleFile = m_ui.TuningScaleFileComboBox->currentText();

	const QString  sExt("scl");
	const QString& sTitle  = tr("Open Scale File");

	QStringList filters;
	filters.append(tr("Scale files (*.%1)").arg(sExt));
	filters.append(tr("All files (*.*)"));
	const QString& sFilter = filters.join(";;");

	QWidget *pParentWidget = nullptr;
	QFileDialog::Options options;
	if (pConfig->bDontUseNativeDialogs) {
		options |= QFileDialog::DontUseNativeDialog;
		pParentWidget = QWidget::window();
	}
#if 1//QT_VERSION < QT_VERSION_CHECK(4, 4, 0)
	sTuningScaleFile = QFileDialog::getOpenFileName(pParentWidget,
		sTitle, pConfig->sTuningScaleDir, sFilter, nullptr, options);
#else
	QFileDialog fileDialog(pParentWidget,
		sTitle, sTuningScaleFile, sFilter);
	fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	fileDialog.setFileMode(QFileDialog::ExistingFiles);
	fileDialog.setDefaultSuffix(sExt);
	QList<QUrl> urls(fileDialog.sidebarUrls());
	urls.append(QUrl::fromLocalFile(pConfig->sTuningScaleDir));
	fileDialog.setSidebarUrls(urls);
	fileDialog.setOptions(options);
	if (fileDialog.exec())
		sTuningScaleFile = fileDialog.selectedFiles().first();
#endif

	if (!sTuningScaleFile.isEmpty()) {
		const QFileInfo info(sTuningScaleFile);
		if (setComboBoxCurrentItem(m_ui.TuningScaleFileComboBox, info)) {
			pConfig->sTuningScaleDir = info.absolutePath();
			tuningChanged();
		}
	}
}


void synthv1widget_config::tuningKeyMapFileClicked (void)
{
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig == nullptr)
		return;

	QString sTuningKeyMapFile = m_ui.TuningKeyMapFileComboBox->currentText();

	const QString  sExt("kbm");
	const QString& sTitle  = tr("Open Key Map File");

	QStringList filters;
	filters.append(tr("Key Map files (*.%1)").arg(sExt));
	filters.append(tr("All files (*.*)"));
	const QString& sFilter = filters.join(";;");

	QWidget *pParentWidget = nullptr;
	QFileDialog::Options options;
	if (pConfig->bDontUseNativeDialogs) {
		options |= QFileDialog::DontUseNativeDialog;
		pParentWidget = QWidget::window();
	}
#if 1//QT_VERSION < QT_VERSION_CHECK(4, 4, 0)
	sTuningKeyMapFile = QFileDialog::getOpenFileName(pParentWidget,
		sTitle, pConfig->sTuningKeyMapDir, sFilter, nullptr, options);
#else
	QFileDialog fileDialog(pParentWidget,
		sTitle, sTuningScaleFile, sFilter);
	fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	fileDialog.setFileMode(QFileDialog::ExistingFiles);
	fileDialog.setDefaultSuffix(sExt);
	QList<QUrl> urls(fileDialog.sidebarUrls());
	urls.append(QUrl::fromLocalFile(pConfig->sTuningKeyMapDir));
	fileDialog.setSidebarUrls(urls);
	fileDialog.setOptions(options);
	if (fileDialog.exec())
		sTuningKeyMapFile = fileDialog.selectedFiles().first();
#endif

	if (!sTuningKeyMapFile.isEmpty()) {
		const QFileInfo info(sTuningKeyMapFile);
		if (setComboBoxCurrentItem(m_ui.TuningKeyMapFileComboBox, info)) {
			pConfig->sTuningKeyMapDir = info.absolutePath();
			tuningChanged();
		}
	}
}


void synthv1widget_config::tuningChanged (void)
{
	++m_iDirtyTuning;

	stabilize();
}


// options slot.
void synthv1widget_config::optionsChanged (void)
{
	++m_iDirtyOptions;

	stabilize();
}


// stabilizer.
void synthv1widget_config::stabilize (void)
{
	QTreeWidgetItem *pItem = m_ui.ControlsTreeWidget->currentItem();
	bool bEnabled = (m_pSynthUi && m_pSynthUi->controls() != nullptr);
	m_ui.ControlsAddItemToolButton->setEnabled(bEnabled);
	bEnabled = bEnabled && (pItem != nullptr);
	m_ui.ControlsEditToolButton->setEnabled(bEnabled);
	m_ui.ControlsDeleteToolButton->setEnabled(bEnabled);

	pItem = m_ui.ProgramsTreeWidget->currentItem();
	bEnabled = (m_pSynthUi && m_pSynthUi->programs() != nullptr);
	m_ui.ProgramsPreviewCheckBox->setEnabled(
		bEnabled && m_ui.ProgramsEnabledCheckBox->isChecked());
	m_ui.ProgramsAddBankToolButton->setEnabled(bEnabled);
	m_ui.ProgramsAddItemToolButton->setEnabled(bEnabled);
	bEnabled = bEnabled && (pItem != nullptr);
	m_ui.ProgramsEditToolButton->setEnabled(bEnabled);
	m_ui.ProgramsDeleteToolButton->setEnabled(bEnabled);

	bEnabled = m_ui.TuningEnabledCheckBox->isChecked();
	const bool bTuningRefEnabled = bEnabled
		&& comboBoxCurrentItem(m_ui.TuningKeyMapFileComboBox).isEmpty();
	m_ui.TuningRefNoteTextLabel->setEnabled(bTuningRefEnabled);
	m_ui.TuningRefNoteComboBox->setEnabled(bTuningRefEnabled);
	m_ui.TuningRefPitchSpinBox->setEnabled(bTuningRefEnabled);
	m_ui.TuningRefNotePushButton->setEnabled(bTuningRefEnabled);
	m_ui.TuningScaleFileTextLabel->setEnabled(bEnabled);
	m_ui.TuningScaleFileComboBox->setEnabled(bEnabled);
	m_ui.TuningScaleFileToolButton->setEnabled(bEnabled);
	m_ui.TuningKeyMapFileTextLabel->setEnabled(bEnabled);
	m_ui.TuningKeyMapFileComboBox->setEnabled(bEnabled);
	m_ui.TuningKeyMapFileToolButton->setEnabled(bEnabled);

	const bool bValid
		= (m_iDirtyTuning   > 0
		|| m_iDirtyControls > 0
		|| m_iDirtyPrograms > 0
		|| m_iDirtyOptions  > 0);
	m_ui.DialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(bValid);
}


// dialog slots.
void synthv1widget_config::accept (void)
{
	synthv1_config *pConfig = synthv1_config::getInstance();

	if (m_iDirtyTuning > 0 && pConfig && m_pSynthUi) {
		// Micro-tonal tuning settings...
		if (m_ui.TuningTabBar->currentIndex() == 0) {
			// Global (default) scope...
			pConfig->bTuningEnabled = m_ui.TuningEnabledCheckBox->isChecked();
			pConfig->iTuningRefNote = m_ui.TuningRefNoteComboBox->currentIndex();
			pConfig->fTuningRefPitch = float(m_ui.TuningRefPitchSpinBox->value());
			pConfig->sTuningScaleFile = comboBoxCurrentItem(m_ui.TuningScaleFileComboBox);
			pConfig->sTuningKeyMapFile = comboBoxCurrentItem(m_ui.TuningKeyMapFileComboBox);
		} else {
			m_pSynthUi->setTuningEnabled(
				m_ui.TuningEnabledCheckBox->isChecked());
			m_pSynthUi->setTuningRefNote(
				m_ui.TuningRefNoteComboBox->currentIndex());
			m_pSynthUi->setTuningRefPitch(
				float(m_ui.TuningRefPitchSpinBox->value()));
			m_pSynthUi->setTuningScaleFile(comboBoxCurrentItem(
				m_ui.TuningScaleFileComboBox).toUtf8().constData());
			m_pSynthUi->setTuningKeyMapFile(comboBoxCurrentItem(
				m_ui.TuningKeyMapFileComboBox).toUtf8().constData());
		}
		// Reset/update micro-tonal tuning...
		m_pSynthUi->resetTuning();
		// Save other conveniency options...
		saveComboBoxHistory(m_ui.TuningScaleFileComboBox);
		saveComboBoxHistory(m_ui.TuningKeyMapFileComboBox);
		// Reset dirty flag.
		m_iDirtyTuning = 0;
	}

	if (m_iDirtyControls > 0 && pConfig && m_pSynthUi) {
		// Save controls...
		synthv1_controls *pControls = m_pSynthUi->controls();
		if (pControls) {
			m_ui.ControlsTreeWidget->saveControls(pControls);
			pConfig->saveControls(pControls);
			// Reset dirty flag.
			m_iDirtyControls = 0;
		}
	}

	if (m_iDirtyPrograms > 0 && pConfig && m_pSynthUi) {
		// Save programs...
		synthv1_programs *pPrograms = m_pSynthUi->programs();
		if (pPrograms) {
			m_ui.ProgramsTreeWidget->savePrograms(pPrograms);
			pConfig->savePrograms(pPrograms);
			// Reset dirty flag.
			m_iDirtyPrograms = 0;
		}
	}

	if (m_iDirtyOptions > 0 && pConfig) {
		// Save options...
		pConfig->bProgramsPreview = m_ui.ProgramsPreviewCheckBox->isChecked();
		pConfig->bUseNativeDialogs = m_ui.UseNativeDialogsCheckBox->isChecked();
		pConfig->bDontUseNativeDialogs = !pConfig->bUseNativeDialogs;
		pConfig->iKnobDialMode = m_ui.KnobDialModeComboBox->currentIndex();
		synthv1widget_dial::setDialMode(
			synthv1widget_dial::DialMode(pConfig->iKnobDialMode));
		pConfig->iKnobEditMode = m_ui.KnobEditModeComboBox->currentIndex();
		synthv1widget_edit::setEditMode(
			synthv1widget_edit::EditMode(pConfig->iKnobEditMode));
		const QString sOldCustomColorTheme = pConfig->sCustomColorTheme;
		if (m_ui.CustomColorThemeComboBox->currentIndex() > 0)
			pConfig->sCustomColorTheme = m_ui.CustomColorThemeComboBox->currentText();
		else
			pConfig->sCustomColorTheme.clear();
		const QString sOldCustomStyleTheme = pConfig->sCustomStyleTheme;
		if (m_ui.CustomStyleThemeComboBox->currentIndex() > 0)
			pConfig->sCustomStyleTheme = m_ui.CustomStyleThemeComboBox->currentText();
		else
			pConfig->sCustomStyleTheme.clear();
		pConfig->fRandomizePercent = float(m_ui.RandomizePercentSpinBox->value());
		int iNeedRestart = 0;
		if (pConfig->sCustomStyleTheme != sOldCustomStyleTheme) {
			if (pConfig->sCustomStyleTheme.isEmpty()) {
				++iNeedRestart;
			} else {
				QApplication::setStyle(
					QStyleFactory::create(pConfig->sCustomStyleTheme));
			}
 		}
 		if (pConfig->sCustomColorTheme != sOldCustomColorTheme) {
			if (pConfig->sCustomColorTheme.isEmpty()) {
				++iNeedRestart;
			} else {
				QPalette pal;
				if (synthv1widget_palette::namedPalette(
						pConfig, pConfig->sCustomColorTheme, pal))
					QApplication::setPalette(pal);
			}
 		}
		// Show restart message if needed...
 		if (iNeedRestart > 0) {
			QMessageBox::information(this,
				tr("Information"),
				tr("Some settings may be only effective\n"
				"next time you start this application."));
		}
		// Reset dirty flag.
		m_iDirtyOptions = 0;
	}

	// Just go with dialog acceptance.
	QDialog::accept();
}


void synthv1widget_config::reject (void)
{
	bool bReject = true;

	// Check if there's any pending changes...
	if (m_iDirtyTuning   > 0 ||
		m_iDirtyControls > 0 ||
		m_iDirtyPrograms > 0 ||
		m_iDirtyOptions  > 0) {
		QMessageBox::StandardButtons buttons
			= QMessageBox::Discard | QMessageBox::Cancel;
		if (m_ui.DialogButtonBox->button(QDialogButtonBox::Ok)->isEnabled())
			buttons |= QMessageBox::Apply;
		switch (QMessageBox::warning(this,
			tr("Warning"),
			tr("Some settings have been changed.\n\n"
			"Do you want to apply the changes?"),
			buttons)) {
		case QMessageBox::Apply:
			accept();
			return;
		case QMessageBox::Discard:
			break;
		default:    // Cancel.
			bReject = false;
		}
	}

	if (bReject)
		QDialog::reject();
}


// Custom color palette theme manager.
void synthv1widget_config::editCustomColorThemes (void)
{
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig == nullptr)
		return;

	synthv1widget_palette form(this);
	form.setSettings(pConfig);

	QString sCustomColorTheme;
	int iDirtyCustomColorTheme = 0;

	const int iCustomColorTheme
		= m_ui.CustomColorThemeComboBox->currentIndex();
	if (iCustomColorTheme > 0) {
		sCustomColorTheme = m_ui.CustomColorThemeComboBox->itemText(iCustomColorTheme);
		form.setPaletteName(sCustomColorTheme);
	}

	if (form.exec() == QDialog::Accepted) {
		sCustomColorTheme = form.paletteName();
		++iDirtyCustomColorTheme;
	}

	if (iDirtyCustomColorTheme > 0 || form.isDirty()) {
		resetCustomColorThemes(sCustomColorTheme);
		optionsChanged();
	}
}


// Custom color palette themes settler.
void synthv1widget_config::resetCustomColorThemes (
	const QString& sCustomColorTheme )
{
	m_ui.CustomColorThemeComboBox->clear();
	m_ui.CustomColorThemeComboBox->addItem(
		tr(g_pszDefName));
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig) m_ui.CustomColorThemeComboBox->addItems(
		synthv1widget_palette::namedPaletteList(pConfig));

	int iCustomColorTheme = 0;
	if (!sCustomColorTheme.isEmpty()) {
		iCustomColorTheme = m_ui.CustomColorThemeComboBox->findText(
			sCustomColorTheme);
		if (iCustomColorTheme < 0)
			iCustomColorTheme = 0;
	}
	m_ui.CustomColorThemeComboBox->setCurrentIndex(iCustomColorTheme);
}


// Custom widget style themes settler.
void synthv1widget_config::resetCustomStyleThemes (
	const QString& sCustomStyleTheme )
{
	m_ui.CustomStyleThemeComboBox->clear();
	m_ui.CustomStyleThemeComboBox->addItem(
		tr(g_pszDefName));
	m_ui.CustomStyleThemeComboBox->addItems(
		QStyleFactory::keys());

	int iCustomStyleTheme = 0;
	if (!sCustomStyleTheme.isEmpty()) {
		iCustomStyleTheme = m_ui.CustomStyleThemeComboBox->findText(
			sCustomStyleTheme);
		if (iCustomStyleTheme < 0)
			iCustomStyleTheme = 0;
	}
	m_ui.CustomStyleThemeComboBox->setCurrentIndex(iCustomStyleTheme);
}


// Combo box history persistence helper implementation.
void synthv1widget_config::loadComboBoxHistory ( QComboBox *pComboBox )
{
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig == nullptr)
		return;

	// Load combobox list from configuration settings file...
	const bool bBlockSignals = pComboBox->blockSignals(true);
	pConfig->beginGroup("/History");
	const QStringList& history
		= pConfig->value('/' + pComboBox->objectName()).toStringList();
	QStringListIterator iter(history);
	while (iter.hasNext()) {
		const QFileInfo info(iter.next());
		if (info.exists() && info.isReadable()) {
			const QString& sPath = info.canonicalFilePath();
			pComboBox->insertItem(0, info.fileName(), sPath);
		}
	}
	pConfig->endGroup();
	pComboBox->blockSignals(bBlockSignals);
}


void synthv1widget_config::saveComboBoxHistory ( QComboBox *pComboBox )
{
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig == nullptr)
		return;

	// Save combobox list to configuration settings file...
	const bool bBlockSignals = pComboBox->blockSignals(true);
	pConfig->beginGroup("/History");
	QStringList history;
	const int iCount = pComboBox->count();
	for (int i = 0; i < iCount; ++i) {
		const QString& sData = pComboBox->itemData(i).toString();
		if (!sData.isEmpty())
			history.prepend(sData);
	}
	pConfig->setValue('/' + pComboBox->objectName(), history);
	pConfig->endGroup();
	pComboBox->blockSignals(bBlockSignals);
}


// Combo box settter/gettter helper prototypes.
bool synthv1widget_config::setComboBoxCurrentItem (
	QComboBox *pComboBox, const QFileInfo& info )
{
	const bool bBlockSignals = pComboBox->blockSignals(true);
	const bool bResult = info.exists() && info.isReadable();
	if (bResult) {
		const QString& sData = info.canonicalFilePath();
		int iIndex = pComboBox->findData(sData);
		if (iIndex < 0) {
			pComboBox->insertItem(0, info.fileName(), sData);
			iIndex = 0;
		}
		pComboBox->setCurrentIndex(iIndex);
		pComboBox->setToolTip(sData);
	} else {
		pComboBox->setCurrentIndex(pComboBox->count() - 1);
		pComboBox->setToolTip(pComboBox->currentText());
	}
	pComboBox->blockSignals(bBlockSignals);

	return bResult;
}


QString synthv1widget_config::comboBoxCurrentItem ( QComboBox *pComboBox )
{
	QString sData;

	const int iIndex = pComboBox->currentIndex();
	if (iIndex >= 0)
		sData = pComboBox->itemData(iIndex).toString();

	return sData;
}


// end of synthv1widget_config.cpp
