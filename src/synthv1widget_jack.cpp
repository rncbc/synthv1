// synthv1widget_jack.cpp
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

#include "synthv1widget_jack.h"

#include "synthv1_jack.h"

#ifdef CONFIG_NSM
#include "synthv1_nsm.h"
#endif

#include <QApplication>
#include <QFileInfo>
#include <QDir>

#include <QCloseEvent>

#include <QStyleFactory>

#ifndef CONFIG_LIBDIR
#if defined(__x86_64__)
#define CONFIG_LIBDIR CONFIG_PREFIX "/lib64"
#else
#define CONFIG_LIBDIR CONFIG_PREFIX "/lib"
#endif
#endif

#if QT_VERSION < 0x050000
#define CONFIG_PLUGINSDIR CONFIG_LIBDIR "/qt4/plugins"
#else
#define CONFIG_PLUGINSDIR CONFIG_LIBDIR "/qt5/plugins"
#endif


//-------------------------------------------------------------------------
// synthv1widget_jack - impl.
//

// Constructor.
synthv1widget_jack::synthv1widget_jack ( synthv1_jack *pSynth )
	: synthv1widget(), m_pSynth(pSynth)
	#ifdef CONFIG_NSM
		, m_pNsmClient(NULL)
	#endif
{
	// Special style paths...
	if (QDir(CONFIG_PLUGINSDIR).exists())
		QApplication::addLibraryPath(CONFIG_PLUGINSDIR);

	// Custom style theme...
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig && !pConfig->sCustomStyleTheme.isEmpty())
		QApplication::setStyle(QStyleFactory::create(pConfig->sCustomStyleTheme));

	// Initialize (user) interface stuff...
	m_pSynthUi = new synthv1_ui(m_pSynth, false);

	// May initialize the scheduler/work notifier.
	openSchedNotifier();

	// Initialize preset stuff...
	// initPreset();
	resetParamKnobs();
	resetParamValues();
}


// Destructor.
synthv1widget_jack::~synthv1widget_jack (void)
{
	delete m_pSynthUi;
}


// Synth engine accessor.
synthv1_ui *synthv1widget_jack::ui_instance (void) const
{
	return m_pSynthUi;
}

#ifdef CONFIG_NSM

// NSM client accessors.
void synthv1widget_jack::setNsmClient ( synthv1_nsm *pNsmClient )
{
	m_pNsmClient = pNsmClient;
}

synthv1_nsm *synthv1widget_jack::nsmClient (void) const
{
	return m_pNsmClient;
}

#endif	// CONFIG_NSM


// Param port method.
void synthv1widget_jack::updateParam (
	synthv1::ParamIndex index, float fValue ) const
{
	m_pSynthUi->setParamValue(index, fValue);
}


// Dirty flag method.
void synthv1widget_jack::updateDirtyPreset ( bool bDirtyPreset )
{
	synthv1widget::updateDirtyPreset(bDirtyPreset);

#ifdef CONFIG_NSM
	if (m_pNsmClient && m_pNsmClient->is_active())
		m_pNsmClient->dirty(true); // as far as NSM goes, we're always filthy!
#endif
}


// Application close.
void synthv1widget_jack::closeEvent ( QCloseEvent *pCloseEvent )
{
#ifdef CONFIG_NSM
	if (m_pNsmClient && m_pNsmClient->is_active())
		synthv1widget::updateDirtyPreset(false);
#endif

	// Let's be sure about that...
	if (queryClose()) {
		pCloseEvent->accept();
		QApplication::quit();
	} else {
		pCloseEvent->ignore();
	}
}


#ifdef CONFIG_NSM

// Optional GUI handlers.
void synthv1widget_jack::showEvent ( QShowEvent *pShowEvent )
{
	QWidget::showEvent(pShowEvent);

	if (m_pNsmClient)
		m_pNsmClient->visible(true);
}

void synthv1widget_jack::hideEvent ( QHideEvent *pHideEvent )
{
	if (m_pNsmClient)
		m_pNsmClient->visible(false);

	QWidget::hideEvent(pHideEvent);
}

#endif	// CONFIG_NSM


// end of synthv1widget_jack.cpp
