// synthv1_lv2.cpp
//
/****************************************************************************
   Copyright (C) 2012-2020, rncbc aka Rui Nuno Capela. All rights reserved.

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
#include "synthv1_config.h"
#include "synthv1_sched.h"

#include "synthv1_programs.h"
#include "synthv1_controls.h"

#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/time/time.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"

#include "lv2/lv2plug.in/ns/ext/state/state.h"

#include "lv2/lv2plug.in/ns/ext/options/options.h"
#include "lv2/lv2plug.in/ns/ext/buf-size/buf-size.h"

#ifdef CONFIG_LV2_PATCH
#include "lv2/lv2plug.in/ns/ext/patch/patch.h"
#endif

#ifndef CONFIG_LV2_ATOM_FORGE_OBJECT
#define lv2_atom_forge_object(forge, frame, id, otype) \
		lv2_atom_forge_blank(forge, frame, id, otype)
#endif

#ifndef CONFIG_LV2_ATOM_FORGE_KEY
#define lv2_atom_forge_key(forge, key) \
		lv2_atom_forge_property_head(forge, key, 0)
#endif

#ifndef LV2_STATE__StateChanged
#define LV2_STATE__StateChanged LV2_STATE_PREFIX "StateChanged"
#endif

#ifndef LV2_ATOM__portEvent
#define LV2_ATOM__portEvent LV2_ATOM_PREFIX "portEvent"
#endif

#include <stdlib.h>
#include <math.h>

#include <QApplication>
#include <QDomDocument>


//-------------------------------------------------------------------------
// synthv1_lv2 - impl.
//

// atom-like message used internally with worker/schedule
typedef struct {
	LV2_Atom atom;
	union {
		uint32_t    key;
		const char *path; // not used
	} data;
} synthv1_lv2_worker_message;


synthv1_lv2::synthv1_lv2 (
	double sample_rate, const LV2_Feature *const *host_features )
	: synthv1(2, float(sample_rate))
{
	::memset(&m_urids, 0, sizeof(m_urids));

	m_urid_map = nullptr;
	m_atom_in  = nullptr;
	m_atom_out = nullptr;
	m_schedule = nullptr;
	m_ndelta   = 0;

	const LV2_Options_Option *host_options = nullptr;

	for (int i = 0; host_features && host_features[i]; ++i) {
		const LV2_Feature *host_feature = host_features[i];
		if (::strcmp(host_feature->URI, LV2_URID_MAP_URI) == 0) {
			m_urid_map = (LV2_URID_Map *) host_feature->data;
			if (m_urid_map) {
				m_urids.p201_tuning_enabled = m_urid_map->map(
					m_urid_map->handle, SYNTHV1_LV2_PREFIX "P201_TUNING_ENABLED");
				m_urids.p202_tuning_refPitch = m_urid_map->map(
					m_urid_map->handle, SYNTHV1_LV2_PREFIX "P202_TUNING_REF_PITCH");
				m_urids.p203_tuning_refNote = m_urid_map->map(
					m_urid_map->handle, SYNTHV1_LV2_PREFIX "P203_TUNING_REF_NOTE");
				m_urids.p204_tuning_scaleFile = m_urid_map->map(
					m_urid_map->handle, SYNTHV1_LV2_PREFIX "P204_TUNING_SCALE_FILE");
				m_urids.p205_tuning_keyMapFile = m_urid_map->map(
					m_urid_map->handle, SYNTHV1_LV2_PREFIX "P205_TUNING_KEYMAP_FILE");
				m_urids.tun1_update = m_urid_map->map(
					m_urid_map->handle, SYNTHV1_LV2_PREFIX "TUN1_UPDATE");
				m_urids.atom_Blank = m_urid_map->map(
					m_urid_map->handle, LV2_ATOM__Blank);
				m_urids.atom_Object = m_urid_map->map(
					m_urid_map->handle, LV2_ATOM__Object);
				m_urids.atom_Float = m_urid_map->map(
					m_urid_map->handle, LV2_ATOM__Float);
				m_urids.atom_Int = m_urid_map->map(
					m_urid_map->handle, LV2_ATOM__Int);
				m_urids.atom_Bool = m_urid_map->map(
					m_urid_map->handle, LV2_ATOM__Bool);
				m_urids.atom_Path = m_urid_map->map(
					m_urid_map->handle, LV2_ATOM__Path);
				m_urids.atom_portEvent = m_urid_map->map(
					m_urid_map->handle, LV2_ATOM__portEvent);
				m_urids.time_Position = m_urid_map->map(
					m_urid_map->handle, LV2_TIME__Position);
				m_urids.time_beatsPerMinute = m_urid_map->map(
					m_urid_map->handle, LV2_TIME__beatsPerMinute);
				m_urids.midi_MidiEvent = m_urid_map->map(
					m_urid_map->handle, LV2_MIDI__MidiEvent);
				m_urids.bufsz_minBlockLength = m_urid_map->map(
					m_urid_map->handle, LV2_BUF_SIZE__minBlockLength);
				m_urids.bufsz_maxBlockLength = m_urid_map->map(
					m_urid_map->handle, LV2_BUF_SIZE__maxBlockLength);
			#ifdef LV2_BUF_SIZE__nominalBlockLength
				m_urids.bufsz_nominalBlockLength = m_urid_map->map(
					m_urid_map->handle, LV2_BUF_SIZE__nominalBlockLength);
			#endif
				m_urids.state_StateChanged = m_urid_map->map(
					m_urid_map->handle, LV2_STATE__StateChanged);
			#ifdef CONFIG_LV2_PATCH
				m_urids.patch_Get = m_urid_map->map(
					m_urid_map->handle, LV2_PATCH__Get);
				m_urids.patch_Set = m_urid_map->map(
					m_urid_map->handle, LV2_PATCH__Set);
				m_urids.patch_Put = m_urid_map->map(
					m_urid_map->handle, LV2_PATCH__Put);
				m_urids.patch_body = m_urid_map->map(
					m_urid_map->handle, LV2_PATCH__body);
				m_urids.patch_property = m_urid_map->map(
					m_urid_map->handle, LV2_PATCH__property);
				m_urids.patch_value = m_urid_map->map(
 					m_urid_map->handle, LV2_PATCH__value);
			#endif
			}
		}
		else
		if (::strcmp(host_feature->URI, LV2_WORKER__schedule) == 0)
			m_schedule = (LV2_Worker_Schedule *) host_feature->data;
		else
		if (::strcmp(host_feature->URI, LV2_OPTIONS__options) == 0)
			host_options = (const LV2_Options_Option *) host_feature->data;
	}

	uint32_t buffer_size = 0; // whatever happened to safe default?

	for (int i = 0; host_options && host_options[i].key; ++i) {
		const LV2_Options_Option *host_option = &host_options[i];
		if (host_option->type == m_urids.atom_Int) {
			uint32_t block_length = 0;
			if (host_option->key == m_urids.bufsz_minBlockLength)
				block_length = *(int *) host_option->value;
			else
			if (host_option->key == m_urids.bufsz_maxBlockLength)
				block_length = *(int *) host_option->value;
		#ifdef LV2_BUF_SIZE__nominalBlockLength
			else
			if (host_option->key == m_urids.bufsz_nominalBlockLength)
				block_length = *(int *) host_option->value;
		#endif
			// choose the lengthier...
			if (buffer_size < block_length)
				buffer_size = block_length;
		}
	}

	synthv1::setBufferSize(buffer_size);

	lv2_atom_forge_init(&m_forge, m_urid_map);

	const uint16_t nchannels = synthv1::channels();
	m_ins  = new float * [nchannels];
	m_outs = new float * [nchannels];
	for (uint16_t k = 0; k < nchannels; ++k)
		m_ins[k] = m_outs[k] = nullptr;
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
		m_atom_in = (LV2_Atom_Sequence *) data;
		break;
	case Notify:
		m_atom_out = (LV2_Atom_Sequence *) data;
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
		synthv1::setParamPort(synthv1::ParamIndex(port - ParamBase), (float *) data);
		break;
	}
}


void synthv1_lv2::run ( uint32_t nframes )
{
	const uint16_t nchannels = synthv1::channels();
	float *ins[nchannels], *outs[nchannels];
	for (uint16_t k = 0; k < nchannels; ++k) {
		ins[k]  = m_ins[k];
		outs[k] = m_outs[k];
	}

	if (m_atom_out) {
		const uint32_t capacity = m_atom_out->atom.size;
		lv2_atom_forge_set_buffer(&m_forge, (uint8_t *) m_atom_out, capacity);
		lv2_atom_forge_sequence_head(&m_forge, &m_notify_frame, 0);
	}

	uint32_t ndelta = 0;

	if (m_atom_in) {
		LV2_ATOM_SEQUENCE_FOREACH(m_atom_in, event) {
			if (event == nullptr)
				continue;
			if (event->body.type == m_urids.midi_MidiEvent) {
				uint8_t *data = (uint8_t *) LV2_ATOM_BODY(&event->body);
				if (event->time.frames > ndelta) {
					const uint32_t nread = event->time.frames - ndelta;
					if (nread > 0) {
						synthv1::process(ins, outs, nread);
						for (uint16_t k = 0; k < nchannels; ++k) {
							ins[k]  += nread;
							outs[k] += nread;
						}
					}
				}
				ndelta = event->time.frames;
				synthv1::process_midi(data, event->body.size);
			}
			else
			if (event->body.type == m_urids.atom_Blank ||
				event->body.type == m_urids.atom_Object) {
				const LV2_Atom_Object *object
					= (LV2_Atom_Object *) &event->body;
				if (object->body.otype == m_urids.time_Position) {
					LV2_Atom *atom = nullptr;
					lv2_atom_object_get(object,
						m_urids.time_beatsPerMinute, &atom, nullptr);
					if (atom && atom->type == m_urids.atom_Float) {
						const float host_bpm = ((LV2_Atom_Float *) atom)->body;
						if (::fabsf(host_bpm - synthv1::tempo()) > 0.001f)
							synthv1::setTempo(host_bpm);
					}
				}
			#ifdef CONFIG_LV2_PATCH
				else 
				if (object->body.otype == m_urids.patch_Set) {
					// set property value
					const LV2_Atom *property = nullptr;
					const LV2_Atom *value = nullptr;
					lv2_atom_object_get(object,
						m_urids.patch_property, &property,
						m_urids.patch_value, &value, 0);
					if (property && value && property->type == m_forge.URID) {
						const uint32_t key = ((const LV2_Atom_URID *) property)->body;
						const LV2_URID type = value->type;
						if (key == m_urids.p201_tuning_enabled
							&& type == m_urids.atom_Bool) {
							const uint32_t enabled
								= *(uint32_t *) LV2_ATOM_BODY_CONST(value);
							synthv1::setTuningEnabled(enabled > 0);
							updateTuning();
						}
						else
						if (key == m_urids.p202_tuning_refPitch
							&& type == m_urids.atom_Float) {
							const float refPitch
								= *(float *) LV2_ATOM_BODY_CONST(value);
							synthv1::setTuningRefPitch(refPitch);
							updateTuning();
						}
						else
						if (key == m_urids.p203_tuning_refNote
							&& type == m_urids.atom_Int) {
							const uint32_t refNote
								= *(uint32_t *) LV2_ATOM_BODY_CONST(value);
							synthv1::setTuningRefNote(refNote);
							updateTuning();
						}
						else
						if (key == m_urids.p204_tuning_scaleFile
							&& type == m_urids.atom_Path) {
							const char *scaleFile
								= (const char *) LV2_ATOM_BODY_CONST(value);
							synthv1::setTuningScaleFile(scaleFile);
							updateTuning();
						}
						else
						if (key == m_urids.p205_tuning_keyMapFile
							&& type == m_urids.atom_Path) {
							const char *keyMapFile
								= (const char *) LV2_ATOM_BODY_CONST(value);
							synthv1::setTuningKeyMapFile(keyMapFile);
							updateTuning();
						}
					}
				}
				else
				if (object->body.otype == m_urids.patch_Get) {
					// put all property values (probably to UI)
					patch_put();
				}
			#endif	// CONFIG_LV2_PATCH
			}
		}
		// remember last time for worker response
		m_ndelta = ndelta;
	//	m_atom_in = nullptr;
	}

	if (nframes > ndelta)
		synthv1::process(ins, outs, nframes - ndelta);
}


void synthv1_lv2::activate (void)
{
	synthv1::reset();
}


void synthv1_lv2::deactivate (void)
{
	synthv1::reset();
}


uint32_t synthv1_lv2::urid_map ( const char *uri ) const
{
	return (m_urid_map ? m_urid_map->map(m_urid_map->handle, uri) : 0);
}


//-------------------------------------------------------------------------
// synthv1_lv2 - Instantiation and cleanup.
//

QApplication *synthv1_lv2::g_qapp_instance = nullptr;
unsigned int  synthv1_lv2::g_qapp_refcount = 0;


void synthv1_lv2::qapp_instantiate (void)
{
	if (qApp == nullptr && g_qapp_instance == nullptr) {
		static int s_argc = 1;
		static const char *s_argv[] = { __func__, nullptr };
		g_qapp_instance = new QApplication(s_argc, (char **) s_argv);
	}

	if (g_qapp_instance)
		g_qapp_refcount++;
}


void synthv1_lv2::qapp_cleanup (void)
{
	if (g_qapp_instance && --g_qapp_refcount == 0) {
		delete g_qapp_instance;
		g_qapp_instance = nullptr;
	}
}


QApplication *synthv1_lv2::qapp_instance (void)
{
	return g_qapp_instance;
}


//-------------------------------------------------------------------------
// synthv1_lv2 - LV2 State interface.
//

static LV2_State_Status synthv1_lv2_state_save ( LV2_Handle instance,
	LV2_State_Store_Function store, LV2_State_Handle handle,
	uint32_t flags, const LV2_Feature *const */*features*/ )
{
	synthv1_lv2 *pPlugin = static_cast<synthv1_lv2 *> (instance);
	if (pPlugin == nullptr)
		return LV2_STATE_ERR_UNKNOWN;

	// FIXME: At this time, only micro-tonal (aka. tuning) settings
	// are posed to be saved into some XML chunk as state...
	if (!pPlugin->isTuningEnabled())
		return LV2_STATE_SUCCESS;

	// Save state as XML chunk...
	//
	const uint32_t key = pPlugin->urid_map(SYNTHV1_LV2_PREFIX "state");
	if (key == 0)
		return LV2_STATE_ERR_NO_PROPERTY;

	const uint32_t type = pPlugin->urid_map(LV2_ATOM__Chunk);
	if (type == 0)
		return LV2_STATE_ERR_BAD_TYPE;
#if 0
	if ((flags & (LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE)) == 0)
		return LV2_STATE_ERR_BAD_FLAGS;
#else
	flags |= (LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE);
#endif

	QDomDocument doc(SYNTHV1_TITLE);
	QDomElement eState = doc.createElement("state");

	QDomElement eTuning = doc.createElement("tuning");
	synthv1_param::saveTuning(pPlugin, doc, eTuning);
	eState.appendChild(eTuning);

	doc.appendChild(eState);

	const QByteArray data(doc.toByteArray());
	const char *value = data.constData();
	size_t size = data.size();

	return (*store)(handle, key, value, size, type, flags);
}


static LV2_State_Status synthv1_lv2_state_restore ( LV2_Handle instance,
	LV2_State_Retrieve_Function retrieve, LV2_State_Handle handle,
	uint32_t flags, const LV2_Feature *const */*features*/ )
{
	synthv1_lv2 *pPlugin = static_cast<synthv1_lv2 *> (instance);
	if (pPlugin == nullptr)
		return LV2_STATE_ERR_UNKNOWN;

	// Retrieve state as XML chunk...
	//
	const uint32_t key = pPlugin->urid_map(SYNTHV1_LV2_PREFIX "state");
	if (key == 0)
		return LV2_STATE_ERR_NO_PROPERTY;

	const uint32_t chunk_type = pPlugin->urid_map(LV2_ATOM__Chunk);
	if (chunk_type == 0)
		return LV2_STATE_ERR_BAD_TYPE;

	size_t size = 0;
	uint32_t type = 0;
//	flags = 0;

	const char *value
		= (const char *) (*retrieve)(handle, key, &size, &type, &flags);

	if (size < 2)
		return LV2_STATE_ERR_UNKNOWN;

	if (type != chunk_type)
		return LV2_STATE_ERR_BAD_TYPE;

	if ((flags & (LV2_STATE_IS_POD | LV2_STATE_IS_PORTABLE)) == 0)
		return LV2_STATE_ERR_BAD_FLAGS;

	if (value == nullptr)
		return LV2_STATE_ERR_UNKNOWN;

	QDomDocument doc(SYNTHV1_TITLE);
	if (doc.setContent(QByteArray(value, size))) {
		QDomElement eState = doc.documentElement();
		if (eState.tagName() == "state") {
			for (QDomNode nChild = eState.firstChild();
					!nChild.isNull();
						nChild = nChild.nextSibling()) {
				QDomElement eChild = nChild.toElement();
				if (eChild.isNull())
					continue;
				if (eChild.tagName() == "tuning")
					synthv1_param::loadTuning(pPlugin, eChild);
			}
		}
	}

	pPlugin->reset();

	synthv1_sched::sync_notify(pPlugin, synthv1_sched::Wave, 1);

	return LV2_STATE_SUCCESS;
}


static const LV2_State_Interface synthv1_lv2_state_interface =
{
	synthv1_lv2_state_save,
	synthv1_lv2_state_restore
};


#ifdef CONFIG_LV2_PROGRAMS

#include "synthv1_programs.h"

const LV2_Program_Descriptor *synthv1_lv2::get_program ( uint32_t index )
{
	synthv1_programs *pPrograms = synthv1::programs();
	const synthv1_programs::Banks& banks = pPrograms->banks();
	synthv1_programs::Banks::ConstIterator bank_iter = banks.constBegin();
	const synthv1_programs::Banks::ConstIterator& bank_end = banks.constEnd();
	for (uint32_t i = 0; bank_iter != bank_end; ++bank_iter) {
		synthv1_programs::Bank *pBank = bank_iter.value();
		const synthv1_programs::Progs& progs = pBank->progs();
		synthv1_programs::Progs::ConstIterator prog_iter = progs.constBegin();
		const synthv1_programs::Progs::ConstIterator& prog_end = progs.constEnd();
		for ( ; prog_iter != prog_end; ++prog_iter, ++i) {
			synthv1_programs::Prog *pProg = prog_iter.value();
			if (i >= index) {
				m_aProgramName = pProg->name().toUtf8();
				m_program.bank = pBank->id();
				m_program.program = pProg->id();
				m_program.name = m_aProgramName.constData();
				return &m_program;
			}
		}
	}

	return nullptr;
}

void synthv1_lv2::select_program ( uint32_t bank, uint32_t program )
{
	synthv1::programs()->select_program(bank, program);
}

#endif	// CONFIG_LV2_PROGRAMS


void synthv1_lv2::updatePreset ( bool /*bDirty*/ )
{
	if (m_schedule /*&& bDirty*/) {
		synthv1_lv2_worker_message mesg;
		mesg.atom.type = m_urids.state_StateChanged;
		mesg.atom.size = 0; // nothing else matters.
		m_schedule->schedule_work(
			m_schedule->handle, sizeof(mesg), &mesg);
	}
}


void synthv1_lv2::updateParam ( synthv1::ParamIndex index )
{
	if (m_schedule) {
		synthv1_lv2_worker_message mesg;
		mesg.atom.type = m_urids.atom_portEvent;
		mesg.atom.size = sizeof(mesg.data.key);
		mesg.data.key  = uint32_t(index);
		m_schedule->schedule_work(
			m_schedule->handle, sizeof(mesg), &mesg);
	}
}


void synthv1_lv2::updateParams (void)
{
	if (m_schedule) {
		synthv1_lv2_worker_message mesg;
		mesg.atom.type = m_urids.atom_portEvent;
		mesg.atom.size = 0; // nothing else matters.
		m_schedule->schedule_work(
			m_schedule->handle, sizeof(mesg), &mesg);
	}
}


void synthv1_lv2::updateTuning (void)
{
	if (m_schedule) {
		synthv1_lv2_worker_message mesg;
		mesg.atom.type = m_urids.tun1_update;
		mesg.atom.size = 0; // nothing else matters.
		m_schedule->schedule_work(
			m_schedule->handle, sizeof(mesg), &mesg);
	}
}


bool synthv1_lv2::worker_work ( const void *data, uint32_t size )
{
	if (size != sizeof(synthv1_lv2_worker_message))
		return false;

	const synthv1_lv2_worker_message *mesg
		= (const synthv1_lv2_worker_message *) data;

	if (mesg->atom.type == m_urids.atom_portEvent)
		return true;
	else
	if (mesg->atom.type == m_urids.state_StateChanged)
		return true;
	else
	if (mesg->atom.type == m_urids.tun1_update) {
		synthv1::resetTuning();
		return true;
	}

	return false;
}


bool synthv1_lv2::worker_response ( const void *data, uint32_t size )
{
	if (size != sizeof(synthv1_lv2_worker_message))
		return false;

	const synthv1_lv2_worker_message *mesg
		= (const synthv1_lv2_worker_message *) data;

	if (mesg->atom.type == m_urids.atom_portEvent) {
		if (mesg->atom.size > 0)
			return port_event(synthv1::ParamIndex(mesg->data.key));
		else
			return port_events();
	}
	else
	if (mesg->atom.type == m_urids.state_StateChanged)
		return state_changed();

#ifdef CONFIG_LV2_PATCH
	return patch_put();
#else
	return true;
#endif
}


bool synthv1_lv2::state_changed (void)
{
	lv2_atom_forge_frame_time(&m_forge, m_ndelta);

	LV2_Atom_Forge_Frame frame;
	lv2_atom_forge_object(&m_forge, &frame, 0, m_urids.state_StateChanged);
	lv2_atom_forge_pop(&m_forge, &frame);

	return true;
}


#ifdef CONFIG_LV2_PATCH

bool synthv1_lv2::patch_put ( uint32_t type )
{
	static char s_szNull[1] = {'\0'};

	if (type == m_urids.patch_Put)
		type = 0;

	lv2_atom_forge_frame_time(&m_forge, m_ndelta);

	LV2_Atom_Forge_Frame patch_frame;
	lv2_atom_forge_object(&m_forge, &patch_frame, 0, m_urids.patch_Put);
	lv2_atom_forge_key(&m_forge, m_urids.patch_body);

	LV2_Atom_Forge_Frame body_frame;
	lv2_atom_forge_object(&m_forge, &body_frame, 0, 0);

	if (type == 0 || type == m_urids.p201_tuning_enabled) {
		lv2_atom_forge_key(&m_forge, m_urids.p201_tuning_enabled);
		lv2_atom_forge_bool(&m_forge, synthv1::isTuningEnabled());
	}
	if (type == 0 || type == m_urids.p202_tuning_refPitch) {
		lv2_atom_forge_key(&m_forge, m_urids.p202_tuning_refPitch);
		lv2_atom_forge_float(&m_forge, synthv1::tuningRefPitch());
	}
	if (type == 0 || type == m_urids.p203_tuning_refNote) {
		lv2_atom_forge_key(&m_forge, m_urids.p203_tuning_refNote);
		lv2_atom_forge_int(&m_forge, synthv1::tuningRefNote());
	}
	if (type == 0 || type == m_urids.p204_tuning_scaleFile) {
		const char *pszScaleFile = synthv1::tuningScaleFile();
		if (pszScaleFile == nullptr)
			pszScaleFile = s_szNull;
		lv2_atom_forge_key(&m_forge, m_urids.p204_tuning_scaleFile);
		lv2_atom_forge_path(&m_forge, pszScaleFile, ::strlen(pszScaleFile) + 1);
	}
	if (type == 0 || type == m_urids.p205_tuning_keyMapFile) {
		const char *pszKeyMapFile = synthv1::tuningKeyMapFile();
		if (pszKeyMapFile == nullptr)
			pszKeyMapFile = s_szNull;
		lv2_atom_forge_key(&m_forge, m_urids.p205_tuning_keyMapFile);
		lv2_atom_forge_path(&m_forge, pszKeyMapFile, ::strlen(pszKeyMapFile) + 1);
	}

	lv2_atom_forge_pop(&m_forge, &body_frame);
	lv2_atom_forge_pop(&m_forge, &patch_frame);

	return true;
}

#endif	// CONFIG_LV2_PATCH


bool synthv1_lv2::port_event ( synthv1::ParamIndex index )
{
	lv2_atom_forge_frame_time(&m_forge, m_ndelta);

	LV2_Atom_Forge_Frame obj_frame;
	lv2_atom_forge_object(&m_forge, &obj_frame, 0, m_urids.atom_portEvent);
	lv2_atom_forge_key(&m_forge, m_forge.Tuple);

	LV2_Atom_Forge_Frame tup_frame;
	lv2_atom_forge_tuple(&m_forge, &tup_frame);

	lv2_atom_forge_int(&m_forge, int32_t(ParamBase + index));
	lv2_atom_forge_float(&m_forge, synthv1::paramValue(index));

	lv2_atom_forge_pop(&m_forge, &tup_frame);
	lv2_atom_forge_pop(&m_forge, &obj_frame);

	return true;
}


bool synthv1_lv2::port_events (void)
{
	lv2_atom_forge_frame_time(&m_forge, m_ndelta);

	LV2_Atom_Forge_Frame obj_frame;
	lv2_atom_forge_object(&m_forge, &obj_frame, 0, m_urids.atom_portEvent);
	lv2_atom_forge_key(&m_forge, m_forge.Tuple);

	LV2_Atom_Forge_Frame tup_frame;
	lv2_atom_forge_tuple(&m_forge, &tup_frame);

	for (uint32_t i = 0; i < synthv1::NUM_PARAMS; ++i) {
		synthv1::ParamIndex index = synthv1::ParamIndex(i);
		lv2_atom_forge_int(&m_forge, int32_t(ParamBase + index));
		lv2_atom_forge_float(&m_forge, synthv1::paramValue(index));
	}

	lv2_atom_forge_pop(&m_forge, &tup_frame);
	lv2_atom_forge_pop(&m_forge, &obj_frame);

	return true;
}


//-------------------------------------------------------------------------
// synthv1_lv2 - LV2 desc.
//

static LV2_Handle synthv1_lv2_instantiate (
	const LV2_Descriptor *, double sample_rate, const char *,
	const LV2_Feature *const *host_features )
{
	synthv1_lv2::qapp_instantiate();

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

	synthv1_lv2::qapp_cleanup();
}


#ifdef CONFIG_LV2_PROGRAMS

static const LV2_Program_Descriptor *synthv1_lv2_programs_get_program (
	LV2_Handle instance, uint32_t index )
{
	synthv1_lv2 *pPlugin = static_cast<synthv1_lv2 *> (instance);
	if (pPlugin)
		return pPlugin->get_program(index);
	else
		return nullptr;
}

static void synthv1_lv2_programs_select_program (
	LV2_Handle instance, uint32_t bank, uint32_t program )
{
	synthv1_lv2 *pPlugin = static_cast<synthv1_lv2 *> (instance);
	if (pPlugin)
		pPlugin->select_program(bank, program);
}

static const LV2_Programs_Interface synthv1_lv2_programs_interface =
{
	synthv1_lv2_programs_get_program,
	synthv1_lv2_programs_select_program,
};

#endif	// CONFIG_LV2_PROGRAMS


static LV2_Worker_Status synthv1_lv2_worker_work (
	LV2_Handle instance, LV2_Worker_Respond_Function respond,
	LV2_Worker_Respond_Handle handle, uint32_t size, const void *data )
{
	synthv1_lv2 *pSynth = static_cast<synthv1_lv2 *> (instance);
	if (pSynth && pSynth->worker_work(data, size)) {
		respond(handle, size, data);
		return LV2_WORKER_SUCCESS;
	}

	return LV2_WORKER_ERR_UNKNOWN;
}


static LV2_Worker_Status synthv1_lv2_worker_response (
	LV2_Handle instance, uint32_t size, const void *data )
{
	synthv1_lv2 *pSynth = static_cast<synthv1_lv2 *> (instance);
	if (pSynth && pSynth->worker_response(data, size))
		return LV2_WORKER_SUCCESS;
	else
		return LV2_WORKER_ERR_UNKNOWN;
}


static const LV2_Worker_Interface synthv1_lv2_worker_interface =
{
	synthv1_lv2_worker_work,
	synthv1_lv2_worker_response,
	nullptr
};


static const void *synthv1_lv2_extension_data ( const char *uri )
{
#ifdef CONFIG_LV2_PROGRAMS
	if (::strcmp(uri, LV2_PROGRAMS__Interface) == 0)
		return (void *) &synthv1_lv2_programs_interface;
	else
#endif
	if (::strcmp(uri, LV2_WORKER__interface) == 0)
		return &synthv1_lv2_worker_interface;
	else
	if (::strcmp(uri, LV2_STATE__interface) == 0)
		return &synthv1_lv2_state_interface;

	return nullptr;
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
	return (index == 0 ? &synthv1_lv2_descriptor : nullptr);
}


// end of synthv1_lv2.cpp

