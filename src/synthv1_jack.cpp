// synthv1_jack.cpp
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

#include "synthv1_jack.h"
#include "synthv1_config.h"
#include "synthv1_param.h"

#include "synthv1_programs.h"
#include "synthv1_controls.h"

#include <jack/midiport.h>

#include <cstdio>
#include <cstring>
#include <cmath>

#include <QCoreApplication>
#include <QDir>


#ifdef CONFIG_ALSA_MIDI

//-------------------------------------------------------------------------
// alsa input thread.

#include <QThread>


class synthv1_alsa_thread : public QThread
{
public:

	synthv1_alsa_thread(synthv1_jack *synth)
		: QThread(), m_synth(synth), m_running(false) {}

	~synthv1_alsa_thread()
	{	// fake sync and wait
		if (m_running && isRunning()) do {
			m_running = false;
		}
		while (!wait(100));
	}

protected:

	void run()
	{
		snd_seq_t *seq = m_synth->alsa_seq();
		if (seq == nullptr)
			return;

		m_running = true;

		int nfds;
		struct pollfd *pfds;

		nfds = snd_seq_poll_descriptors_count(seq, POLLIN);
		pfds = (struct pollfd *) alloca(nfds * sizeof(struct pollfd));
		snd_seq_poll_descriptors(seq, pfds, nfds, POLLIN);

		int poll_rc = 0;

		while (m_running && poll_rc >= 0) {
			poll_rc = ::poll(pfds, nfds, 200);
			while (poll_rc > 0) {
				snd_seq_event_t *ev = nullptr;
				snd_seq_event_input(seq, &ev);
				m_synth->alsa_capture(ev);
			//	snd_seq_free_event(ev);
				poll_rc = snd_seq_event_input_pending(seq, 0);
			}
		}

		m_running = false;
	}

private:

	synthv1_jack *m_synth;

	volatile bool m_running;
};

#endif	// CONFIG_ALSA_MIDI


//-------------------------------------------------------------------------
// JACK process callback.

static
int synthv1_jack_process ( jack_nframes_t nframes, void *arg )
{
	return static_cast<synthv1_jack *> (arg)->process(nframes);
}


//----------------------------------------------------------------------
// JACK buffer-size change callback.

static int synthv1_jack_buffer_size ( jack_nframes_t nframes, void *arg )
{
	static_cast<synthv1_jack *> (arg)->setBufferSize(nframes);

	return 0;
}


//----------------------------------------------------------------------
// JACK on-shutdown callback.

static void synthv1_jack_on_shutdown ( void *arg )
{
	static_cast<synthv1_jack *> (arg)->shutdown();
}


#ifdef CONFIG_JACK_SESSION

#include <jack/session.h>

#include <QDir>

//----------------------------------------------------------------------
// synthv1_jack_session_event -- JACK session event callabck
//

static void synthv1_jack_session_event (
	jack_session_event_t *pSessionEvent, void *pvArg )
{
	synthv1_jack *pSynth = static_cast<synthv1_jack *> (pvArg);

	if (pSynth)
		pSynth->sessionEvent(pSessionEvent);
}

#endif	// CONFIG_JACK_SESSION


//-------------------------------------------------------------------------
// synthv1_jack - impl.
//

synthv1_jack::synthv1_jack (const char *client_name) : synthv1(2)
{
	m_client = nullptr;

	m_activated = false;

	m_audio_ins = nullptr;
	m_audio_outs = nullptr;

	m_ins = m_outs = nullptr;

	::memset(m_params, 0, synthv1::NUM_PARAMS * sizeof(float));

#ifdef CONFIG_JACK_MIDI
	m_midi_in = nullptr;
#endif
#ifdef CONFIG_ALSA_MIDI
	m_alsa_seq     = nullptr;
//	m_alsa_client  = -1;
	m_alsa_port    = -1;
	m_alsa_decoder = nullptr;
	m_alsa_buffer  = nullptr;
	m_alsa_thread  = nullptr;
#endif

	synthv1::programs()->enabled(true);
	synthv1::controls()->enabled(true);

	open(client_name);
	activate();
}


synthv1_jack::~synthv1_jack (void)
{
	deactivate();
	close();
}


jack_client_t *synthv1_jack::client (void) const
{
	return m_client;
}


int synthv1_jack::process ( jack_nframes_t nframes )
{
	if (!m_activated)
		return 0;

	const uint16_t nchannels = synthv1::channels();
	float **ins = m_ins, **outs = m_outs;
	for (uint16_t k = 0; k < nchannels; ++k) {
		ins[k]  = static_cast<float *> (
			::jack_port_get_buffer(m_audio_ins[k], nframes));
		outs[k] = static_cast<float *> (
			::jack_port_get_buffer(m_audio_outs[k], nframes));
	}

	jack_position_t pos;
	jack_transport_query(m_client, &pos);
	if (pos.valid & JackPositionBBT) {
		const float host_bpm = float(pos.beats_per_minute);
		if (::fabsf(host_bpm - synthv1::tempo()) > 0.001f)
			synthv1::setTempo(host_bpm);
	}

	uint32_t ndelta = 0;

#ifdef CONFIG_JACK_MIDI
	void *midi_in = ::jack_port_get_buffer(m_midi_in, nframes);
	if (midi_in) {
		const uint32_t nevents = ::jack_midi_get_event_count(midi_in);
		for (uint32_t n = 0; n < nevents; ++n) {
			jack_midi_event_t event;
			::jack_midi_event_get(&event, midi_in, n);
			if (event.time > ndelta) {
				const uint32_t nread = event.time - ndelta;
				if (nread > 0) {
					synthv1::process(ins, outs, nread);
					for (uint16_t k = 0; k < nchannels; ++k) {
						ins[k]  += nread;
						outs[k] += nread;
					}
				}
				ndelta = event.time;
			}
			synthv1::process_midi(event.buffer, event.size);
		}
	}
#endif
#ifdef CONFIG_ALSA_MIDI
	const jack_nframes_t buffer_size = ::jack_get_buffer_size(m_client);
	const jack_nframes_t frame_time  = ::jack_last_frame_time(m_client);
	uint8_t event_buffer[1024];
	jack_midi_event_t event;
	while (::jack_ringbuffer_peek(m_alsa_buffer,
			(char *) &event, sizeof(event)) == sizeof(event)) {
		if (event.time > frame_time)
			break;
		jack_nframes_t event_time = frame_time - event.time;
		if (event_time > buffer_size)
			event_time = 0;
		else
			event_time = buffer_size - event_time;
		if (event_time > ndelta) {
			const uint32_t nread = event_time - ndelta;
			if (nread > 0) {
				synthv1::process(ins, outs, nread);
				for (uint16_t k = 0; k < nchannels; ++k) {
					ins[k]  += nread;
					outs[k] += nread;
				}
			}
			ndelta = event_time;
		}
		::jack_ringbuffer_read_advance(m_alsa_buffer, sizeof(event));
		::jack_ringbuffer_read(m_alsa_buffer, (char *) event_buffer, event.size);
		synthv1::process_midi(event_buffer, event.size);
	}
#endif // CONFIG_ALSA_MIDI

	if (nframes > ndelta)
		synthv1::process(ins, outs, nframes - ndelta);

	return 0;
}


#ifdef CONFIG_JACK_SESSION
#if defined(Q_CC_GNU) || defined(Q_CC_MINGW)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif
#endif

void synthv1_jack::open ( const char *client_name )
{
	// init param ports
	for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i) {
		const synthv1::ParamIndex index = synthv1::ParamIndex(i);
		m_params[i] = synthv1_param::paramDefaultValue(index);
		synthv1::setParamPort(index, &m_params[i]);
	}

	// open client
	m_client = ::jack_client_open(client_name, JackNullOption, nullptr);
	if (m_client == nullptr)
		return;

	// set sample rate
	synthv1::setSampleRate(float(jack_get_sample_rate(m_client)));
//	synthv1::reset();

	// register audio ports & buffers
	uint16_t nchannels = synthv1::channels();

	m_audio_ins  = new jack_port_t * [nchannels];
	m_audio_outs = new jack_port_t * [nchannels];

	m_ins  = new float * [nchannels];
	m_outs = new float * [nchannels];

	char port_name[32];
	for (uint16_t k = 0; k < nchannels; ++k) {
		::snprintf(port_name, sizeof(port_name), "in_%d", k + 1);
		m_audio_ins[k] = ::jack_port_register(m_client,
			port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
		m_ins[k] = nullptr;
		::snprintf(port_name, sizeof(port_name), "out_%d", k + 1);
		m_audio_outs[k] = ::jack_port_register(m_client,
			port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
		m_outs[k] = nullptr;
	}

	// register midi port
#ifdef CONFIG_JACK_MIDI
	m_midi_in = ::jack_port_register(m_client,
		"in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
#endif
#ifdef CONFIG_ALSA_MIDI
	m_alsa_seq     = nullptr;
//	m_alsa_client  = -1;
	m_alsa_port    = -1;
	m_alsa_decoder = nullptr;
	m_alsa_buffer  = nullptr;
	m_alsa_thread  = nullptr;
	// open alsa sequencer client...
	if (snd_seq_open(&m_alsa_seq, "hw", SND_SEQ_OPEN_INPUT, 0) >= 0) {
		snd_seq_set_client_name(m_alsa_seq, client_name);
	//	m_alsa_client = snd_seq_client_id(m_alsa_seq);
		m_alsa_port = snd_seq_create_simple_port(m_alsa_seq, "in",
			SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
			SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
		snd_midi_event_new(1024, &m_alsa_decoder);
		m_alsa_buffer = ::jack_ringbuffer_create(
			1024 * (sizeof(jack_midi_event_t) + 4));
		m_alsa_thread = new synthv1_alsa_thread(this);
		m_alsa_thread->start(QThread::TimeCriticalPriority);
	}
#endif	// CONFIG_ALSA_MIDI

	// setup any local, initial buffers...
	synthv1::setBufferSize(::jack_get_buffer_size(m_client) << 2);

	::jack_set_buffer_size_callback(m_client,
		synthv1_jack_buffer_size, this);

	::jack_on_shutdown(m_client,
		synthv1_jack_on_shutdown, this);

	// set process callbacks...
	::jack_set_process_callback(m_client,
		synthv1_jack_process, this);

#ifdef CONFIG_JACK_SESSION
	// JACK session event callback...
	::jack_set_session_callback(m_client,
		synthv1_jack_session_event, this);
#endif
}

#ifdef CONFIG_JACK_SESSION
#if defined(Q_CC_GNU) || defined(Q_CC_MINGW)
#pragma GCC diagnostic pop
#endif
#endif


void synthv1_jack::activate (void)
{
	if (!m_activated) {
		synthv1::reset();
		if (m_client) {
			::jack_activate(m_client);
			m_activated = true;
		}
	}
}

void synthv1_jack::deactivate (void)
{
	if (m_activated) {
		if (m_client) {
			m_activated = false;
			::jack_deactivate(m_client);
		}
	}
}


void synthv1_jack::close (void)
{
#ifdef CONFIG_ALSA_MIDI
	// close alsa sequencer client...
	if (m_alsa_seq) {
		if (m_alsa_thread) {
			delete m_alsa_thread;
			m_alsa_thread = nullptr;
		}
		if (m_alsa_buffer) {
			::jack_ringbuffer_free(m_alsa_buffer);
			m_alsa_buffer = nullptr;
		}
		if (m_alsa_decoder) {
			snd_midi_event_free(m_alsa_decoder);
			m_alsa_decoder = nullptr;
		}
		if (m_alsa_port >= 0) {
			snd_seq_delete_simple_port(m_alsa_seq, m_alsa_port);
			m_alsa_port = -1;
		}
		snd_seq_close(m_alsa_seq);
	//	m_alsa_client = -1;
		m_alsa_seq = nullptr;
	}
#endif

	if (m_client == nullptr)
		return;

#ifdef CONFIG_JACK_MIDI
	// unregister midi ports
	if (m_midi_in) {
		::jack_port_unregister(m_client, m_midi_in);
		m_midi_in = nullptr;
	}
#endif

	// unregister audio ports
	const uint16_t nchannels = synthv1::channels();

	for (uint16_t k = 0; k < nchannels; ++k) {
		if (m_audio_outs && m_audio_outs[k]) {
			::jack_port_unregister(m_client, m_audio_outs[k]);
			m_audio_outs[k] = nullptr;
		}
		if (m_outs && m_outs[k])
			m_outs[k] = nullptr;
		if (m_audio_ins && m_audio_ins[k]) {
			::jack_port_unregister(m_client, m_audio_ins[k]);
			m_audio_ins[k] = nullptr;
		}
		if (m_ins && m_ins[k])
			m_ins[k] = nullptr;
	}

	if (m_outs) {
		delete [] m_outs;
		m_outs = nullptr;
	}
	if (m_ins) {
		delete [] m_ins;
		m_ins = nullptr;
	}

	if (m_audio_outs) {
		delete [] m_audio_outs;
		m_audio_outs = nullptr;
	}
	if (m_audio_ins) {
		delete [] m_audio_ins;
		m_audio_ins = nullptr;
	}

	// close client
	::jack_client_close(m_client);
	m_client = nullptr;
}


#ifdef CONFIG_ALSA_MIDI

// alsa sequencer client.
snd_seq_t *synthv1_jack::alsa_seq (void) const
{
	return m_alsa_seq;
}

// alsa event capture.
void synthv1_jack::alsa_capture ( snd_seq_event_t *ev )
{
	if (m_alsa_decoder == nullptr)
		return;

	if (ev == nullptr)
		return;

	// ignored events...
	switch(ev->type) {
	case SND_SEQ_EVENT_OSS:
	case SND_SEQ_EVENT_CLIENT_START:
	case SND_SEQ_EVENT_CLIENT_EXIT:
	case SND_SEQ_EVENT_CLIENT_CHANGE:
	case SND_SEQ_EVENT_PORT_START:
	case SND_SEQ_EVENT_PORT_EXIT:
	case SND_SEQ_EVENT_PORT_CHANGE:
	case SND_SEQ_EVENT_PORT_SUBSCRIBED:
	case SND_SEQ_EVENT_PORT_UNSUBSCRIBED:
	case SND_SEQ_EVENT_USR0:
	case SND_SEQ_EVENT_USR1:
	case SND_SEQ_EVENT_USR2:
	case SND_SEQ_EVENT_USR3:
	case SND_SEQ_EVENT_USR4:
	case SND_SEQ_EVENT_USR5:
	case SND_SEQ_EVENT_USR6:
	case SND_SEQ_EVENT_USR7:
	case SND_SEQ_EVENT_USR8:
	case SND_SEQ_EVENT_USR9:
	case SND_SEQ_EVENT_BOUNCE:
	case SND_SEQ_EVENT_USR_VAR0:
	case SND_SEQ_EVENT_USR_VAR1:
	case SND_SEQ_EVENT_USR_VAR2:
	case SND_SEQ_EVENT_USR_VAR3:
	case SND_SEQ_EVENT_USR_VAR4:
	case SND_SEQ_EVENT_NONE:
		return;
	}

#ifdef CONFIG_DEBUG_0
	// - show (input) event for debug purposes...
	fprintf(stderr, "ALSA MIDI In: 0x%02x", ev->type);
	if (ev->type == SND_SEQ_EVENT_SYSEX) {
		fprintf(stderr, " SysEx {");
		unsigned char *data = (unsigned char *) ev->data.ext.ptr;
		for (unsigned int i = 0; i < ev->data.ext.len; ++i)
			fprintf(stderr, " %02x", data[i]);
		fprintf(stderr, " }\n");
	} else {
		for (unsigned int i = 0; i < sizeof(ev->data.raw8.d); ++i)
			fprintf(stderr, " %3d", ev->data.raw8.d[i]);
		fprintf(stderr, "\n");
	}
#endif

	const unsigned int nlimit = ::jack_ringbuffer_write_space(m_alsa_buffer);
	if (nlimit > sizeof(jack_midi_event_t) + 4) {
		unsigned char  ev_buff[nlimit];
		unsigned char *ev_data = &ev_buff[0] + sizeof(jack_midi_event_t);
		const int ev_size = snd_midi_event_decode(m_alsa_decoder,
			ev_data, nlimit - sizeof(jack_midi_event_t), ev);
		if (ev_size > 0) {
			jack_midi_event_t *ev_head = (jack_midi_event_t *) &ev_buff[0];
			ev_head->time = ::jack_frame_time(m_client);
			ev_head->size = ev_size;
			ev_head->buffer = (jack_midi_data_t *) ev_data;
			::jack_ringbuffer_write(m_alsa_buffer,
				(const char *) ev_buff, sizeof(jack_midi_event_t) + ev_size);
		}
		snd_midi_event_reset_decode(m_alsa_decoder);
	}
}

#endif	// CONFIG_ALSA_MIDI


#ifdef CONFIG_JACK_SESSION

#if defined(Q_CC_GNU) || defined(Q_CC_MINGW)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

// JACK session event handler.
void synthv1_jack::sessionEvent ( void *pvSessionArg )
{
	jack_session_event_t *pJackSessionEvent
		= (jack_session_event_t *) pvSessionArg;

#ifdef CONFIG_DEBUG
	qDebug("synthv1_jack::sessionEvent()"
		" type=%d client_uuid=\"%s\" session_dir=\"%s\"",
		int(pJackSessionEvent->type),
		pJackSessionEvent->client_uuid,
		pJackSessionEvent->session_dir);
#endif

	const bool bQuit = (pJackSessionEvent->type == JackSessionSaveAndQuit);

	const QString sSessionDir
		= QString::fromUtf8(pJackSessionEvent->session_dir);
	const QString sSessionName
		= QFileInfo(QFileInfo(sSessionDir).canonicalPath()).completeBaseName();
	const QString sSessionFile = sSessionName + '.' + SYNTHV1_TITLE;

	QStringList args;
	args << QCoreApplication::applicationFilePath();
	args << QString("\"${SESSION_DIR}%1\"").arg(sSessionFile);

	synthv1_param::savePreset(this,
		QFileInfo(sSessionDir, sSessionFile).absoluteFilePath(), true);

	const QByteArray aCmdLine = args.join(" ").toUtf8();
	pJackSessionEvent->command_line = ::strdup(aCmdLine.constData());

	::jack_session_reply(m_client, pJackSessionEvent);
	::jack_session_event_free(pJackSessionEvent);

	if (bQuit)
	#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
		QCoreApplication::exit(0);
	#else
		QCoreApplication::quit();
	#endif
}

#if defined(Q_CC_GNU) || defined(Q_CC_MINGW)
#pragma GCC diagnostic pop
#endif

#endif	// CONFIG_JACK_SESSION


void synthv1_jack::updatePreset ( bool /*bDirty*/ )
{
	// nothing to do here...
}


void synthv1_jack::updateParam ( synthv1::ParamIndex /*index*/ )
{
	// nothing to do here...
}


void synthv1_jack::updateParams (void)
{
	// nothing to do here...
}


void synthv1_jack::updateTuning (void)
{
	synthv1::resetTuning();
}


void synthv1_jack::shutdown (void)
{
	synthv1_jack_application *pApp = synthv1_jack_application::getInstance();
	if (pApp)
		pApp->shutdown();
}


void synthv1_jack::shutdown_close (void)
{
	m_activated = false;

	if (m_client) {
		::jack_client_close(m_client);
		m_client = nullptr;
	}

	close();
}


//-------------------------------------------------------------------------
// synthv1_jack_application -- Singleton application instance.
//

#include "synthv1widget_jack.h"

#include <QApplication>
#include <QTextStream>

#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)
#include <QCommandLineParser>
#include <QCommandLineOption>
#if defined(Q_OS_WINDOWS)
#include <QMessageBox>
#endif
#endif

#ifdef CONFIG_NSM
#include "synthv1_nsm.h"
#endif


#ifdef HAVE_SIGNAL_H

#include <QSocketNotifier>

#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <signal.h>

// File descriptor for SIGTERM notifier.
static int g_fdSigterm[2] = { -1, -1 };

// Unix SIGTERM signal handler.
static void synthv1_sigterm_handler ( int /*signo*/ )
{
	char c = 1;

	(void) (::write(g_fdSigterm[0], &c, sizeof(c)) > 0);
}

#endif	// HAVE_SIGNAL_H


// Constructor.
synthv1_jack_application::synthv1_jack_application ( int& argc, char **argv )
	: QObject(nullptr), m_pApp(nullptr), m_bGui(true),
		m_sClientName(SYNTHV1_TITLE), m_pSynth(nullptr), m_pWidget(nullptr)
	  #ifdef CONFIG_NSM
		, m_pNsmClient(nullptr)
	  #endif
{
#ifdef Q_WS_X11
	m_bGui = (::getenv("DISPLAY") != 0);
#endif
	for (int i = 1; i < argc; ++i) {
		const QString& sArg
			= QString::fromLocal8Bit(argv[i]);
		if (sArg == "-g" || sArg == "--no-gui")
			m_bGui = false;
	}

	if (m_bGui) {
	#if defined(Q_OS_LINUX)
		::setenv("QT_QPA_PLATFORM", "xcb", 0);
	#endif
	#if QT_VERSION >= QT_VERSION_CHECK(5, 6, 0)
	#if QT_VERSION <  QT_VERSION_CHECK(6, 0, 0)
		QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	#endif
	#endif
		QApplication *pApp = new QApplication(argc, argv);
	#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
		pApp->setApplicationDisplayName(SYNTHV1_TITLE);
		//	SYNTHV1_TITLE " - " + QObject::tr(SYNTHV1_SUBTITLE));
		QString sVersion(CONFIG_BUILD_VERSION);
		sVersion += '\n';
		sVersion += QString("Qt: %1").arg(qVersion());
	#if defined(QT_STATIC)
		sVersion += "-static";
	#endif
		QApplication::setApplicationVersion(sVersion);
	#endif
		m_pApp = pApp;
	} else {
		m_pApp = new QCoreApplication(argc, argv);
	}

#if QT_VERSION >= QT_VERSION_CHECK(5, 1, 0)
	m_pApp->setApplicationName(SYNTHV1_TITLE);
#endif
	
#ifdef HAVE_SIGNAL_H

	// Set to ignore any fatal "Broken pipe" signals.
	::signal(SIGPIPE, SIG_IGN);

	// Initialize file descriptors for SIGTERM socket notifier.
	::socketpair(AF_UNIX, SOCK_STREAM, 0, g_fdSigterm);
	m_pSigtermNotifier
		= new QSocketNotifier(g_fdSigterm[1], QSocketNotifier::Read, this);

	QObject::connect(m_pSigtermNotifier,
		SIGNAL(activated(int)),
		SLOT(handle_sigterm()));

	// Install SIGTERM signal handler.
	struct sigaction sigterm;
	sigterm.sa_handler = synthv1_sigterm_handler;
	sigemptyset(&sigterm.sa_mask);
	sigterm.sa_flags = 0;
	sigterm.sa_flags |= SA_RESTART;
	::sigaction(SIGTERM, &sigterm, nullptr);
	::sigaction(SIGQUIT, &sigterm, nullptr);

	// Ignore SIGHUP/SIGINT signals.
	::signal(SIGHUP, SIG_IGN);
	::signal(SIGINT, SIG_IGN);

#else

	m_pSigtermNotifier = nullptr;

#endif	// !HAVE_SIGNAL_H

	// Pseudo-singleton instance.
	g_pInstance = this;
}


// Destructor.
synthv1_jack_application::~synthv1_jack_application (void)
{
	g_pInstance = nullptr;

#ifdef HAVE_SIGNAL_H
	if (m_pSigtermNotifier) delete m_pSigtermNotifier;
#endif
#ifdef CONFIG_NSM
	if (m_pNsmClient) delete m_pNsmClient;
#endif
	if (m_pWidget) delete m_pWidget;
	if (m_pSynth) delete m_pSynth;
	if (m_pApp) delete m_pApp;
}


#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)

void synthv1_jack_application::show_error( const QString& msg )
{
#if defined(Q_OS_WINDOWS)
	QMessageBox::information(nullptr, QApplication::applicationName(), msg);
#else
	const QByteArray tmp = msg.toUtf8() + '\n';
	::fputs(tmp.constData(), stderr);
#endif
}

#endif


// Argument parser method.
bool synthv1_jack_application::parse_args (void)
{
	const QStringList& args = m_pApp->arguments();

#if QT_VERSION >= QT_VERSION_CHECK(5, 2, 0)

	QCommandLineParser parser;
	parser.setApplicationDescription(
		SYNTHV1_TITLE " - " + QObject::tr(SYNTHV1_SUBTITLE));

	parser.addOption({{"g", "no-gui"},
		QObject::tr("Disable the graphical user interface (GUI)")});
	parser.addOption({{"n", "client-name"},
		QObject::tr("Set the JACK client name (default: %1)")
			.arg(SYNTHV1_TITLE), "label"});
	parser.addHelpOption();
	parser.addVersionOption();
	parser.addPositionalArgument("preset-file",
		QObject::tr("Load preset file (.%1)").arg(SYNTHV1_TITLE),
		QObject::tr("[preset-file]"));
	parser.process(args);

	if (parser.isSet("no-gui")) {
		// Ignored: parsed on startup...
	}

	if (parser.isSet("client-name")) {
		const QString& sVal = parser.value("client-name");
		if (sVal.isEmpty()) {
			show_error(QObject::tr("Option -n requires an argument (label)."));
			return false;
		}
		m_sClientName = sVal;
	}

	foreach(const QString& sArg, parser.positionalArguments()) {
		m_presets.append(sArg);
	}

#else

	QTextStream out(stderr);
	const int argc = args.count();

	for (int i = 1; i < argc; ++i) {

		QString sArg = args.at(i);

		QString sVal;
		const int iEqual = sArg.indexOf('=');
		if (iEqual >= 0) {
			sVal = sArg.right(sArg.length() - iEqual - 1);
			sArg = sArg.left(iEqual);
		}
		else if (i < argc - 1) {
			sVal = args.at(i + 1);
			if (sVal.at(0) == '-')
				sVal.clear();
		}

		if (sArg == "-n" || sArg == "--client-name") {
			if (sVal.isNull()) {
				out << QObject::tr("Option -n requires an argument (label).\n\n");
				return false;
			}
			m_sClientName = sVal;
			if (iEqual < 0)
				++i;
		}
		else
		if (sArg == "-h" || sArg == "--help") {
			const QString sEot = "\n\t";
			const QString sEol = "\n\n";
			out << QObject::tr("Usage: %1 [options]").arg(args.at(0)) + sEol;
			out << SYNTHV1_TITLE " - " << QObject::tr(SYNTHV1_SUBTITLE) + sEol;
			out << QObject::tr("Options:") + sEol;
			out << "  -g, --no-gui" + sEot +
				QObject::tr("Disable the graphical user interface (GUI)") + sEol;
			out << "  -n, --client-name=[label]" + sEot +
				QObject::tr("Set the JACK client name (default: %1)").arg(SYNTHV1_TITLE) + sEol;
			out << "  -h, --help" + sEot +
				QObject::tr("Show help about command line options.") + sEol;
			out << "  -v, --version" + sEot +
				QObject::tr("Show version information.") + sEol;
			return false;
		}
		else
		if (sArg == "-v" || sArg == "-V" || sArg == "--version") {
			out << QString("Qt: %1").arg(qVersion());
		#if defined(QT_STATIC)
			out << "-static";
		#endif
			out << '\n';
			out << QString("%1: %2\n")
				.arg(SYNTHV1_TITLE)
				.arg(CONFIG_BUILD_VERSION);
			return false;
		}
		else {
			// If we don't have one by now,
			// this will be the startup preset file...
			m_presets.append(sArg);
		}
	}

#endif

	return true;
}


// Startup methods.
bool synthv1_jack_application::setup (void)
{
	if (m_pApp == nullptr)
		return false;

	if (!parse_args()) {
		m_pApp->quit();
		return false;
	}

	QObject::connect(this,
		SIGNAL(shutdown_signal()),
		SLOT(shutdown_slot()));

	const QByteArray aClientName
		= m_sClientName.toLocal8Bit();
	const char *client_name
		= aClientName.constData();

	m_pSynth = new synthv1_jack(client_name);

	if (m_bGui) {
		m_pWidget = new synthv1widget_jack(m_pSynth);
	//	m_pWidget->show();
		if (m_presets.isEmpty())
			m_pWidget->initPreset();
		else
			m_pWidget->loadPreset(m_presets.first());
	}
	else
	if (!m_presets.isEmpty())
		synthv1_param::loadPreset(m_pSynth, m_presets.first());

#ifdef CONFIG_NSM
	// Check whether to participate into a NSM session...
	const QString& nsm_url
		= QString::fromLatin1(::getenv("NSM_URL"));
	if (!nsm_url.isEmpty()) {
		m_pNsmClient = new synthv1_nsm(nsm_url);
		QObject::connect(m_pNsmClient,
			SIGNAL(open()),
			SLOT(openSession()));
		QObject::connect(m_pNsmClient,
			SIGNAL(save()),
			SLOT(saveSession()));
		QObject::connect(m_pNsmClient,
			SIGNAL(show()),
			SLOT(showSession()));
		QObject::connect(m_pNsmClient,
			SIGNAL(hide()),
			SLOT(hideSession()));
		QString caps(":switch:dirty:");
		if (m_bGui)
			caps += "optional-gui:";
		m_pNsmClient->announce(SYNTHV1_TITLE, caps.toLocal8Bit().constData());
		if (m_pWidget)
			m_pWidget->setNsmClient(m_pNsmClient);
	}
	else
#endif	// CONFIG_NSM
	if (m_pWidget)
		m_pWidget->show();

	return true;
}


// Facade method.
int synthv1_jack_application::exec (void)
{
	return (setup() ? m_pApp->exec() : 1);
}


#ifdef CONFIG_NSM

void synthv1_jack_application::openSession (void)
{
	if (m_pSynth == nullptr)
		return;

	if (m_pNsmClient == nullptr)
		return;

	if (!m_pNsmClient->is_active())
		return;

#ifdef CONFIG_DEBUG
	qDebug("synthv1_jack::openSession()");
#endif

	m_pSynth->deactivate();
	m_pSynth->close();

	const QString& client_name = m_pNsmClient->client_name();
	const QString& path_name = m_pNsmClient->path_name();
	const QString& display_name = m_pNsmClient->display_name();

	m_pSynth->open(client_name.toUtf8().constData());
	m_pSynth->activate();

	const QDir dir(path_name);
	if (!dir.exists())
		dir.mkpath(path_name);

	QFileInfo fi(path_name, "session." SYNTHV1_TITLE);
	if (!fi.exists())
		fi.setFile(path_name, display_name + '.' + SYNTHV1_TITLE);
	if (fi.exists()) {
		const QString& sFilename = fi.absoluteFilePath();
		if (m_pWidget) {
			m_pWidget->loadPreset(sFilename);
		} else {
			synthv1_param::loadPreset(m_pSynth, sFilename);
		}
	}

	m_pNsmClient->open_reply();
	m_pNsmClient->dirty(false);

	if (m_pWidget)
		m_pNsmClient->visible(m_pWidget->isVisible());
}

void synthv1_jack_application::saveSession (void)
{
	if (m_pSynth == nullptr)
		return;

	if (m_pNsmClient == nullptr)
		return;

	if (!m_pNsmClient->is_active())
		return;

#ifdef CONFIG_DEBUG
	qDebug("synthv1_jack::saveSession()");
#endif

//	const QString& client_name = m_pNsmClient->client_name();
	const QString& path_name = m_pNsmClient->path_name();
//	const QString& display_name = m_pNsmClient->display_name();
//	const QFileInfo fi(path_name, display_name + '.' + SYNTHV1_TITLE);
	const QFileInfo fi(path_name, "session." SYNTHV1_TITLE);

	synthv1_param::savePreset(m_pSynth, fi.absoluteFilePath(), true);

	m_pNsmClient->save_reply();
	m_pNsmClient->dirty(false);
}


void synthv1_jack_application::showSession (void)
{
	if (m_pNsmClient == nullptr)
		return;

	if (!m_pNsmClient->is_active())
		return;

#ifdef CONFIG_DEBUG
	qDebug("synthv1_jack::showSession()");
#endif

	if (m_pWidget) {
		m_pWidget->show();
		m_pWidget->raise();
		m_pWidget->activateWindow();
	}
}

void synthv1_jack_application::hideSession (void)
{
	if (m_pNsmClient == nullptr)
		return;

	if (!m_pNsmClient->is_active())
		return;

#ifdef CONFIG_DEBUG
	qDebug("synthv1_jack::hideSession()");
#endif

	if (m_pWidget)
		m_pWidget->hide();
}


#endif	// CONFIG_NSM


#ifdef HAVE_SIGNAL_H

// SIGTERM signal handler.
void synthv1_jack_application::handle_sigterm (void)
{
	char c;

	if (::read(g_fdSigterm[1], &c, sizeof(c)) > 0) {
		if (m_pApp && m_pWidget) {
		#ifdef CONFIG_NSM
			if (m_pNsmClient && m_pNsmClient->is_active())
				m_pWidget->updateDirtyPreset(false);
		#endif
			if (m_pWidget->queryClose())
				m_pApp->quit();
		}
	}
}

#endif	// HAVE_SIGNAL_H


// JACK shutdown handlers.
void synthv1_jack_application::shutdown (void)
{
	emit shutdown_signal();
}


void synthv1_jack_application::shutdown_slot (void)
{
	bool bQuit = true;

	if (m_pSynth)
		m_pSynth->shutdown_close();

	if (m_pWidget)
		bQuit = m_pWidget->queryClose();

	if (m_pApp && bQuit)
		m_pApp->quit();
}


// Pseudo-singleton instance.
synthv1_jack_application *synthv1_jack_application::g_pInstance = nullptr;

synthv1_jack_application *synthv1_jack_application::getInstance (void)
{
	return g_pInstance;
}


//-------------------------------------------------------------------------
// main

int main ( int argc, char *argv[] )
{
	Q_INIT_RESOURCE(synthv1);

	synthv1_jack_application app(argc, argv);

	return app.exec();
}


// end of synthv1_jack.cpp

