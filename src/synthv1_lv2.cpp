// synthv1_lv2.cpp
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

#include "synthv1_lv2.h"

#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"

#include <math.h>


//-------------------------------------------------------------------------
// synthv1_lv2 - impl.
//

synthv1_lv2::synthv1_lv2 (
	double sample_rate, const LV2_Feature *const *host_features )
	: synthv1(2, uint32_t(sample_rate))
{
	::memset(&m_urids, 0, sizeof(m_urids));

	m_atom_sequence = NULL;

	for (int i = 0; host_features[i]; ++i) {
		if (::strcmp(host_features[i]->URI, LV2_URID_MAP_URI) == 0) {
			LV2_URID_Map *urid_map
				= (LV2_URID_Map *) host_features[i]->data;
			if (urid_map) {
				m_urids.atom_Blank = urid_map->map(
					urid_map->handle, LV2_ATOM__Blank);
				m_urids.atom_Object = urid_map->map(
					urid_map->handle, LV2_ATOM__Object);
				m_urids.atom_Float = urid_map->map(
					urid_map->handle, LV2_ATOM__Float);
				m_urids.time_Position = urid_map->map(
					urid_map->handle, LV2_TIME__Position);
				m_urids.time_beatsPerMinute = urid_map->map(
					urid_map->handle, LV2_TIME__beatsPerMinute);
				m_urids.midi_MidiEvent = urid_map->map(
					urid_map->handle, LV2_MIDI__MidiEvent);
				break;
			}
		}
	}

	const uint16_t nchannels = channels();
	m_ins  = new float * [nchannels];
	m_outs = new float * [nchannels];
	for (uint16_t k = 0; k < nchannels; ++k)
		m_ins[k] = m_outs[k] = NULL;
}


synthv1_lv2::~synthv1_lv2 (void)
{
	delete [] m_outs;
	delete [] m_ins;
}


void synthv1_lv2::connect_port ( uint32_t port, void *data )
{
	switch(PortIndex(port)) {
	case MidiIn:
		m_atom_sequence = (LV2_Atom_Sequence *) data;
		break;
	case AudioInL:
		m_ins[0] = (float *) data;
		break;
	case AudioInR:
		m_ins[1] = (float *) data;
		break;
	case AudioOutL:
		m_outs[0] = (float *) data;
		break;
	case AudioOutR:
		m_outs[1] = (float *) data;
		break;
	default:
		setParamPort(ParamIndex(port - ParamBase), (float *) data);
		break;
	}
}


void synthv1_lv2::run ( uint32_t nframes )
{
	const uint16_t nchannels = channels();
	float *ins[nchannels], *outs[nchannels];
	for (uint16_t k = 0; k < nchannels; ++k) {
		ins[k]  = m_ins[k];
		outs[k] = m_outs[k];
	}

	uint32_t ndelta = 0;

	if (m_atom_sequence) {
		LV2_ATOM_SEQUENCE_FOREACH(m_atom_sequence, event) {
			if (event == NULL)
				continue;
			if (event->body.type == m_urids.midi_MidiEvent) {
				uint8_t *data = (uint8_t *) LV2_ATOM_BODY(&event->body);
				const uint32_t nread = event->time.frames - ndelta;
				if (nread > 0) {
					process(ins, outs, nread);
					for (uint16_t k = 0; k < nchannels; ++k) {
						ins[k]  += nread;
						outs[k] += nread;
					}
				}
				ndelta = event->time.frames;
				process_midi(data, event->body.size);
			}
			else
			if (event->body.type == m_urids.atom_Blank ||
				event->body.type == m_urids.atom_Object) {
				const LV2_Atom_Object *object
					= (LV2_Atom_Object *) &event->body;
				if (object->body.otype == m_urids.time_Position) {
					LV2_Atom *atom = NULL;
					lv2_atom_object_get(object,
						m_urids.time_beatsPerMinute, &atom, NULL);
					if (atom && atom->type == m_urids.atom_Float) {
						const float *bpmsync = paramPort(synthv1::DEL1_BPMSYNC);
						if (bpmsync && *bpmsync > 0.0f) {
							float *bpmhost = paramPort(synthv1::DEL1_BPMHOST);
							if (bpmhost) {
								const float bpm	= ((LV2_Atom_Float *) atom)->body;
								if (::fabs(*bpmhost - bpm) > 0.01f)
									*bpmhost = bpm;
							}
						}
					}
				}
			}
		}
	//	m_atom_sequence = NULL;
	}

	process(ins, outs, nframes - ndelta);
}


void synthv1_lv2::activate (void)
{
	reset();
}


void synthv1_lv2::deactivate (void)
{
	reset();
}


//-------------------------------------------------------------------------
// synthv1_lv2 - LV2 desc.
//

static LV2_Handle synthv1_lv2_instantiate (
	const LV2_Descriptor *, double sample_rate, const char *,
	const LV2_Feature *const *host_features )
{
	return new synthv1_lv2(sample_rate, host_features);
}


static void synthv1_lv2_connect_port (
	LV2_Handle instance, uint32_t port, void *data )
{
	synthv1_lv2 *pPlugin = static_cast<synthv1_lv2 *> (instance);
	if (pPlugin)
		pPlugin->connect_port(port, data);
}


static void synthv1_lv2_run ( LV2_Handle instance, uint32_t nframes )
{
	synthv1_lv2 *pPlugin = static_cast<synthv1_lv2 *> (instance);
	if (pPlugin)
		pPlugin->run(nframes);
}


static void synthv1_lv2_activate ( LV2_Handle instance )
{
	synthv1_lv2 *pPlugin = static_cast<synthv1_lv2 *> (instance);
	if (pPlugin)
		pPlugin->activate();
}


static void synthv1_lv2_deactivate ( LV2_Handle instance )
{
	synthv1_lv2 *pPlugin = static_cast<synthv1_lv2 *> (instance);
	if (pPlugin)
		pPlugin->deactivate();
}


static void synthv1_lv2_cleanup ( LV2_Handle instance )
{
	synthv1_lv2 *pPlugin = static_cast<synthv1_lv2 *> (instance);
	if (pPlugin)
		delete pPlugin;
}


static const void *synthv1_lv2_extension_data ( const char * )
{
    return NULL;
}


static const LV2_Descriptor synthv1_lv2_descriptor =
{
	SYNTHV1_LV2_URI,
	synthv1_lv2_instantiate,
	synthv1_lv2_connect_port,
	synthv1_lv2_activate,
	synthv1_lv2_run,
	synthv1_lv2_deactivate,
	synthv1_lv2_cleanup,
	synthv1_lv2_extension_data
};


LV2_SYMBOL_EXPORT const LV2_Descriptor *lv2_descriptor ( uint32_t index )
{
	return (index == 0 ? &synthv1_lv2_descriptor : NULL);
}


// end of synthv1_lv2.cpp
