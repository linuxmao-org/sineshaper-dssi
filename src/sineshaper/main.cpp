/****************************************************************************
    
    main.cpp - the main file for the Sineshaper GUI
    
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

#include <iostream>
#include <string>

#include <gtkmm.h>
#include <libglademm.h>
#include <ladspa.h>

#include "dssiuiclient.hpp"
#include "skindial_gtkmm.hpp"
#include "sineshapergui.hpp"


using namespace std;
using namespace Gdk;
using namespace Gtk;
using namespace Gnome::Glade;
using namespace Glib;


int main(int argc, char** argv) {
  
  // create the objects
  DSSIUIClient dssi(argc, argv, true);
  if (!dssi.is_valid()) {
    cerr<<"Could not start OSC listener. You are not running the "<<endl
	<<"program manually, are you? It's supposed to be started by "<<endl
	<<"the plugin host."<<endl;
    return 1;
  }
  Main kit(argc, argv);
  RefPtr<Xml> xml = Xml::create(INSTALL_DIR "/sineshaper/sineshaper.glade");

  SineShaperGUI* main_win = xml->get_widget_derived("main_win", main_win);
  main_win->set_title(string("Sineshaper: ") + dssi.get_identifier());

  // connect signals
  main_win->connect_all(dssi);
  main_win->signal_select_program.
    connect(mem_fun(dssi, &DSSIUIClient::send_program));
  main_win->signal_programs_changed.
    connect(bind<0>(bind<0>(mem_fun(dssi, &DSSIUIClient::send_configure), 
			    "reloadprograms"), ""));
  dssi.program_received.
    connect(mem_fun(*main_win, &SineShaperGUI::program_selected));
  dssi.show_received.connect(mem_fun(*main_win, &SineShaperGUI::show_all));
  dssi.hide_received.connect(mem_fun(*main_win, &SineShaperGUI::hide));
  dssi.quit_received.connect(&Main::quit);
  main_win->signal_delete_event().connect(bind_return(hide(&Main::quit), true));
  
  // start
  dssi.send_update_request();
  Main::run();
  
  return 0;
}


