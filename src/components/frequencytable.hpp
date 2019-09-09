/****************************************************************************
    
    frequencytable.hpp - a frequency table for use in DSSI plugins
    
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

#ifndef FREQUENCYTABLE_HPP
#define FREQUENCYTABLE_HPP

#include <iostream>
#include <cmath>


using namespace std;


class FrequencyTable {
public:
  
  inline FrequencyTable();
  inline LADSPA_Data operator[](int i);
  
protected:
  
  LADSPA_Data m_table[128];
  
};


FrequencyTable::FrequencyTable() {
  for (int i = 0; i < 11; ++i) {
    LADSPA_Data A = 13.75 * pow(2.0, i);
    for (int n = 0; n < 12; ++n) {
      if (12 * i + n < 128)
	m_table[12 * i + n] = A * pow(2.0, (n - 9.0) / 12.0);
    }
  }
}


LADSPA_Data FrequencyTable::operator[](int i) {
  return m_table[i];
}


#endif
