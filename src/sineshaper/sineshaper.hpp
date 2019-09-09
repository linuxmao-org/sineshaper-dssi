/****************************************************************************
    
    sineshaper.hpp - A DSSI synth plugin
    
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

#ifndef SINESHAPER_HPP
#define SINESHAPER_HPP

#include "dssiplugin.hpp"
#include "sineoscillator.hpp"
#include "adsr.hpp"
#include "frequencytable.hpp"
#include "wavewrapper.hpp"
#include "slide.hpp"
#include "presetmanager.hpp"
#include "dcblocker.hpp"
#include "delay.hpp"


/** This is the class that contains all the code and data for the Sineshaper
    synth plugin. */
class SineShaper : public DSSIPlugin {
public:
  
  SineShaper(unsigned long frame_rate);
  
  void run_synth(unsigned long sample_count, snd_seq_event_t* events,
		 unsigned long event_count);
  int get_midi_controller_for_port(unsigned long port);
  
  const DSSI_Program_Descriptor* get_program(unsigned long index);
  void select_program(unsigned long bank, unsigned long program);
  
  char* configure(const char* key, const char* value);
  
protected:
  
  SineOscillator m_vibrato_lfo;
  SineOscillator m_tremolo_lfo;
  SineOscillator m_shaper_lfo;
  SineOscillator m_osc;
  SineOscillator m_subosc;
  ADSR m_adsr;
  FrequencyTable m_table;
  WaveWrapper m_shaper;
  Slide m_freq_slide;
  Slide m_vel_slide;
  Delay m_delay;
  Slide m_osc_mix_slide;
  Slide m_trem_depth_slide;
  Slide m_shp_env_slide;
  Slide m_shp_total_slide;
  Slide m_shp_split_slide;
  Slide m_shp_shift_slide;
  Slide m_lfo_depth_slide;
  Slide m_att_slide;
  Slide m_dec_slide;
  Slide m_sus_slide;
  Slide m_rel_slide;
  Slide m_amp_env_slide;
  Slide m_drive_slide;
  Slide m_gain_slide;
  Slide m_delay_fb_slide;
  Slide m_delay_mix_slide;
  DCBlocker m_blocker;
  
  unsigned long m_frame_rate;
  unsigned long m_last_frame;
  
  LADSPA_Data m_velocity;
  LADSPA_Data m_pitch;
  unsigned char m_note;
  bool m_note_is_on;
  
  /** This needs to be volatile because it can be changed by a configure() call,
      which may run in another thread. */
  volatile PresetManager* m_pm;
  
  DSSI_Program_Descriptor m_prog;
  
  LADSPA_Data m_pitchbend;
};


#endif
