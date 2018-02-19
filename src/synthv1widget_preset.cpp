// synthv1widget_preset.cpp
//
/****************************************************************************
   Copyright (C) 2012-2018, rncbc aka Rui Nuno Capela. All rights reserved.

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
#include <QFileDialog>
#include <QUrl>


//-------------------------------------------------------------------------
// synthv1widget_preset - Custom edit-box widget.
//

// Constructor.
synthv1widget_preset::synthv1widget_preset ( QWidget *pParent )
	: QWidget (pParent)
{
	m_pNewButton    = new QToolButton();
	m_pOpenButton   = new QToolButton();
	m_pComboBox     = new QComboBox();
	m_pSaveButton   = new QToolButton();
	m_pDeleteButton = new QToolButton();
	m_pResetButton  = new QToolButton();

	m_pNewButton->setIcon(QIcon(":/images/presetNew.png"));
	m_pOpenButton->setIcon(QIcon(":/images/presetOpen.png"));
	m_pComboBox->setEditable(true);
	m_pComboBox->setMinimumWidth(240);
#if QT_VERSION >= 0x040200
	m_pComboBox->setCompleter(NULL);
#endif
	m_pComboBox->setInsertPolicy(QComboBox::NoInsert);
	m_pSaveButton->setIcon(QIcon(":/images/presetSave.png"));
	m_pDeleteButton->setIcon(QIcon(":/images/presetDelete.png"));
	m_pResetButton->setText("Reset");

	m_pNewButton->setToolTip(tr("New Preset"));
	m_pOpenButton->setToolTip(tr("Open Preset"));
	m_pSaveButton->setToolTip(tr("Save Preset"));
	m_pDeleteButton->setToolTip(tr("Delete Preset"));
	m_pResetButton->setToolTip(tr("Reset Preset"));

	QHBoxLayout *pHBoxLayout = new QHBoxLayout();
	pHBoxLayout->setMargin(2);
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
		SIGNAL(editTextChanged(const QString&)),
		SLOT(stabilizePreset()));
	QObject::connect(m_pComboBox,
		SIGNAL(activated(const QString&)),
		SLOT(activatePreset(const QString&)));
	QObject::connect(m_pSaveButton,
		SIGNAL(clicked()),
		SLOT(savePreset()));
	QObject::connect(m_pDeleteButton,
		SIGNAL(clicked()),
		SLOT(deletePreset()));
	QObject::connect(m_pResetButton,
		SIGNAL(clicked()),
		SLOT(resetPreset()));

	refreshPreset();
	stabilizePreset();
}


// Preset name/text accessors.
void synthv1widget_preset::clearPreset (void)
{
	++m_iInitPreset;

	const bool bBlockSignals = m_pComboBox->blockSignals(true);
	m_pComboBox->clearEditText();
	m_pComboBox->blockSignals(bBlockSignals);
}


void synthv1widget_preset::setPreset ( const QString& sPreset )
{
	const bool bBlockSignals = m_pComboBox->blockSignals(true);
	m_pComboBox->setEditText(sPreset);
	m_pComboBox->blockSignals(bBlockSignals);
}

QString synthv1widget_preset::preset (void) const
{
	return m_pComboBox->currentText();
}


// Check whether current preset may be reset.
bool synthv1widget_preset::queryPreset (void)
{
	if (m_iInitPreset == 0)
		return true;

	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig == NULL)
		return false;

	if (m_iDirtyPreset > 0) {
		const QString& sPreset(pConfig->sPreset);
		if (sPreset.isEmpty()) {
			if (QMessageBox::warning(this,
				tr("Warning") + " - " SYNTHV1_TITLE,
				tr("Some parameters have been changed.\n\n"
				"Do you want to discard the changes?"),
				QMessageBox::Discard |
				QMessageBox::Cancel) == QMessageBox::Cancel)
				return false;
		} else {
			switch (QMessageBox::warning(this,
				tr("Warning") + " - " SYNTHV1_TITLE,
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
	if (!sPreset.isEmpty() && queryPreset())
		loadPreset(sPreset);
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
		refreshPreset();
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
		refreshPreset();
	}

	stabilizePreset();
}


void synthv1widget_preset::openPreset (void)
{
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig == NULL)
		return;

	QStringList files;

	const QString  sExt(SYNTHV1_TITLE);
	const QString& sTitle  = tr("Open Preset") + " - " SYNTHV1_TITLE;
	const QString& sFilter = tr("Preset files (*.%1)").arg(sExt);

	QWidget *pParentWidget = NULL;
	QFileDialog::Options options = 0;
	if (pConfig->bDontUseNativeDialogs) {
		options |= QFileDialog::DontUseNativeDialog;
		pParentWidget = QWidget::window();
	}
#if 1//QT_VERSION < 0x040400
	files = QFileDialog::getOpenFileNames(pParentWidget,
		sTitle, pConfig->sPresetDir, sFilter, NULL, options);
#else
	QFileDialog fileDialog(pParentWidget,
		sTitle, pConfig->sPresetDir, sFilter);
	fileDialog.setAcceptMode(QFileDialog::AcceptOpen);
	fileDialog.setFileMode(QFileDialog::ExistingFiles);
	fileDialog.setDefaultSuffix(sExt);
	QList<QUrl> urls(fileDialog.sidebarUrls());
	urls.append(QUrl::fromLocalFile(pConfig->sPresetDir));
	fileDialog.setSidebarUrls(urls);
	fileDialog.setOptions(options);
	if (fileDialog.exec())
		files = fileDialog.selectedFiles();
#endif

	if (!files.isEmpty() && queryPreset()) {
		int iPreset = 0;
		QStringListIterator iter(files);
		while (iter.hasNext()) {
			const QString& sFilename = iter.next();
			const QFileInfo fi(sFilename);
			if (fi.exists()) {
				const QString& sPreset = fi.completeBaseName();
				pConfig->setPresetFile(sPreset, sFilename);
				if (++iPreset == 1) {
					++m_iInitPreset;
					emit loadPresetFile(sFilename);
					pConfig->sPreset = sPreset;
					pConfig->sPresetDir = fi.absolutePath();
					setPreset(sPreset);
				}
			}
			refreshPreset();
		}
	}

	stabilizePreset();
}


void synthv1widget_preset::savePreset (void)
{
	savePreset(m_pComboBox->currentText());
}

void synthv1widget_preset::savePreset ( const QString& sPreset )
{
	if (sPreset.isEmpty())
		return;

	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig == NULL)
		return;

	const QString sExt(SYNTHV1_TITLE);
	QFileInfo fi(QDir(pConfig->sPresetDir), sPreset + '.' + sExt);
	QString sFilename = fi.absoluteFilePath();
	if (!fi.exists()) {
		const QString& sTitle  = tr("Save Preset") + " - " SYNTHV1_TITLE;
		const QString& sFilter = tr("Preset files (*.%1)").arg(sExt);
		QWidget *pParentWidget = NULL;
		QFileDialog::Options options = 0;
		if (pConfig->bDontUseNativeDialogs) {
			options |= QFileDialog::DontUseNativeDialog;
			pParentWidget = QWidget::window();
		}
	#if 1//QT_VERSION < 0x040400
		sFilename = QFileDialog::getSaveFileName(pParentWidget,
			sTitle, sFilename, sFilter, NULL, options);
	#else
		QFileDialog fileDialog(pParentWidget,
			sTitle, sFilename, sFilter);
		fileDialog.setAcceptMode(QFileDialog::AcceptSave);
		fileDialog.setFileMode(QFileDialog::AnyFile);
		fileDialog.setDefaultSuffix(sExt);
		QList<QUrl> urls(fileDialog.sidebarUrls());
		urls.append(QUrl::fromLocalFile(pConfig->sPresetDir));
		fileDialog.setSidebarUrls(urls);
		fileDialog.setOptions(options);
		if (fileDialog.exec())
			sFilename = fileDialog.selectedFiles().first();
	#endif
	} else {
		if (QMessageBox::warning(QWidget::window(),
			tr("Warning") + " - " SYNTHV1_TITLE,
			tr("About to replace preset:\n\n"
			"\"%1\"\n\n"
			"Are you sure?")
			.arg(sPreset),
			QMessageBox::Ok | QMessageBox::Cancel)
			== QMessageBox::Cancel) {
			sFilename.clear();
		}
	}

	if (!sFilename.isEmpty()) {
		if (QFileInfo(sFilename).suffix() != sExt)
			sFilename += '.' + sExt;
		emit savePresetFile(sFilename);
		pConfig->setPresetFile(sPreset, sFilename);
		++m_iInitPreset;
		pConfig->sPreset = sPreset;
		pConfig->sPresetDir = QFileInfo(sFilename).absolutePath();
		refreshPreset();
	}

	stabilizePreset();
}


void synthv1widget_preset::deletePreset (void)
{
	const QString& sPreset = m_pComboBox->currentText();
	if (sPreset.isEmpty())
		return;

	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig == NULL)
		return;
	if (QMessageBox::warning(QWidget::window(),
		tr("Warning") + " - " SYNTHV1_TITLE,
		tr("About to remove preset:\n\n"
		"\"%1\"\n\n"
		"Are you sure?")
		.arg(sPreset),
		QMessageBox::Ok | QMessageBox::Cancel)
		== QMessageBox::Cancel)
		return;

	pConfig->removePreset(sPreset);
	pConfig->sPreset.clear();

	clearPreset();
	refreshPreset();
	stabilizePreset();
}


void synthv1widget_preset::resetPreset (void)
{
	const QString& sPreset = m_pComboBox->currentText();
	const bool bLoadPreset = (!sPreset.isEmpty()
		&& m_pComboBox->findText(sPreset) >= 0);

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
void synthv1widget_preset::refreshPreset (void)
{
	const bool bBlockSignals = m_pComboBox->blockSignals(true);

	const QString sOldPreset = m_pComboBox->currentText();
	const QIcon icon(":/images/synthv1_preset.png");

	m_pComboBox->clear();

	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig) {
		QStringListIterator iter(pConfig->presetList());
		while (iter.hasNext()) {
			const QString& sPreset = iter.next();
			m_pComboBox->addItem(icon, sPreset);
		}
		m_pComboBox->model()->sort(0);
	}

	const int iIndex = m_pComboBox->findText(sOldPreset);
	if (iIndex >= 0)
		m_pComboBox->setCurrentIndex(iIndex);
	else
		m_pComboBox->setEditText(sOldPreset);

	m_iDirtyPreset = 0;

	m_pComboBox->blockSignals(bBlockSignals);
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


// Dirty flag accessors.
void synthv1widget_preset::setDirtyPreset ( bool bDirtyPreset )
{
	if (bDirtyPreset) {
		++m_iDirtyPreset;
	} else {
		m_iDirtyPreset = 0;
	}

	stabilizePreset();
}


bool synthv1widget_preset::isDirtyPreset (void) const
{
	return (m_iDirtyPreset > 0);
}


void synthv1widget_preset::stabilizePreset (void)
{
	const QString& sPreset = m_pComboBox->currentText();

	const bool bEnabled = (!sPreset.isEmpty());
	const bool bExists  = (m_pComboBox->findText(sPreset) >= 0);
	const bool bDirty   = (m_iDirtyPreset > 0);

	m_pSaveButton->setEnabled(bEnabled && (!bExists || bDirty));
	m_pDeleteButton->setEnabled(bEnabled && bExists);
	m_pResetButton->setEnabled(bDirty);
}


// end of synthv1widget_preset.cpp
