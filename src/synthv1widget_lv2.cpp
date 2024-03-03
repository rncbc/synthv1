// synthv1widget_lv2.cpp
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

#include "synthv1widget_lv2.h"

#include "synthv1_lv2.h"

#include <QApplication>
#include <QFileInfo>
#include <QDir>

#include "synthv1widget_palette.h"

#include <QCloseEvent>

#include <QStyleFactory>

#ifndef CONFIG_BINDIR
#define CONFIG_BINDIR	CONFIG_PREFIX "/bin"
#endif

#ifndef CONFIG_LIBDIR
#if defined(__x86_64__)
#define CONFIG_LIBDIR CONFIG_PREFIX "/lib64"
#else
#define CONFIG_LIBDIR CONFIG_PREFIX "/lib"
#endif
#endif

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
#define CONFIG_PLUGINSDIR CONFIG_LIBDIR "/qt5/plugins"
#else
#define CONFIG_PLUGINSDIR CONFIG_LIBDIR "/qt6/plugins"
#endif


//-------------------------------------------------------------------------
// synthv1widget_lv2 - impl.
//

synthv1widget_lv2::synthv1widget_lv2 ( synthv1_lv2 *pSynth,
	LV2UI_Controller controller, LV2UI_Write_Function write_function )
	: synthv1widget()
{
	// Check whether under a dedicated application instance...
	QApplication *pApp = synthv1_lv2::qapp_instance();
	if (pApp) {
		// Special style paths...
		QString sPluginsPath = pApp->applicationDirPath();
		sPluginsPath.remove(CONFIG_BINDIR);
		sPluginsPath.append(CONFIG_PLUGINSDIR);
		if (QDir(sPluginsPath).exists())
			pApp->addLibraryPath(CONFIG_PLUGINSDIR);
	}

	// Custom color/style themes...
	synthv1_config *pConfig = synthv1_config::getInstance();
	if (pConfig) {
		const QChar sep = QDir::separator();
		QString sPalettePath = QApplication::applicationDirPath();
		sPalettePath.remove(CONFIG_BINDIR);
		sPalettePath.append(CONFIG_DATADIR);
		sPalettePath.append(sep);
		sPalettePath.append(PROJECT_NAME);
		sPalettePath.append(sep);
		sPalettePath.append("palette");
		if (QDir(sPalettePath).exists()) {
			QStringList names;
			names.append("KXStudio");
			names.append("Wonton Soup");
			QStringListIterator name_iter(names);
			while (name_iter.hasNext()) {
				const QString& name = name_iter.next();
				const QFileInfo fi(sPalettePath, name + ".conf");
				if (fi.isReadable()) {
					synthv1widget_palette::addNamedPaletteConf(
						pConfig, name, fi.absoluteFilePath());
				}
			}
		}
		if (!pConfig->sCustomColorTheme.isEmpty()) {
			QPalette pal;
			if (synthv1widget_palette::namedPalette(
					pConfig, pConfig->sCustomColorTheme, pal))
				synthv1widget::setPalette(pal);
		}
		if (!pConfig->sCustomStyleTheme.isEmpty()) {
			synthv1widget::setStyle(
				QStyleFactory::create(pConfig->sCustomStyleTheme));
		}
	}

	// Initialize (user) interface stuff...
	m_pSynthUi = new synthv1_lv2ui(pSynth, controller, write_function);

#ifdef CONFIG_LV2_UI_EXTERNAL
	m_external_host = nullptr;
#endif
#ifdef CONFIG_LV2_UI_IDLE
	m_bIdleClosed = false;
#endif

	// Initialise preset stuff...
	clearPreset();

	// Initial update, always...
	//resetParamValues();
	resetParamKnobs();

	// May initialize the scheduler/work notifier.
	openSchedNotifier();
}


// Destructor.
synthv1widget_lv2::~synthv1widget_lv2 (void)
{
	delete m_pSynthUi;
}


// Synth engine accessor.
synthv1_ui *synthv1widget_lv2::ui_instance (void) const
{
	return m_pSynthUi;
}


#ifdef CONFIG_LV2_UI_EXTERNAL

void synthv1widget_lv2::setExternalHost ( LV2_External_UI_Host *external_host )
{
	m_external_host = external_host;
}

const LV2_External_UI_Host *synthv1widget_lv2::externalHost (void) const
{
	return m_external_host;
}

#endif	// CONFIG_LV2_UI_EXTERNAL


#ifdef CONFIG_LV2_UI_IDLE

bool synthv1widget_lv2::isIdleClosed (void) const
{
	return m_bIdleClosed;
}

#endif	// CONFIG_LV2_UI_IDLE


// Close event handler.
void synthv1widget_lv2::closeEvent ( QCloseEvent *pCloseEvent )
{
	synthv1widget::closeEvent(pCloseEvent);

#ifdef CONFIG_LV2_UI_IDLE
	if (pCloseEvent->isAccepted())
		m_bIdleClosed = true;
#endif
#ifdef CONFIG_LV2_UI_EXTERNAL
	if (m_external_host && m_external_host->ui_closed) {
		if (pCloseEvent->isAccepted())
			m_external_host->ui_closed(m_pSynthUi->controller());
	}
#endif
}


// LV2 port event dispatcher.
void synthv1widget_lv2::port_event ( uint32_t port_index,
	uint32_t buffer_size, uint32_t format, const void *buffer )
{
	if (format == 0 && buffer_size == sizeof(float)) {
		const synthv1::ParamIndex index
			= synthv1::ParamIndex(port_index - synthv1_lv2::ParamBase);
		const float fValue = *(float *) buffer;
		setParamValue(index, fValue);
	}
}


// Param method.
void synthv1widget_lv2::updateParam (
	synthv1::ParamIndex index, float fValue ) const
{
	m_pSynthUi->write_function(index, fValue);
}


// end of synthv1widget_lv2.cpp
