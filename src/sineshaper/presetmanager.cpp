/****************************************************************************
    
    presetmanager.cpp - A class that manages presets for DSSI plugins
    
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

#include <cstdlib>
//#include <iostream>
#include <sstream>
#include <fstream>

#include "presetmanager.hpp"


using namespace std;


const vector<PresetManager::Preset>& PresetManager::get_bank(int bank) const {
  return m_banks[bank];
}
 

bool PresetManager::add_preset(unsigned long bank, unsigned long number, 
			       const string& name, const vector<double>& values,
			       bool force) {
  Preset tmp;
  Preset *p = &tmp;
  unsigned long next = 0;
  vector<Preset>& presets = m_banks[bank];
  for (size_t i = 0; i < presets.size(); ++i) {
    if (presets[i].number == number) {
      if (!force)
	return false;
      else {
	p = &presets[i];
	break;
      }
    }
    if (next <= presets[i].number)
      next = presets[i].number + 1;
  }
  p->name = name;
  p->values = values;
  if (p == &tmp) {
    p->number = next;
    presets.push_back(tmp);
  }  
  return true;
}
 

bool PresetManager::save_bank(unsigned long bank, const string& filename) {
  if (bank < m_banks.size()) {
    ofstream ofs(filename.c_str());
    for (unsigned i = 0; i < m_banks[bank].size(); ++i) {
      ofs<<m_banks[bank][i].number<<'\t'<<m_banks[bank][i].name;
      for (unsigned j = 0; j < m_banks[bank][i].values.size(); ++j)
	ofs<<'\t'<<m_banks[bank][i].values[j];
      ofs<<endl;
    }
    return true;
  }
  return false;
}


int PresetManager::load_bank(const string& filename, int n) {
  vector<Preset> presets;
  ifstream ifs(filename.c_str());
  while (ifs.good()) {
    char line_c[1025];
    ifs.getline(line_c, 1024);
    string line(line_c);
    int i = line.find('\t');
    if (i == -1)
      break;
    unsigned preset_number = atoi(line.substr(0, i).c_str());
    line = line.substr(i + 1);
    i = line.find('\t');
    if (i == -1)
      break;
    string preset_name = line.substr(0, i);
    istringstream iss(line.substr(i + 1));
    Preset p;
    for (int k = 0; k < n; ++k) {
      double d;
      iss>>d;
      p.values.push_back(d);
    }
    p.name = preset_name;
    p.number = preset_number;
    presets.push_back(p);
  }
  m_banks.push_back(presets);
  return m_banks.size() - 1;
}


int PresetManager::reload_bank(unsigned long bank, const string& filename, 
			       int n) {
  vector<Preset> presets;
  ifstream ifs(filename.c_str());
  while (ifs.good()) {
    char line_c[1025];
    ifs.getline(line_c, 1024);
    string line(line_c);
    int i = line.find('\t');
    if (i == -1)
      break;
    unsigned preset_number = atoi(line.substr(0, i).c_str());
    line = line.substr(i + 1);
    i = line.find('\t');
    if (i == -1)
      break;
    string preset_name = line.substr(0, i);
    istringstream iss(line.substr(i + 1));
    Preset p;
    for (int k = 0; k < n; ++k) {
      double d;
      iss>>d;
      p.values.push_back(d);
    }
    p.name = preset_name;
    p.number = preset_number;
    presets.push_back(p);
  }
  m_banks[bank] = presets;
  return bank;  
}
