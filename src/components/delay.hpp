/****************************************************************************
    
    delay.hpp - a simple feedback delay for use in DSSI plugins
    
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

#ifndef DELAY_HPP
#define DELAY_HPP

#include <ladspa.h>


class Delay {
public:
  inline Delay(unsigned long frame_rate, LADSPA_Data max_delay);
  inline ~Delay();
  inline void reset();
  inline LADSPA_Data run(LADSPA_Data input, LADSPA_Data time, 
			 LADSPA_Data feedback);
protected:
  
  LADSPA_Data* m_delay_line;
  unsigned long m_line_size;
  unsigned long m_frame_rate;
  unsigned long m_write_pos;
};


Delay::Delay(unsigned long frame_rate, LADSPA_Data max_delay) {
  m_line_size = (unsigned long)(max_delay * frame_rate + 1);
  m_delay_line = new LADSPA_Data[m_line_size];
  m_write_pos = 0;
  m_frame_rate = frame_rate;
  reset();
}


Delay::~Delay() {
  delete [] m_delay_line;
}


void Delay::reset() {
  memset(m_delay_line, 0, m_line_size);
}


LADSPA_Data Delay::run(LADSPA_Data input, LADSPA_Data time,
		       LADSPA_Data feedback) {
  m_write_pos = (m_write_pos + 1) % m_line_size;
  unsigned long read_pos = (m_write_pos + m_line_size - 
			    (unsigned long)(time * m_frame_rate)) % m_line_size;
  m_delay_line[m_write_pos] = input + feedback * m_delay_line[read_pos];
  return m_delay_line[read_pos];
}


#endif
