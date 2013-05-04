// synthv1_jack.cpp
//
/****************************************************************************
   Copyright (C) 2012-2013, rncbc aka Rui Nuno Capela. All rights reserved.

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

#include <jack/midiport.h>

#include <stdio.h>
#include <string.h>


#ifdef CONFIG_ALSA_MIDI

//-------------------------------------------------------------------------
// alsa input thread.

#include <pthread.h>


class synthv1_alsa_thread
{
public:

	synthv1_alsa_thread(synthv1_jack *synth)
		: m_synth(synth), m_running(false) {}

	~synthv1_alsa_thread()
		{ m_running = false; wait(); }

	void start()
		{ pthread_create(&m_pthread, NULL, synthv1_alsa_thread::run, this); }

	void *run()
	{
		snd_seq_t *seq = m_synth->alsa_seq();
		if (seq == NULL)
			return NULL;

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
				snd_seq_event_t *ev = NULL;
				snd_seq_event_input(seq, &ev);
				m_synth->alsa_capture(ev);
			//	snd_seq_free_event(ev);
				poll_rc = snd_seq_event_input_pending(seq, 0);
			}
		}

		m_running = false;
		return NULL;
	}

	void wait()
		{ pthread_join(m_pthread, NULL); }

	void setRunning(bool running)
		{ m_running = running; }
	bool isRunning() const
		{ return m_running; }

protected:

	static void *run ( void *arg )
	{
		return static_cast<synthv1_alsa_thread *> (arg)->run();
	}

private:

	synthv1_jack *m_synth;

	bool m_running;

	pthread_t m_pthread;
};

#endif	// CONFIG_ALSA_MIDI


//-------------------------------------------------------------------------
// jack process callback.

static
int synthv1_jack_process ( jack_nframes_t nframes, void *arg )
{
	return static_cast<synthv1_jack *> (arg)->process(nframes);
}



//-------------------------------------------------------------------------
// synthv1_jack - impl.
//

synthv1_jack::synthv1_jack (void) : synthv1(2)
{
	m_client = NULL;

	m_audio_ins = NULL;
	m_audio_outs = NULL;

	m_ins = m_outs = NULL;

	::memset(m_params, 0, NUM_PARAMS * sizeof(float));

#ifdef CONFIG_JACK_MIDI
	m_midi_in = NULL;
#endif
#ifdef CONFIG_ALSA_MIDI
	m_alsa_seq     = NULL;
//	m_alsa_client  = -1;
	m_alsa_port    = -1;
	m_alsa_decoder = NULL;
	m_alsa_buffer  = NULL;
	m_alsa_thread  = NULL;
#endif

//	open(SYNTHV1_TITLE);
//	activate();
}


synthv1_jack::~synthv1_jack (void)
{
//	deactivate();
//	close();
}


jack_client_t *synthv1_jack::client (void) const
{
	return m_client;
}


int synthv1_jack::process ( jack_nframes_t nframes )
{
	const uint16_t nchannels = channels();
	float *ins[nchannels], *outs[nchannels];
	for (uint16_t k = 0; k < nchannels; ++k) {
		ins[k]  = static_cast<float *> (
			::jack_port_get_buffer(m_audio_ins[k], nframes));
		outs[k] = static_cast<float *> (
			::jack_port_get_buffer(m_audio_outs[k], nframes));
	}

	uint32_t ndelta = 0;

#ifdef CONFIG_JACK_MIDI
	void *midi_in = ::jack_port_get_buffer(m_midi_in, nframes);
	uint32_t nevents = ::jack_midi_get_event_count(midi_in);
	for (uint32_t n = 0; n < nevents; ++n) {
		jack_midi_event_t event;
		::jack_midi_event_get(&event, midi_in, n);
		uint32_t nread = event.time - ndelta;
		if (nread > 0) {
			synthv1::process(ins, outs, nread);
			for (uint16_t k = 0; k < nchannels; ++k) {
				ins[k]  += nread;
				outs[k] += nread;
			}
		}
		ndelta = event.time;
		synthv1::process_midi(event.buffer, event.size);
	}
#endif
#ifdef CONFIG_ALSA_MIDI
	jack_nframes_t buffer_size = ::jack_get_buffer_size(m_client);
	jack_nframes_t frame_time  = ::jack_last_frame_time(m_client);
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
			uint32_t nread = event_time - ndelta;
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

	synthv1::process(ins, outs, nframes - ndelta);

	return 0;
}


void synthv1_jack::open ( const char *client_id )
{
	// init param ports
	for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i)
		synthv1::setParamPort(synthv1::ParamIndex(i), &m_params[i]);

	// open client
	m_client = ::jack_client_open(client_id, JackNullOption, NULL);
	if (m_client == NULL)
		return;

	// set sample rate
	synthv1::setSampleRate(jack_get_sample_rate(m_client));
//	synthv1::reset();

	// register audio ports & buffers
	uint16_t nchannels = channels();

	m_audio_ins  = new jack_port_t * [nchannels];
	m_audio_outs = new jack_port_t * [nchannels];

	m_ins  = new float * [nchannels];
	m_outs = new float * [nchannels];

	char port_name[32];
	for (uint16_t k = 0; k < nchannels; ++k) {
		::snprintf(port_name, sizeof(port_name), "in_%d", k);
		m_audio_ins[k] = ::jack_port_register(m_client,
			port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
		m_ins[k] = NULL;
		::snprintf(port_name, sizeof(port_name), "out_%d", k);
		m_audio_outs[k] = ::jack_port_register(m_client,
			port_name, JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
		m_outs[k] = NULL;
	}

	// register midi port
#ifdef CONFIG_JACK_MIDI
	m_midi_in = ::jack_port_register(m_client,
		"in", JACK_DEFAULT_MIDI_TYPE, JackPortIsInput, 0);
#endif
#ifdef CONFIG_ALSA_MIDI
	m_alsa_seq     = NULL;
//	m_alsa_client  = -1;
	m_alsa_port    = -1;
	m_alsa_decoder = NULL;
	m_alsa_buffer  = NULL;
	m_alsa_thread  = NULL;
	// open alsa sequencer client...
	if (snd_seq_open(&m_alsa_seq, "hw", SND_SEQ_OPEN_INPUT, 0) >= 0) {
		snd_seq_set_client_name(m_alsa_seq, client_id);
	//	m_alsa_client = snd_seq_client_id(m_alsa_seq);
		m_alsa_port = snd_seq_create_simple_port(m_alsa_seq, "in",
			SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
			SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
		snd_midi_event_new(1024, &m_alsa_decoder);
		m_alsa_buffer = ::jack_ringbuffer_create(
			1024 * (sizeof(jack_midi_event_t) + 4));
		m_alsa_thread = new synthv1_alsa_thread(this);
		m_alsa_thread->start();
	}
#endif	// CONFIG_ALSA_MIDI

	// set process callbacks...
	::jack_set_process_callback(m_client,
		synthv1_jack_process, this);
}


void synthv1_jack::activate (void)
{
	synthv1::reset();

	if (m_client) ::jack_activate(m_client);
}

void synthv1_jack::deactivate (void)
{
	if (m_client) ::jack_deactivate(m_client);
}


void synthv1_jack::close (void)
{
	if (m_client == NULL)
		return;

#ifdef CONFIG_ALSA_MIDI
	// close alsa sequencer client...
	if (m_alsa_seq) {
		if (m_alsa_thread) {
			delete m_alsa_thread;
			m_alsa_thread = NULL;
		}
		if (m_alsa_buffer) {
			::jack_ringbuffer_free(m_alsa_buffer);
			m_alsa_buffer = NULL;
		}
		if (m_alsa_decoder) {
			snd_midi_event_free(m_alsa_decoder);
			m_alsa_decoder = NULL;
		}
		if (m_alsa_port >= 0) {
			snd_seq_delete_simple_port(m_alsa_seq, m_alsa_port);
			m_alsa_port = -1;
		}
		snd_seq_close(m_alsa_seq);
	//	m_alsa_client = -1;
		m_alsa_seq = NULL;
	}
#endif

#ifdef CONFIG_JACK_MIDI
	// unregister midi ports
	if (m_midi_in) {
		::jack_port_unregister(m_client, m_midi_in);
		m_midi_in = NULL;
	}
#endif

	// unregister audio ports
	uint16_t nchannels = channels();

	for (uint16_t k = 0; k < nchannels; ++k) {
		if (m_audio_outs && m_audio_outs[k]) {
			::jack_port_unregister(m_client, m_audio_outs[k]);
			m_audio_outs[k] = NULL;
		}
		if (m_outs && m_outs[k])
			m_outs[k] = NULL;
		if (m_audio_ins && m_audio_ins[k]) {
			::jack_port_unregister(m_client, m_audio_ins[k]);
			m_audio_ins[k] = NULL;
		}
		if (m_ins && m_ins[k])
			m_ins[k] = NULL;
	}

	if (m_outs) {
		delete [] m_outs;
		m_outs = NULL;
	}
	if (m_ins) {
		delete [] m_ins;
		m_ins = NULL;
	}

	if (m_audio_outs) {
		delete [] m_audio_outs;
		m_audio_outs = NULL;
	}
	if (m_audio_ins) {
		delete [] m_audio_ins;
		m_audio_ins = NULL;
	}

	// close client
	::jack_client_close(m_client);
	m_client = NULL;
}


void synthv1_jack::setParamValue ( synthv1::ParamIndex index, float fValue )
{
	m_params[index] = fValue;
}

float synthv1_jack::paramValue ( synthv1::ParamIndex index ) const
{
	return m_params[index];
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
	if (m_alsa_decoder == NULL)
		return;

	if (ev == NULL)
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


// end of synthv1_jack.cpp
