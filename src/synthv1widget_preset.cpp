// synthv1widget_preset.cpp
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

#include "synthv1widget_preset.h"

#include "synthv1_config.h"

#include <QHBoxLayout>

#include <QToolButton>
#include <QComboBox>

#include <QMessageBox>
#include <QFileInfo>


//-------------------------------------------------------------------------
// synthv1widget_preset - Custom preset-box widget.
//

// Constructor.
synthv1widget_preset::synthv1widget_preset ( QWidget *pParent )
	: QWidget(pParent)
{
	m_pNewButton    = new QToolButton();
	m_pOpenButton   = new QToolButton();
	m_pComboBox     = new synthv1widget_presets::ComboBox();
	m_pSaveButton   = new QToolButton();
	m_pDeleteButton = new QToolButton();
	m_pResetButton  = new QToolButton();

	m_pPresetsView = m_pComboBox->presetsView();

	m_pNewButton->setIcon(QIcon(":/images/presetNew.png"));
	m_pOpenButton->setIcon(QIcon(":/images/presetOpen.png"));
	m_pComboBox->setEditable(true);
	m_pComboBox->setValidator(m_pPresetsView->presetsValidator());
	m_pComboBox->setCompleter(nullptr);
	m_pComboBox->setMinimumWidth(240);
	m_pSaveButton->setIcon(QIcon(":/images/presetSave.png"));
	m_pDeleteButton->setIcon(QIcon(":/images/presetDelete.png"));
	m_pResetButton->setText("Reset");

	m_pNewButton->setToolTip(tr("New Preset"));
	m_pOpenButton->setToolTip(tr("Open Preset"));
	m_pSaveButton->setToolTip(tr("Save Preset"));
	m_pDeleteButton->setToolTip(tr("Delete Preset"));
	m_pResetButton->setToolTip(tr("Reset Preset"));

	QHBoxLayout *pHBoxLayout = new QHBoxLayout();
	pHBoxLayout->setContentsMargins(2, 2, 2, 2);
	pHBoxLayout->setSpacing(4);
	pHBoxLayout->addWidget(m_pNewButton);
	pHBoxLayout->addWidget(m_pOpenButton);
	pHBoxLayout->addWidget(m_pComboBox);
	pHBoxLayout->addWidget(m_pSaveButton);
	pHBoxLayout->addWidget(m_pDeleteButton);
	pHBoxLayout->addSpacing(4);
	pHBoxLayout->addWidget(m_pResetButton);
	QWidget::setLayout(pHBoxLayout);

	m_iInitPreset  = 0;
	m_iDirtyPreset = 0;

	// UI signal/slot connections...
	QObject::connect(m_pNewButton,
		SIGNAL(clicked()),
		SLOT(newPreset()));
	QObject::connect(m_pOpenButton,
		SIGNAL(clicked()),
		SLOT(openPreset()));
	QObject::connect(m_pComboBox,
		SIGNAL(currentTextChanged(const QString&)),
		SLOT(activatePreset(const QString&)));
#if 0
	QObject::connect(m_pComboBox,
	#if QT_VERSION >= QT_VERSION_CHECK(5, 14, 0)
		SIGNAL(textActivated(const QString&)),
	#else
		SIGNAL(activated(const QString&)),
	#endif
		SLOT(activatePreset(const QString&)));
#endif
	QObject::connect(m_pSaveButton,
		SIGNAL(clicked()),
		SLOT(savePreset()));
	QObject::connect(m_pDeleteButton,
		SIGNAL(clicked()),
		SLOT(deletePreset()));
	QObject::connect(m_pResetButton,
		SIGNAL(clicked()),
		SLOT(resetPreset()));

	loadPresets();
}


// Preset name/text accessors.
void synthv1widget_preset::clearPreset (void)
{
	++m_iInitPreset;

	const bool bBlockSignals
		= m_pComboBox->blockSignals(true);
	m_pComboBox->clearEditText();
	m_pComboBox->blockSignals(bBlockSignals);
}


void synthv1widget_preset::setPreset ( const QString& sPreset )
{
	const bool bBlockSignals
		= m_pComboBox->blockSignals(true);
	m_pComboBox->setCurrentPreset(sPreset);
	m_pComboBox->blockSignals(bBlockSignals);
}

QString synthv1widget_preset::preset (void) const
{
	return m_pComboBox->currentPreset();
}


// Check whether current preset may be reset.
bool synthv1widget_preset::queryPreset (void)
{
	if (m_iInitPreset == 0)
		return true;

	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig == nullptr)
		return false;

	if (m_iDirtyPreset > 0) {
		const QString& sPreset(pConfig->sPreset);
		if (sPreset.isEmpty()) {
			if (QMessageBox::warning(this,
				tr("Warning"),
				tr("Some parameters have been changed.\n\n"
				"Do you want to discard the changes?"),
				QMessageBox::Discard |
				QMessageBox::Cancel) == QMessageBox::Cancel)
				return false;
		} else {
			switch (QMessageBox::warning(this,
				tr("Warning"),
				tr("Some preset parameters have been changed:\n\n"
				"\"%1\".\n\nDo you want to save the changes?")
				.arg(sPreset),
				QMessageBox::Save |
				QMessageBox::Discard |
				QMessageBox::Cancel)) {
			case QMessageBox::Save:
				savePreset(sPreset);
				// Fall thru...
			case QMessageBox::Discard:
				break;
			default: // Cancel...
				setPreset(sPreset);
				return false;
			}
		}
	}

	return true;
}


// Preset management slots...
void synthv1widget_preset::activatePreset ( const QString& sPreset )
{
	QTreeWidgetItem *pPresetItem = m_pPresetsView->presetItem(sPreset);
	if (pPresetItem && queryPreset()) {
		loadPreset(sPreset);
		emit presetActivated(sPreset);
		return;
	}

	if (pPresetItem == nullptr && m_pPresetsView->bankItem(sPreset))
		m_pComboBox->resetCurrentPreset();

	stabilizePreset();
}


void synthv1widget_preset::loadPreset ( const QString& sPreset )
{
	if (sPreset.isEmpty())
		return;

	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig) {
		emit loadPresetFile(pConfig->presetFile(sPreset));
		++m_iInitPreset;
		pConfig->sPreset = sPreset;
		setPreset(sPreset);
	}

	stabilizePreset();
}


void synthv1widget_preset::newPreset (void)
{
	if (!queryPreset())
		return;

	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig) {
		emit newPresetFile();
		pConfig->sPreset.clear();
		clearPreset();
	}

	stabilizePreset();
}


void synthv1widget_preset::openPreset (void)
{
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig == nullptr)
		return;

	m_pPresetsView->setPresetDir(pConfig->sPresetDir);
	m_pPresetsView->setPresetExt(PROJECT_NAME);
	m_pPresetsView->setDontUseNativeDialogs(pConfig->bDontUseNativeDialogs);

	const QStringList& files
		= m_pPresetsView->openPresetFiles();

	if (files.isEmpty())
		return;

	if (!queryPreset())
		return;

	int iPreset = 0;
	QString sAfterPreset = m_pComboBox->currentPreset();
	QStringListIterator iter(files);
	while (iter.hasNext()) {
		const QString& sPresetFile = iter.next();
		const QFileInfo fi(sPresetFile);
		const QString& sPreset
			= m_pPresetsView->presetRenum(fi.completeBaseName());
		pConfig->setPresetFile(sPreset, sPresetFile);
		synthv1_presets::Preset *pPreset
			= pConfig->presets.add_preset(sPreset, sAfterPreset);
		if (pPreset) {
			pPreset->set_file(sPresetFile);
			if (++iPreset == 1) {
				++m_iInitPreset;
				emit loadPresetFile(sPresetFile);
				pConfig->sPreset = sPreset;
				pConfig->sPresetDir = fi.absolutePath();
			}
			sAfterPreset = sPreset;
		}
	}

	if (iPreset > 0) {
		m_pPresetsView->loadPresets(&(pConfig->presets));
		setPreset(pConfig->sPreset);
		m_iDirtyPreset = 0;
	}

	stabilizePreset();
}


void synthv1widget_preset::savePreset (void)
{
	savePreset(m_pComboBox->currentPreset());
}

void synthv1widget_preset::savePreset ( const QString& sPreset )
{
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig == nullptr)
		return;

	m_pPresetsView->setPresetDir(pConfig->sPresetDir);
	m_pPresetsView->setPresetExt(PROJECT_NAME);
	m_pPresetsView->setDontUseNativeDialogs(pConfig->bDontUseNativeDialogs);

	if (sPreset.isEmpty())
		return;

	const QString& sPresetFile
		= m_pPresetsView->savePresetFile(sPreset);

	if (sPresetFile.isEmpty()) {
		emit savePresetFile(sPresetFile);
		pConfig->setPresetFile(sPreset, sPresetFile);
		synthv1_presets::Preset *pPreset
			= pConfig->presets.add_preset(sPreset);
		if (pPreset) {
			pPreset->set_file(sPresetFile);
			++m_iInitPreset;
			pConfig->sPreset = sPreset;
			pConfig->sPresetDir = QFileInfo(sPresetFile).absolutePath();
			m_pPresetsView->loadPresets(&(pConfig->presets));
			setPreset(pConfig->sPreset);
			m_iDirtyPreset = 0;
		}
	}

	stabilizePreset();
}


void synthv1widget_preset::deletePreset (void)
{
	const QString& sPreset
		= m_pComboBox->currentPreset();
	if (sPreset.isEmpty())
		return;

	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig == nullptr)
		return;

	if (QMessageBox::warning(QWidget::window(),
		tr("Warning"),
		tr("About to remove preset:\n\n"
		"\"%1\"\n\n"
		"Are you sure?")
		.arg(sPreset),
		QMessageBox::Ok | QMessageBox::Cancel)
		== QMessageBox::Cancel)
		return;

	pConfig->presets.remove_preset(sPreset);
	pConfig->removePreset(sPreset);
	pConfig->sPreset.clear();

	m_pPresetsView->loadPresets(&(pConfig->presets));
	clearPreset();
//	m_iDirtyPreet = 0;

	stabilizePreset();
}


void synthv1widget_preset::resetPreset (void)
{
	const QString& sPreset
		= m_pComboBox->currentPreset();
	const bool bLoadPreset
		= (!sPreset.isEmpty() && m_pPresetsView->presetItem(sPreset) != nullptr);

	if (bLoadPreset && !queryPreset())
		return;

	if (bLoadPreset) {
		loadPreset(sPreset);
	} else {
		emit resetPresetFile();
		m_iDirtyPreset = 0;
		stabilizePreset();
	}
}


// Widget refreshner-loader.
void synthv1widget_preset::loadPresets (void)
{
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig == nullptr)
		return;

	m_pPresetsView->loadPresets(&(pConfig->presets));

	if (!pConfig->sPreset.isEmpty()) {
		if (m_pPresetsView->presetItem(pConfig->sPreset) != nullptr) {
			setPreset(pConfig->sPreset);
			m_iDirtyPreset = 0;
		} else {
			pConfig->sPreset.clear();
			clearPreset();
			++m_iDirtyPreset;
		}
	}

	stabilizePreset();
}


void synthv1widget_preset::savePresets (void)
{
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig == nullptr)
		return;

	if (m_pPresetsView->isDirtyPresets()) {
		m_pPresetsView->savePresets(&(pConfig->presets));
		m_pPresetsView->setDirtyPresets(false);
	}
}


// Preset control.
void synthv1widget_preset::initPreset (void)
{
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig && !pConfig->sPreset.isEmpty())
		loadPreset(pConfig->sPreset);
	else
		newPreset();
}


// Dirty flag accessor.
void synthv1widget_preset::setDirtyPreset ( bool bDirtyPreset )
{
	if (bDirtyPreset)
		++m_iDirtyPreset;
	else
		m_iDirtyPreset = 0;

	stabilizePreset();
}


bool synthv1widget_preset::isDirtyPreset (void) const
{
	return (m_iDirtyPreset > 0);
}


void synthv1widget_preset::stabilizePreset (void)
{
	const QString& sPreset = m_pComboBox->currentPreset();

	const bool bEnabled = (!sPreset.isEmpty());
	const bool bExists  = (m_pPresetsView->presetItem(sPreset) != nullptr);
	const bool bDirty   = (m_iDirtyPreset > 0);

	m_pSaveButton->setEnabled(bEnabled && (!bExists || bDirty));
	m_pDeleteButton->setEnabled(bEnabled && bExists);
	m_pResetButton->setEnabled(bDirty);
}


// end of synthv1widget_preset.cpp
