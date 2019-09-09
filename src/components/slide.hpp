/****************************************************************************
    
    slide.hpp - a slew limiter for use in DSSI plugins
    
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

#ifndef SLIDE_HPP
#define SLIDE_HPP


class Slide {
public:
  
  inline Slide(unsigned long frame_rate);
  
  inline LADSPA_Data run(LADSPA_Data input, unsigned long slide_time);
  
protected:
  
  unsigned long m_last_change;
  LADSPA_Data m_from;
  LADSPA_Data m_to;
  LADSPA_Data m_last_output;
};


Slide::Slide(unsigned long) : m_from(0), m_to(0), m_last_output(0) {

}


LADSPA_Data Slide::run(LADSPA_Data input, unsigned long slide_time) {
  if (input != m_to) {
    m_from = m_last_output;
    m_to = input;
  }
  
  LADSPA_Data result;
  
  if (slide_time == 0) 
    result = input;
  else {
    LADSPA_Data increment = (m_to - m_from) / slide_time;
    bool rising = (m_to > m_from);
    result = m_last_output + increment;
    if ((result > m_to && rising) || (result < m_to && !rising))
      result = m_to;
  }
  
  m_last_output = result;
  return result;
}


#endif
