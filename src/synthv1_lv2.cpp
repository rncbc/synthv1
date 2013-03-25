// synthv1_lv2.cpp
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

#include "synthv1_lv2.h"

#include "synthv1widget_lv2.h"

#include "lv2/lv2plug.in/ns/ext/urid/urid.h"
#include "lv2/lv2plug.in/ns/ext/midi/midi.h"
#include "lv2/lv2plug.in/ns/ext/atom/util.h"


//-------------------------------------------------------------------------
// synthv1_lv2 - impl.
//

synthv1_lv2::synthv1_lv2 (
	double sample_rate, const LV2_Feature *const *host_features )
	: synthv1(2, uint32_t(sample_rate))
{
	m_midi_event_type = 0;
	m_atom_sequence = NULL;

	for (int i = 0; host_features[i]; ++i) {
		if (::strcmp(host_features[i]->URI, LV2_URID_MAP_URI) == 0) {
			LV2_URID_Map *urid_map
				= (LV2_URID_Map *) host_features[i]->data;
			if (urid_map) {
				m_midi_event_type = urid_map->map(
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
			if (event && event->body.type == m_midi_event_type) {
				uint8_t *data = (uint8_t *) LV2_ATOM_BODY(&event->body);
				uint32_t nread = event->time.frames - ndelta;
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


static LV2UI_Handle synthv1_lv2ui_instantiate (
	const LV2UI_Descriptor *, const char *, const char *,
	LV2UI_Write_Function write_function,
	LV2UI_Controller controller, LV2UI_Widget *widget,
	const LV2_Feature *const * )
{
	synthv1widget_lv2 *pWidget = new synthv1widget_lv2(controller, write_function);
	*widget = pWidget;
	return pWidget;
}

static void synthv1_lv2ui_cleanup ( LV2UI_Handle ui )
{
	synthv1widget_lv2 *pWidget = static_cast<synthv1widget_lv2 *> (ui);
	if (pWidget)
		delete pWidget;
}

static void synthv1_lv2ui_port_event (
	LV2UI_Handle ui, uint32_t port_index,
	uint32_t buffer_size, uint32_t format, const void *buffer )
{
	synthv1widget_lv2 *pWidget = static_cast<synthv1widget_lv2 *> (ui);
	if (pWidget)
		pWidget->port_event(port_index, buffer_size, format, buffer);
}

static const void *synthv1_lv2ui_extension_data ( const char * )
{
	return NULL;
}


#ifdef CONFIG_LV2_EXTERNAL_UI

struct synthv1_lv2ui_external_widget
{
	LV2_External_UI_Widget external;
	synthv1widget_lv2     *widget;
};

static void synthv1_lv2ui_external_run ( LV2_External_UI_Widget *ui_external )
{
	synthv1_lv2ui_external_widget *pExtWidget
		= (synthv1_lv2ui_external_widget *) (ui_external);
	if (pExtWidget && pExtWidget->widget)
		pExtWidget->widget->update();
}

static void synthv1_lv2ui_external_show ( LV2_External_UI_Widget *ui_external )
{
	synthv1_lv2ui_external_widget *pExtWidget
		= (synthv1_lv2ui_external_widget *) (ui_external);
	if (pExtWidget && pExtWidget->widget)
		pExtWidget->widget->show();
}

static void synthv1_lv2ui_external_hide ( LV2_External_UI_Widget *ui_external )
{
	synthv1_lv2ui_external_widget *pExtWidget
		= (synthv1_lv2ui_external_widget *) (ui_external);
	if (pExtWidget && pExtWidget->widget)
		pExtWidget->widget->hide();
}

static LV2UI_Handle synthv1_lv2ui_external_instantiate (
	const LV2UI_Descriptor *, const char *, const char *,
	LV2UI_Write_Function write_function,
	LV2UI_Controller controller, LV2UI_Widget *widget,
	const LV2_Feature *const *ui_features )
{
	LV2_External_UI_Host *external_host = NULL;
	for (int i = 0; ui_features[i] && !external_host; ++i) {
		if (::strcmp(ui_features[i]->URI, LV2_EXTERNAL_UI__Host) == 0 ||
			::strcmp(ui_features[i]->URI, LV2_EXTERNAL_UI_DEPRECATED_URI) == 0) {
			external_host = (LV2_External_UI_Host *) ui_features[i]->data;
		}
	}
	
	synthv1_lv2ui_external_widget *pExtWidget = new synthv1_lv2ui_external_widget;
	pExtWidget->external.run  = synthv1_lv2ui_external_run;
	pExtWidget->external.show = synthv1_lv2ui_external_show;
	pExtWidget->external.hide = synthv1_lv2ui_external_hide;
	pExtWidget->widget = new synthv1widget_lv2(controller, write_function);
	if (external_host)
		pExtWidget->widget->setExternalHost(external_host);
	*widget = pExtWidget;
	return pExtWidget;
}

static void synthv1_lv2ui_external_cleanup ( LV2UI_Handle ui )
{
	synthv1_lv2ui_external_widget *pExtWidget
		= static_cast<synthv1_lv2ui_external_widget *> (ui);
	if (pExtWidget) {
		if (pExtWidget->widget)
			delete pExtWidget->widget;
		delete pExtWidget;
	}
}

static void synthv1_lv2ui_external_port_event (
	LV2UI_Handle ui, uint32_t port_index,
	uint32_t buffer_size, uint32_t format, const void *buffer )
{
	synthv1_lv2ui_external_widget *pExtWidget
		= static_cast<synthv1_lv2ui_external_widget *> (ui);
	if (pExtWidget && pExtWidget->widget)
		pExtWidget->widget->port_event(port_index, buffer_size, format, buffer);
}

static const void *synthv1_lv2ui_external_extension_data ( const char * )
{
	return NULL;
}

#endif	// CONFIG_LV2_EXTERNAL_UI


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

static const LV2UI_Descriptor synthv1_lv2ui_descriptor =
{
	SYNTHV1_LV2UI_URI,
	synthv1_lv2ui_instantiate,
	synthv1_lv2ui_cleanup,
	synthv1_lv2ui_port_event,
	synthv1_lv2ui_extension_data
};

#ifdef CONFIG_LV2_EXTERNAL_UI
static const LV2UI_Descriptor synthv1_lv2ui_external_descriptor =
{
	SYNTHV1_LV2UI_EXTERNAL_URI,
	synthv1_lv2ui_external_instantiate,
	synthv1_lv2ui_external_cleanup,
	synthv1_lv2ui_external_port_event,
	synthv1_lv2ui_external_extension_data
};
#endif	// CONFIG_LV2_EXTERNAL_UI


LV2_SYMBOL_EXPORT const LV2_Descriptor *lv2_descriptor ( uint32_t index )
{
	return (index == 0 ? &synthv1_lv2_descriptor : NULL);
}


LV2_SYMBOL_EXPORT const LV2UI_Descriptor *lv2ui_descriptor ( uint32_t index )
{
	if (index == 0)
		return &synthv1_lv2ui_descriptor;
	else
#ifdef CONFIG_LV2_EXTERNAL_UI
	if (index == 1)
		return &synthv1_lv2ui_external_descriptor;
	else
#endif
	return NULL;
}


// end of synthv1_lv2.cpp
