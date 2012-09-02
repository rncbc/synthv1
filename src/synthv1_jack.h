// synthv1_jack.h
//
/****************************************************************************
   Copyright (C) 2012, rncbc aka Rui Nuno Capela. All rights reserved.

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

	synthv1_jack();

	~synthv1_jack();

	jack_client_t *client() const;

	int process(jack_nframes_t nframes);

	void activate();
	void deactivate();

	void setParamValue(synthv1::ParamIndex index, float fValue);
	float paramValue(synthv1::ParamIndex index) const;

#ifdef CONFIG_ALSA_MIDI
	snd_seq_t *alsa_seq() const;
	void alsa_capture(snd_seq_event_t *ev);
#endif

protected:

	void open();
	void close();

private:

	jack_client_t *m_client;

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


#endif// __synthv1_jack_h

// end of synthv1_jack.h
