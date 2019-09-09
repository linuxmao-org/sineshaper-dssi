/****************************************************************************
    
    presetmanager.hpp - A class that manages presets for DSSI plugins
    
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

#ifndef PRESETMANAGER_HPP
#define PRESETMANAGER_HPP

#include <map>
#include <vector>
#include <string>


using namespace std;


/** This class loads and writes preset banks consisting of control 
    port values. */
class PresetManager {
public:
  
  struct Preset {
    unsigned long number;
    string name;
    vector<double> values;
  };
  
  /** Return bank @c bank. */
  const vector<Preset>& get_bank(int bank) const;
  
  /** Add a preset (this function does not save the preset to file, you have to 
      do that with save_bank()). */
  bool add_preset(unsigned long bank, unsigned long number, const string& name, 
		  const vector<double>& values, bool force = false);
  
  /** Save a bank to a file. */
  bool save_bank(unsigned long bank, const string& filename);
  
  /** Load a bank from file. @c n is the number of control ports. */
  int load_bank(const string& filename, int n);
  
  /** Load a bank and replace an already loaded bank. */
  int reload_bank(unsigned long bank, const string& filename, int n);
  
protected:
  
  vector< vector<Preset> > m_banks;
  
};


#endif
