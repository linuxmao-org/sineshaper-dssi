/****************************************************************************
    
    polyphonicmidinote.hpp - a MIDI event handler for use in DSSI plugins
    
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

#ifndef POLYPHONICMIDINOTE_HPP
#define POLYPHONICMIDINOTE_HPP

#include <iostream>

#include <dssi.h>

#include "frequencytable.hpp"


using namespace std;


class PolyphonicMidiNote {
public:
  
  inline PolyphonicMidiNote(unsigned long samplerate);
  
  inline const LADSPA_Data run(snd_seq_event_t* events, 
			       unsigned long event_count, 
			       unsigned long& next_event,
			       unsigned long tick);
  
  inline const LADSPA_Data& get_gate(int voice) const;
  inline const LADSPA_Data get_frequency(int voice) const;
  inline const LADSPA_Data& get_velocity(int voice) const;
  inline const bool& get_trigger(int voice) const;
  
private:
  
  LADSPA_Data m_gate[4];
  unsigned char m_note[4];
  LADSPA_Data m_frequency[4];
  LADSPA_Data m_bend_factor;
  LADSPA_Data m_velocity[4];
  bool m_trigger[4];
  FrequencyTable m_table;
  
};


PolyphonicMidiNote::PolyphonicMidiNote(unsigned long) : m_bend_factor(1) {
  for (int i = 0; i < 4; ++i) {
    m_gate[i] = 0;
    m_frequency[i] = 0;
    m_trigger[i] = false;
    m_note[i] = 0;
  }
  
}


const LADSPA_Data PolyphonicMidiNote::run(snd_seq_event_t* events, 
					  unsigned long event_count,
					  unsigned long& next_event,
					  unsigned long tick) {
  static int next_voice = 0;
  for (int i = 0; i < 4; ++i)
    m_trigger[i] = false;
  
  while (next_event < event_count && events[next_event].time.tick == tick) {
    if (events[next_event].type == SND_SEQ_EVENT_NOTEON) {
      //cerr<<"NOTE ON, VOICE "<<next_voice<<endl;
      m_trigger[next_voice] = true;
      m_gate[next_voice] = 1.0f;
      m_note[next_voice] = events[next_event].data.note.note;
      m_frequency[next_voice] = m_table[m_note[next_voice]];
      m_velocity[next_voice] = events[next_event].data.note.velocity / 128.0f;
      next_voice = (next_voice + 1) % 4;
    }
    else if (events[next_event].type == SND_SEQ_EVENT_NOTEOFF) {
      for (int i = 0; i < 4; ++i) {
	if (events[next_event].data.note.note == m_note[i]) {
	  //cerr<<"NOTE OFF, VOICE "<<i<<endl;
	  m_gate[i] = 0.0f;
	}
      }
    }
    else if (events[next_event].type == SND_SEQ_EVENT_PITCHBEND) {
      int bend = events[next_event].data.control.value;
      m_bend_factor = pow(2.0f, (LADSPA_Data(bend) / 8192.0f));
    }
    ++next_event;
  }
  
  return m_frequency[0] * m_bend_factor;
}


const LADSPA_Data& PolyphonicMidiNote::get_gate(int voice) const {
  return m_gate[voice];
}


const LADSPA_Data PolyphonicMidiNote::get_frequency(int voice) const {
  return m_frequency[voice] * m_bend_factor;
}


const LADSPA_Data& PolyphonicMidiNote::get_velocity(int voice) const {
  return m_velocity[voice];
}


const bool& PolyphonicMidiNote::get_trigger(int voice) const {
  return m_trigger[voice];
}





#endif
