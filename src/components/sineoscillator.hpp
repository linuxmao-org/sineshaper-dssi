/****************************************************************************
    
    sineoscillator.hpp - a bandlimited sine oscillator for use in DSSI plugins
    
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

/** Based on the Sine plugin in the CMT package by Richard Furse */

#ifndef SINEOSCILLATOR_HPP
#define SINEOSCILLATOR_HPP

#include <cmath>

#include <ladspa.h>

#define SINE_TABLE_BITS 14
#define SINE_TABLE_SHIFT (8 * sizeof(unsigned long) - SINE_TABLE_BITS)


using namespace std;


class SineOscillator {
public:
  
  inline SineOscillator(unsigned long frame_rate);
  
  inline void reset();
  inline LADSPA_Data run(LADSPA_Data frequency);
  
protected:
  
  inline void set_frequency(LADSPA_Data frequency);
  
  static inline void initialise_table();
  
  unsigned long m_phase;
  unsigned long m_phase_step;
  LADSPA_Data m_cached_frequency;
  const LADSPA_Data m_limit_frequency;
  LADSPA_Data m_phase_step_scalar;

  static LADSPA_Data m_sine[1 << SINE_TABLE_BITS];
  static LADSPA_Data m_phase_step_base;
  static bool m_initialised;
};


LADSPA_Data SineOscillator::m_sine[1 << SINE_TABLE_BITS];
LADSPA_Data SineOscillator::m_phase_step_base;
bool SineOscillator::m_initialised(false);


SineOscillator::SineOscillator(unsigned long frame_rate)
  : m_phase(0), m_phase_step(0), m_cached_frequency(0), 
    m_limit_frequency(LADSPA_Data(frame_rate * 0.5)) {

  if (!m_initialised)
    initialise_table();
  
  m_phase_step_scalar = LADSPA_Data(m_phase_step_base / frame_rate);
}


void SineOscillator::reset() {
  m_phase = 0;
}


LADSPA_Data SineOscillator::run(LADSPA_Data frequency) {
  LADSPA_Data result = m_sine[m_phase >> SINE_TABLE_SHIFT];
  set_frequency(frequency);
  m_phase += m_phase_step;
  return result;
}


void SineOscillator::set_frequency(LADSPA_Data frequency) {
  if (frequency != m_cached_frequency) {
    if (frequency >= 0 && frequency < m_limit_frequency) 
      m_phase_step = (unsigned long)(m_phase_step_scalar * frequency);
    else 
      m_phase_step = 0;
    m_cached_frequency = frequency;
  }
}


void SineOscillator::initialise_table() {
  unsigned long lTableSize = 1 << SINE_TABLE_BITS;
  double dShift = (double(M_PI) * 2) / lTableSize;
  for (unsigned long lIndex = 0; lIndex < lTableSize; lIndex++)
    m_sine[lIndex] = LADSPA_Data(sin(dShift * lIndex));
  m_phase_step_base = (LADSPA_Data)pow(2.0f, int(sizeof(unsigned long) * 8));
}


#endif
