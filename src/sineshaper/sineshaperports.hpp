/****************************************************************************
    
    sineshaperports.hpp - A list of all ports in the Sineshaper synth
    
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

#ifndef SINESHAPERPORTS_HPP
#define SINESHAPERPORTS_HPP


enum SineShaperPorts {
  TUN = 0,
  OCT,
  
  SUB_TUN,
  SUB_OCT,
  OSC_MIX,

  PRT_ON,
  PRT_TIM,
  PRT_TIE,
  
  VIB_FRQ,
  VIB_DPT,

  TRM_FRQ,
  TRM_DPT,

  SHP_ENV,
  SHP_TOT,
  SHP_SPL,
  SHP_SHF,
  LFO_FRQ,
  LFO_DPT,
  
  ATT,
  DEC,
  SUS,
  REL,
  
  AMP_ENV,
  DRIVE,
  GAIN,
  
  DEL_TIM,
  DEL_FB,
  DEL_MIX,
  
  OUT
};


static const unsigned int INPUT_PORTS = DEL_MIX + 1;

#endif
