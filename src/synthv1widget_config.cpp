// synthv1widget_config.cpp
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

#include "synthv1widget_config.h"

#include <QPushButton>
#include <QMessageBox>

#include <QMenu>


//----------------------------------------------------------------------------
// synthv1widget_config -- UI wrapper form.

// ctor.
synthv1widget_config::synthv1widget_config (
	QWidget *pParent, Qt::WindowFlags wflags )
	: QDialog(pParent, wflags)
{
	// Setup UI struct...
	m_ui.setupUi(this);

	// Setup options...
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig)
		m_ui.UseNativeDialogsCheckBox->setChecked(pConfig->bUseNativeDialogs);

	// Signal/slots connections...
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
		SLOT(programsCurrentChanged(QTreeWidgetItem *, QTreeWidgetItem *)));
	QObject::connect(m_ui.ProgramsTreeWidget,
		SIGNAL(itemChanged(QTreeWidgetItem *, int)),
		SLOT(programsChanged()));
	QObject::connect(m_ui.ProgramsTreeWidget,
		SIGNAL(itemActivated(QTreeWidgetItem *, int)),
		SLOT(programsChanged()));
	QObject::connect(m_ui.ProgramsTreeWidget,
		SIGNAL(itemDoubleClicked(QTreeWidgetItem *, int)),
		SLOT(programsDoubleClicked(QTreeWidgetItem *, int)));

	// Custom context menu...
	m_ui.ProgramsTreeWidget->setContextMenuPolicy(Qt::CustomContextMenu);

	QObject::connect(m_ui.ProgramsTreeWidget,
		SIGNAL(customContextMenuRequested(const QPoint&)),
		SLOT(programsContextMenuRequested(const QPoint&)));

	// Options slots...
	QObject::connect(m_ui.UseNativeDialogsCheckBox,
		SIGNAL(toggled(bool)),
		SLOT(optionsChanged()));

	// Dialog commands...
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(accepted()),
		SLOT(accept()));
	QObject::connect(m_ui.DialogButtonBox,
		SIGNAL(rejected()),
		SLOT(reject()));

	// Programs database.
	m_pPrograms = NULL;

	// Dialog dirty flags.
	m_iDirtyPrograms = 0;
	m_iDirtyOptions  = 0;

	// Done.
	stabilize();
}


// dtor.
synthv1widget_config::~synthv1widget_config (void)
{
}


// programs accessors.
void synthv1widget_config::setPrograms ( synthv1_programs *pPrograms )
{
	// Programs database.
	m_pPrograms = pPrograms;

	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig && m_pPrograms) {
		// Load programs.
		m_ui.ProgramsTreeWidget->loadPrograms(m_pPrograms);
		// Selected current program, if any...
		m_ui.ProgramsTreeWidget->selectPrograms(m_pPrograms);
	}

	// Reset Dialog dirty flags.
	m_iDirtyPrograms = 0;

	stabilize();
}


synthv1_programs *synthv1widget_config::programs (void) const
{
	return m_pPrograms;
}


// command slots.
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


// janitor slots.
void synthv1widget_config::programsCurrentChanged ( QTreeWidgetItem *, QTreeWidgetItem * )
{
	stabilize();
}


void synthv1widget_config::programsDoubleClicked ( QTreeWidgetItem *pItem, int iColumn )
{
	if (pItem->parent() && iColumn > 0)
		accept();
}


void synthv1widget_config::programsContextMenuRequested ( const QPoint& pos )
{
	QTreeWidgetItem *pItem = m_ui.ProgramsTreeWidget->currentItem();

	QMenu menu(this);
	QAction *pAction;

	bool bEnabled = (m_pPrograms != NULL);

	pAction = menu.addAction(QIcon(":/images/presetBank.png"),
		tr("Add &Bank"), this, SLOT(programsAddBankItem()));
	pAction->setEnabled(bEnabled);

	pAction = menu.addAction(QIcon(":/images/presetItem.png"),
		tr("Add &Program"), this, SLOT(programsAddItem()));
	pAction->setEnabled(bEnabled);

	menu.addSeparator();

	bEnabled = bEnabled && (pItem != NULL);

	pAction = menu.addAction(QIcon(":/images/presetEdit.png"),
		tr("&Edit"), this, SLOT(programsEditItem()));
	pAction->setEnabled(bEnabled);

	menu.addSeparator();

	pAction = menu.addAction(QIcon(":/images/presetDelete.png"),
		tr("&Delete"), this, SLOT(programsDeleteItem()));
	pAction->setEnabled(bEnabled);

	menu.exec(m_ui.ProgramsTreeWidget->mapToGlobal(pos));
}


void synthv1widget_config::programsChanged (void)
{
	++m_iDirtyPrograms;

	stabilize();
}


void synthv1widget_config::optionsChanged (void)
{
	++m_iDirtyOptions;

	stabilize();
}


// stabilizer.
void synthv1widget_config::stabilize (void)
{
	QTreeWidgetItem *pItem = m_ui.ProgramsTreeWidget->currentItem();

	bool bEnabled = (m_pPrograms != NULL);
	m_ui.ProgramsAddBankToolButton->setEnabled(bEnabled);
	m_ui.ProgramsAddItemToolButton->setEnabled(bEnabled);
	bEnabled = bEnabled && (pItem != NULL);
	m_ui.ProgramsEditToolButton->setEnabled(bEnabled);
	m_ui.ProgramsDeleteToolButton->setEnabled(bEnabled);

	const bool bValid = (m_iDirtyPrograms > 0 || m_iDirtyOptions > 0);
	m_ui.DialogButtonBox->button(QDialogButtonBox::Ok)->setEnabled(bValid);
}


// dialog slots.
void synthv1widget_config::accept (void)
{
	synthv1_config *pConfig = synthv1_config::getInstance();

	if (m_iDirtyPrograms > 0 && pConfig && m_pPrograms) {
		// Save programs...
		m_ui.ProgramsTreeWidget->savePrograms(m_pPrograms);
		pConfig->savePrograms(m_pPrograms);
		// Reset dirty flag.
		m_iDirtyPrograms = 0;
	}

	if (m_iDirtyOptions > 0 && pConfig) {
		// Save options...
		pConfig->bUseNativeDialogs = m_ui.UseNativeDialogsCheckBox->isChecked();
		pConfig->bDontUseNativeDialogs = !pConfig->bUseNativeDialogs;
		pConfig->save();
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
	if (m_iDirtyPrograms > 0 || m_iDirtyOptions > 0) {
		QMessageBox::StandardButtons buttons
			= QMessageBox::Discard | QMessageBox::Cancel;
		if (m_ui.DialogButtonBox->button(QDialogButtonBox::Ok)->isEnabled())
			buttons |= QMessageBox::Apply;
		switch (QMessageBox::warning(this,
			tr("Warning") + " - " SYNTHV1_TITLE,
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


// end of synthv1widget_config.cpp
