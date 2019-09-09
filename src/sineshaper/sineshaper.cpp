/****************************************************************************
    
    sineshaper.cpp - A DSSI synth plugin
    
    Copyright (C) 2005  Lars Luthman <larsl@users.sourceforge.net>
    
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 01222-1307  USA

****************************************************************************/

#include <cmath>

#include "sineshaper.hpp"
#include "sineshaperports.hpp"


using namespace std;


SineShaper::SineShaper(unsigned long frame_rate) 
  : m_vibrato_lfo(frame_rate), m_tremolo_lfo(frame_rate), 
    m_shaper_lfo(frame_rate), m_osc(frame_rate), m_subosc(frame_rate), 
    m_adsr(frame_rate), m_freq_slide(frame_rate), m_vel_slide(frame_rate), 
    m_delay(frame_rate, 3), m_osc_mix_slide(frame_rate), 
    m_trem_depth_slide(frame_rate), m_shp_env_slide(frame_rate), 
    m_shp_total_slide(frame_rate), m_shp_split_slide(frame_rate), 
    m_shp_shift_slide(frame_rate), m_lfo_depth_slide(frame_rate), 
    m_att_slide(frame_rate), m_dec_slide(frame_rate), 
    m_sus_slide(frame_rate), m_rel_slide(frame_rate), 
    m_amp_env_slide(frame_rate), m_drive_slide(frame_rate),
    m_gain_slide(frame_rate), m_delay_fb_slide(frame_rate), 
    m_delay_mix_slide(frame_rate), m_blocker(frame_rate), 
    m_frame_rate(frame_rate), m_last_frame(0), m_velocity(0.5f),
    m_pitch(0.0f), m_note_is_on(false), m_pitchbend(1.0f) {
  
  PresetManager* tmp = new PresetManager;
  tmp->load_bank(INSTALL_DIR "/sineshaper/presets", INPUT_PORTS);
  tmp->load_bank(string(getenv("HOME")) + "/.sineshaperpresets", INPUT_PORTS);
  m_pm = tmp;
}


void SineShaper::run_synth(unsigned long sample_count, snd_seq_event_t* events,
			   unsigned long event_count) {
  
  LADSPA_Data freq;
  LADSPA_Data vel;
  
  // get all the port values and buffers
  
  LADSPA_Data tune = *(m_ports[TUN]) * pow(2.0f, *(m_ports[OCT]));
  LADSPA_Data subtune = *(m_ports[SUB_TUN]) * pow(2.0f, *(m_ports[SUB_OCT]));
  LADSPA_Data& osc_mix = *(m_ports[OSC_MIX]);

  LADSPA_Data& porta_on = *(m_ports[PRT_ON]);
  LADSPA_Data& porta_time = *(m_ports[PRT_TIM]);
  LADSPA_Data& vib_freq = *(m_ports[VIB_FRQ]);
  LADSPA_Data& vib_depth = *(m_ports[VIB_DPT]);
  LADSPA_Data& trem_freq = *(m_ports[TRM_FRQ]);
  LADSPA_Data& trem_depth = *(m_ports[TRM_DPT]);
  
  LADSPA_Data& shp_env = *(m_ports[SHP_ENV]);
  LADSPA_Data& shp_total = *(m_ports[SHP_TOT]);
  LADSPA_Data& shp_split = *(m_ports[SHP_SPL]);
  LADSPA_Data shp_shift = *(m_ports[SHP_SHF]) * M_PI / 2;
  LADSPA_Data& lfo_freq = *(m_ports[LFO_FRQ]);
  LADSPA_Data& lfo_depth = *(m_ports[LFO_DPT]);
  
  LADSPA_Data& att = *(m_ports[ATT]);
  LADSPA_Data& dec = *(m_ports[DEC]);
  LADSPA_Data& sus = *(m_ports[SUS]);
  LADSPA_Data& rel = *(m_ports[REL]);

  LADSPA_Data& amp_env = *(m_ports[AMP_ENV]);
  LADSPA_Data& drive = *(m_ports[DRIVE]);
  LADSPA_Data& gain = *(m_ports[GAIN]);

  LADSPA_Data& delay_time = *(m_ports[DEL_TIM]);
  LADSPA_Data& delay_feedback = *(m_ports[DEL_FB]);
  LADSPA_Data& delay_mix = *(m_ports[DEL_MIX]);
  
  LADSPA_Data& tie_overlapping = *(m_ports[PRT_TIE]);
  
  LADSPA_Data* output = m_ports[OUT];
  
  static const unsigned long param_slide_time = 60;
  static const LADSPA_Data twelfth_root_of_2 = pow(2, 1/12.0);
  
  unsigned long e = 0;
  
  //  cerr<<"got "<<event_count<<" MIDI events!"<<endl;
  
  // this is the main loop!
  for (unsigned long i = 0; i < sample_count; ++i) {
    
    unsigned long freq_slide_time = (unsigned long)(porta_time * m_frame_rate);
    unsigned long vel_slide_time = (unsigned long)(porta_time * m_frame_rate);
    
    // handle any MIDI events that occur in this frame
    while (e < event_count && events[e].time.tick == i) {
      
      // note on
      if (events[e].type == SND_SEQ_EVENT_NOTEON) {
	if (!m_note_is_on || tie_overlapping <= 0) {
	  m_adsr.on(m_last_frame);
	  if (porta_on <= 0) {
	    freq_slide_time = 0;
	    vel_slide_time = 30;
	  }
	}
	m_note_is_on = true;
	m_note = events[e].data.note.note;
	m_pitch = m_table[m_note];
	m_velocity = events[e].data.note.velocity / 128.0f;
	//cerr<<"note = "<<m_note<<", pitch = "<<m_pitch<<", vel = "<<m_velocity
	//  <<endl;
      }
      
      // note off, all notes off
      else if ((events[e].type == SND_SEQ_EVENT_NOTEOFF && 
		events[e].data.note.note == m_note) ||
	       (events[e].type == SND_SEQ_EVENT_CONTROLLER &&
		events[e].data.control.param == 123)) {
	m_adsr.off(m_last_frame);
	m_note_is_on = false;
      }
      
      // pitchbend
      else if (events[e].type == SND_SEQ_EVENT_PITCHBEND) {
	m_pitchbend = pow(twelfth_root_of_2, 
			  events[e].data.control.value / 4096.0f);
      }
      
      // all sound off
      else if (events[e].type == SND_SEQ_EVENT_CONTROLLER &&
	       events[e].data.control.param == 120) {
	// XXX This should really turn off the delay too
	m_adsr.fast_off(m_last_frame);
	m_note_is_on = false;
      }
      ++e;
    }
    
    // don't allow fast changes for controls that can produce clicks
    vel = m_vel_slide.run(m_velocity, vel_slide_time);
    LADSPA_Data osc_mix2 = m_osc_mix_slide.run(osc_mix, param_slide_time);
    LADSPA_Data trem_depth2 = m_trem_depth_slide.run(trem_depth, 
						    param_slide_time);
    LADSPA_Data att2 = m_att_slide.run(att, param_slide_time);
    LADSPA_Data dec2 = m_dec_slide.run(dec, param_slide_time);
    LADSPA_Data sus2 = m_sus_slide.run(sus, param_slide_time);
    LADSPA_Data rel2 = m_rel_slide.run(rel, param_slide_time);
    LADSPA_Data shp_env2 = m_shp_env_slide.run(shp_env, param_slide_time);
    LADSPA_Data shp_total2 = m_shp_total_slide.run(shp_total, param_slide_time);
    LADSPA_Data shp_split2 = m_shp_split_slide.run(shp_split, param_slide_time);
    LADSPA_Data shp_shift2 = m_shp_shift_slide.run(shp_shift, param_slide_time);
    LADSPA_Data lfo_depth2 = m_lfo_depth_slide.run(lfo_depth, param_slide_time);
    LADSPA_Data drive2 = m_drive_slide.run(drive, param_slide_time);
    LADSPA_Data gain2 = m_gain_slide.run(gain, param_slide_time);
    LADSPA_Data amp_env2 = m_amp_env_slide.run(amp_env, param_slide_time);
    LADSPA_Data delay_feedback2 = m_delay_fb_slide.run(delay_feedback, 
						       param_slide_time);
    LADSPA_Data delay_mix2 = m_delay_mix_slide.run(delay_mix, param_slide_time);
    
    // portamento
    freq = m_pitchbend * m_freq_slide.run(m_pitch, freq_slide_time);

    // add vibrato to the fundamental frequency
    freq += freq * vib_depth * m_vibrato_lfo.run(vib_freq);
    
    // run the envelope
    LADSPA_Data env = m_adsr.run(m_last_frame, att2, dec2, sus2, rel2);
    
    // run the oscillators
    LADSPA_Data osc1 = m_osc.run(tune * freq);
    LADSPA_Data osc2 = m_subosc.run(tune * subtune * freq);
    LADSPA_Data osc = vel * ((1 - osc_mix2) * osc1 + osc_mix2 * osc2);
    
    // compute the shape amounts for the two shapers
    shp_total2 *= (1.0f + lfo_depth2 * m_shaper_lfo.run(lfo_freq));
    LADSPA_Data shp2 = shp_total2 * shp_split2;
    LADSPA_Data shp1 = shp_total2 - shp2;
    
    // run the shapers (and apply the envelope to the shape amount for shaper 1)
    output[i] = m_shaper.run(m_shaper.run(osc, shp1 * (shp_env2 * env + 
						       (1 - shp_env2)), 0),
			     shp2, shp_shift2);
    
    // apply the envelope to the amplitude
    output[i] = output[i] * env * amp_env2 + output[i] * (1 - amp_env2);
    
    // apply tremolo
    output[i] *= (1.0f +  trem_depth2 * m_tremolo_lfo.run(trem_freq));
    
    // apply gain and overdrive
    output[i] = gain2 * (drive2 * atan((1 + 10 * drive2) * output[i]) + (1 - drive2) * output[i]);
    
    // run delay
    output[i] = m_delay.run(output[i], delay_time, delay_feedback2) * 
      delay_mix2 + output[i] * (1 - delay_mix2);
    
    // and DC blocker (does this actually help?)
    output[i] = m_blocker.run(output[i]);
    
    ++m_last_frame;
  }
  
}


int SineShaper::get_midi_controller_for_port(unsigned long port) {
  switch (port) {
    
  case TUN:
    return DSSI_CC(16);
    
  case OCT:
    return DSSI_CC(17);
    
  case SUB_TUN:
    return DSSI_CC(18);
    
  case SUB_OCT:
    return DSSI_CC(19);

  case OSC_MIX:
    return DSSI_CC(95);
    
  case PRT_ON:
    return DSSI_CC(65);
    
  case PRT_TIM:
    return DSSI_CC(5);
    
  case PRT_TIE:
    return DSSI_CC(68);
    
  case VIB_FRQ:
    return DSSI_CC(2);
    
  case VIB_DPT:
    return DSSI_CC(1);

  case TRM_FRQ:
    return DSSI_CC(75);
    
  case TRM_DPT:
    return DSSI_CC(92);
    
  case SHP_ENV:
    return DSSI_CC(71);
    
  case SHP_TOT:
    return DSSI_CC(74);
    
  case SHP_SPL:
    return DSSI_CC(70);
    
  case SHP_SHF:
    return DSSI_CC(76);
    
  case LFO_FRQ:
    return DSSI_CC(77);
    
  case LFO_DPT:
    return DSSI_CC(78);
    
  case ATT:
    return DSSI_CC(73);
    
  case DEC:
    return DSSI_CC(4);
      
  case SUS:
    return DSSI_CC(79);
    
  case REL:
    return DSSI_CC(72);
    
  case AMP_ENV:
    return DSSI_CC(93);
    
  case DRIVE:
    return DSSI_CC(94);
    
  case GAIN:
    return DSSI_CC(7);

  case DEL_TIM:
    return DSSI_CC(12);
    
  case DEL_FB:
    return DSSI_CC(13);
    
  case DEL_MIX:
    return DSSI_CC(91);
    
  default:
    return DSSI_NONE;
  }
}


const DSSI_Program_Descriptor* SineShaper::get_program(unsigned long index) {
  // the descriptor returned will be overwritten next time someone calls this
  // function, but that is OK according to the RFC
  memset(&m_prog, 0, sizeof(DSSI_Program_Descriptor));
  PresetManager* pm = const_cast<PresetManager*>(m_pm);
  if (index < pm->get_bank(0).size()) {
    m_prog.Bank = 0;
    m_prog.Program = pm->get_bank(0)[index].number;
    m_prog.Name = pm->get_bank(0)[index].name.c_str();
    return &m_prog;
  }
  else if ((index -= pm->get_bank(0).size()) < pm->get_bank(1).size()) {
    m_prog.Bank = 1;
    m_prog.Program = pm->get_bank(1)[index].number;
    m_prog.Name = pm->get_bank(1)[index].name.c_str();
    return &m_prog;
  }
  return NULL;
}


void SineShaper::select_program(unsigned long bank, unsigned long program) {
  PresetManager* pm = const_cast<PresetManager*>(m_pm);
  if (bank > 1)
    return;
  const vector<PresetManager::Preset>& bank_data = pm->get_bank(bank);
  unsigned index;
  for (index = 0; index < bank_data.size(); ++index) {
    if (bank_data[index].number == program)
      break;
  }
  if (index < bank_data.size()) {
    for (unsigned i = 0; i < INPUT_PORTS; ++i)
      m_ports[i][0] = pm->get_bank(bank)[index].values[i];
  }
}


char* SineShaper::configure(const char* key, const char* value) {
  if (!strcmp(key, "reloadprograms")) {
    PresetManager* tmp = new PresetManager;
    *tmp = *const_cast<PresetManager*>(m_pm);
    tmp->reload_bank(1, string(getenv("HOME")) + "/.sineshaperpresets", 
		     INPUT_PORTS);
    PresetManager* tmp2 = const_cast<PresetManager*>(m_pm);
    m_pm = tmp;
    // XXX DANGEROUS! The preset manager could be used by select_program(),
    //     which is an audio class function and could run at the same time
    //     as this configure call. Must get a confirmation from an audio 
    //     class function that this PresetManager is not used.
    delete tmp2;
  }
  return NULL;
}


void initialise() {
  DSSIPortList ports;
  
  // control ports
  LADSPA_PortDescriptor c_desc = LADSPA_PORT_CONTROL | LADSPA_PORT_INPUT;
  LADSPA_PortRangeHintDescriptor r_desc = LADSPA_HINT_BOUNDED_BELOW | 
    LADSPA_HINT_BOUNDED_ABOVE;
  LADSPA_PortRangeHintDescriptor i_desc = LADSPA_HINT_BOUNDED_BELOW | 
    LADSPA_HINT_BOUNDED_ABOVE | LADSPA_HINT_INTEGER;
  LADSPA_PortRangeHintDescriptor t_desc = LADSPA_HINT_TOGGLED;
  LADSPA_PortRangeHintDescriptor d_middle = LADSPA_HINT_DEFAULT_MIDDLE;
  LADSPA_PortRangeHintDescriptor d_low = LADSPA_HINT_DEFAULT_MIDDLE;
  LADSPA_PortRangeHintDescriptor d_high = LADSPA_HINT_DEFAULT_MIDDLE;
  LADSPA_PortRangeHintDescriptor d_0 = LADSPA_HINT_DEFAULT_0;
  
  ports.add_port(c_desc, "Tune", r_desc | d_high, 0.5, 2);
  ports.add_port(c_desc, "Octave", i_desc | d_middle, -10, 10);
  
  ports.add_port(c_desc, "Sub oscillator tune", r_desc | d_high, 0.5, 2);
  ports.add_port(c_desc, "Sub oscillator octave", i_desc | d_middle, -10, 10);
  ports.add_port(c_desc, "Oscillator mix", r_desc | d_middle, 0, 1);
  
  ports.add_port(c_desc, "Portamento on", t_desc | d_0, -1, 1);
  ports.add_port(c_desc, "Portamento time", r_desc | d_low, 0.001, 3);
  ports.add_port(c_desc, "Tie overlapping notes", t_desc | d_0, -1, 1);

  ports.add_port(c_desc, "Vibrato frequency", r_desc, 0, 10);
  ports.add_port(c_desc, "Vibrato depth", r_desc | d_low, 0, 0.25);

  ports.add_port(c_desc, "Tremolo frequency", r_desc, 0, 10);
  ports.add_port(c_desc, "Tremolo depth", r_desc | d_low, 0, 1);
  
  ports.add_port(c_desc, "Shaper envelope sensitivity", r_desc | d_high, 0, 1);
  ports.add_port(c_desc, "Shape amount", r_desc | d_middle, 0, 6);
  ports.add_port(c_desc, "Shape split", r_desc | d_middle, 0, 1);
  ports.add_port(c_desc, "Shape shift", r_desc | d_low, 0, 1);
  ports.add_port(c_desc, "Shaper LFO frequency", r_desc, 0, 10);
  ports.add_port(c_desc, "Shaper LFO depth", r_desc | d_low, 0, 1);
  
  ports.add_port(c_desc, "Attack", r_desc | d_low, 0.0005, 1);
  ports.add_port(c_desc, "Decay", r_desc | d_middle, 0.0005, 1);
  ports.add_port(c_desc, "Sustain", r_desc | d_middle, 0, 1);
  ports.add_port(c_desc, "Release", r_desc | d_middle, 0.0005, 3);
  
  ports.add_port(c_desc, "Amp envelope sensitivity", r_desc | d_low, 0, 1);
  ports.add_port(c_desc, "Drive", r_desc | d_low, 0, 1);
  ports.add_port(c_desc, "Gain", r_desc | d_middle, 0, 2);
  
  ports.add_port(c_desc, "Delay time", r_desc | d_low, 0, 3);
  ports.add_port(c_desc, "Delay feedback", r_desc | d_middle, 0, 1);
  ports.add_port(c_desc, "Delay mix", r_desc | d_low, 0, 1);

  // audio output
  ports.add_port(LADSPA_PORT_AUDIO | LADSPA_PORT_OUTPUT, "Output");
  
  register_dssi<SineShaper>(2746, "ll-sineshaper", 0, "Sineshaper", 
			    "Lars Luthman", "GPL", ports);
}

