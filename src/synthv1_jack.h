// synthv1_jack.h
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

#ifndef __synthv1_jack_h
#define __synthv1_jack_h

#include "synthv1.h"

#include <jack/jack.h>


#ifdef CONFIG_ALSA_MIDI
#include <jack/ringbuffer.h>
#include <alsa/asoundlib.h>
// forward decls.
class synthv1_alsa_thread;
#endif


//-------------------------------------------------------------------------
// synthv1_jack - decl.
//

class synthv1_jack : public synthv1
{
public:

	synthv1_jack(const char *client_name);

	~synthv1_jack();

	jack_client_t *client() const;

	void open(const char *client_name);
	void close();

	void activate();
	void deactivate();

	int process(jack_nframes_t nframes);

#ifdef CONFIG_ALSA_MIDI
	snd_seq_t *alsa_seq() const;
	void alsa_capture(snd_seq_event_t *ev);
#endif

#ifdef CONFIG_JACK_SESSION
	// JACK session event handler.
	void sessionEvent(void *pvSessionArg);
#endif

	void shutdown();
	void shutdown_close();

protected:
 
	void updatePreset(bool bDirty);
	void updateParam(synthv1::ParamIndex index);
	void updateParams();
	void updateTuning();

private:

	jack_client_t *m_client;

	volatile bool m_activated;

	jack_port_t **m_audio_ins;
	jack_port_t **m_audio_outs;

	float **m_ins;
	float **m_outs;

	float m_params[synthv1::NUM_PARAMS];

#ifdef CONFIG_JACK_MIDI
	jack_port_t *m_midi_in;
#endif
#ifdef CONFIG_ALSA_MIDI
	snd_seq_t *m_alsa_seq;
//	int m_alsa_client;
	int m_alsa_port;
	snd_midi_event_t *m_alsa_decoder;
	jack_ringbuffer_t *m_alsa_buffer;
	synthv1_alsa_thread *m_alsa_thread;
#endif
};


//-------------------------------------------------------------------------
// synthv1_jack_application -- Singleton application instance.
//

#include <QObject>
#include <QStringList>


// forward decls.
class QCoreApplication;
class synthv1widget_jack;

#ifdef CONFIG_NSM
class synthv1_nsm;
#endif

#ifdef HAVE_SIGNAL_H
class QSocketNotifier;
#endif

class synthv1_jack_application : public QObject
{
	Q_OBJECT

public:

	// Constructor.
	synthv1_jack_application(int& argc, char **argv);

	// Destructor.
	~synthv1_jack_application();

	// Facade method.
	int exec();

	// JACK shutdown handler.
	void shutdown();

	// Pseudo-singleton accessor.
	static synthv1_jack_application *getInstance();

signals:

	void shutdown_signal();

protected slots:

#ifdef CONFIG_NSM
	// NSM callback slots.
	void openSession();
	void saveSession();
	void hideSession();
	void showSession();
#endif	// CONFIG_NSM

#ifdef HAVE_SIGNAL_H
	// SIGTERM signal handler.
	void handle_sigterm();
#endif

	void shutdown_slot();

protected:

	// Argument parser method.
	bool parse_args();
#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
	void show_error(const QString& msg);
#endif

	// Startup method.
	bool setup();

private:

	// Instance variables.
	QCoreApplication *m_pApp;
	bool m_bGui;

	QString m_sClientName;
	QStringList m_presets;

	synthv1_jack *m_pSynth;
	synthv1widget_jack *m_pWidget;

#ifdef CONFIG_NSM
	synthv1_nsm *m_pNsmClient;
#endif

#ifdef HAVE_SIGNAL_H
	QSocketNotifier *m_pSigtermNotifier;
#endif

	static synthv1_jack_application *g_pInstance;
};


#endif// __synthv1_jack_h

// end of synthv1_jack.h

