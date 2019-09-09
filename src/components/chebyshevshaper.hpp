/****************************************************************************
    
    chebyshev.hpp - a simple Chebyshev waveshaper for use in DSSI plugins
    
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

#ifndef CHEBYSHEVSHAPER_HPP
#define CHEBYSHEVSHAPER_HPP

#include <cmath>
#include <cstring>
#include <iostream>

#include <ladspa.h>


using namespace std;


class ChebyshevShaper {
public:
  
  inline ChebyshevShaper(unsigned long frame_rate);
  
  inline LADSPA_Data run(LADSPA_Data input, LADSPA_Data freq);
  
  inline void set_coefficients(int n, LADSPA_Data* coefficients);

private:
  
  int m_order;
  LADSPA_Data* m_coeffs;
  unsigned long m_frame_rate;
};


ChebyshevShaper::ChebyshevShaper(unsigned long frame_rate) 
  : m_order(0), m_coeffs(NULL), m_frame_rate(frame_rate) { 
 
}


LADSPA_Data ChebyshevShaper::run(LADSPA_Data input, LADSPA_Data freq) {

  LADSPA_Data result = 0;
  LADSPA_Data poly_value0 = 1;
  LADSPA_Data poly_value1 = input;
  if (m_order > 0)
    result = m_coeffs[1] * poly_value1;
  for (int i = 2; i <= m_order; ++i) {
    if (freq * i > m_frame_rate / 2)
      break;
    LADSPA_Data poly_value = 2.0f * input * poly_value1 - poly_value0;
    result += m_coeffs[i] * poly_value;
    poly_value0 = poly_value1;
    poly_value1 = poly_value;
  }
  
  return result;
}


void ChebyshevShaper::set_coefficients(int n, LADSPA_Data* coefficients) {
  m_order = n;
  LADSPA_Data* tmp = m_coeffs;
  m_coeffs = coefficients;
  delete [] tmp;
}


#endif
