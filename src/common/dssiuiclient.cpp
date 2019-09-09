/****************************************************************************
    
    dssiuiclient.cpp - a class that makes writing DSSI GUIs easier
    
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

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/stat.h>

#include <iostream>
#include <sstream>

#include <glibmm.h>

#include "dssi_shm.h"
#include "dssiuiclient.hpp"


using namespace Glib;
using namespace sigc;


DSSIUIClient::DSSIUIClient(int argc, char** argv, bool wait) 
  : m_plugin_flag(NULL), m_valid(false), m_blocking(false) {
  
  if (argc < 5) {
    cerr<<"Not enough arguments passed to the DSSIUIClient constructor! "<<endl;
    return;
  }
  
  /*
  cerr<<"Plugin URL: "<<argv[1]<<endl
      <<"Plugin library: "<<argv[2]<<endl
      <<"Plugin label: "<<argv[3]<<endl
      <<"Plugin identifier: "<<argv[4]<<endl;
  */
  
  m_identifier = argv[4];
  
  if(!thread_supported())
    thread_init();
  
  m_control_dispatcher.
    connect(mem_fun(*this, &DSSIUIClient::control_receiver));
  m_program_dispatcher.
    connect(mem_fun(*this, &DSSIUIClient::program_receiver));
  m_configure_dispatcher.
    connect(mem_fun(*this, &DSSIUIClient::configure_receiver));
  
  control_received.connect(mem_fun(*this, &DSSIUIClient::update_adjustments));

  m_plugin_address = lo_address_new_from_url(argv[1]);
  m_plugin_path = lo_url_get_path(argv[1]);
  m_server_thread = lo_server_thread_new(NULL, NULL);

  lo_server_thread_add_method(m_server_thread, "/scope/control", "if",
			      &DSSIUIClient::control_handler, this);
  lo_server_thread_add_method(m_server_thread, "/scope/program", "ii", 
			      &DSSIUIClient::program_handler, this);
  lo_server_thread_add_method(m_server_thread, "/scope/configure", "ss", 
			      &DSSIUIClient::configure_handler, this);
  lo_server_thread_add_method(m_server_thread, "/scope/show", "", 
			      &DSSIUIClient::show_handler, this);
  lo_server_thread_add_method(m_server_thread, "/scope/hide", "", 
			      &DSSIUIClient::hide_handler, this);
  lo_server_thread_add_method(m_server_thread, "/scope/quit", "", 
			      &DSSIUIClient::quit_handler, this);

  lo_server_thread_start(m_server_thread);
  
  m_valid = true;
  
  if (!wait) {
    send_update_request();
  }
}


DSSIUIClient::~DSSIUIClient() {
  if (m_shm_key.size() > 0) {
    dssi_shm_free(m_shm_key.c_str());
    if (m_valid)
      send_configure("shm_detach", m_shm_key.c_str());
    m_shm_key = "";
  }
  if (m_valid) {
    send_exiting();
    lo_server_thread_stop(m_server_thread);
    lo_server_thread_free(m_server_thread);
  }
}


const string& DSSIUIClient::get_identifier() const {
  return m_identifier;
}


void DSSIUIClient::connect_gui(RefPtr<Xml> xml) {
  // XXX Find a smarter way to do this!
  for (int i = 0; i < 100; ++i) {
    ostringstream oss;
    oss<<"dssi_"<<i<<"_linear";
    Widget* w = xml->get_widget(oss.str());
    if (w) {
      cerr<<"Yes, found "<<oss.str()<<endl;
      // check standard gtkmm widgets
      if (dynamic_cast<Range*>(w))
	connect_adjustment(dynamic_cast<Range*>(w)->get_adjustment(), i);
      else if (dynamic_cast<SpinButton*>(w))
	connect_adjustment(dynamic_cast<SpinButton*>(w)->get_adjustment(), i);
      // XXX check registered custom widgets
      else {
	
      }
    }
  }
}


void DSSIUIClient::connect_adjustment(Adjustment* adj, int port) {
  adj->signal_value_changed().
    connect(compose(bind<0>(mem_fun(*this, &DSSIUIClient::send_control), port),
		    mem_fun(*adj, &Adjustment::get_value)));
  if (m_adjustments.size() <= unsigned(port))
    m_adjustments.resize(port + 1, NULL);
  m_adjustments[port] = adj;
}


void DSSIUIClient::send_control(int port, float value) {
  if (m_valid && !m_blocking) {
    lo_send(m_plugin_address, string(m_plugin_path + "/control").c_str(),
	    "if", port, value);
  }
}


void DSSIUIClient::send_program(int bank, int program) {
  if (m_valid && !m_blocking) {
    lo_send(m_plugin_address, string(m_plugin_path + "/program").c_str(),
	    "ii", bank, program);
    /* The host does not send a /program back when we told it to change
       program, so we fire off the signal directly instead */
    program_received(bank, program);
  }
}


void DSSIUIClient::send_update_request() {
  if (m_valid) {
    char* my_url = lo_server_thread_get_url(m_server_thread);
    string url(my_url);
    free(my_url);
    lo_send(m_plugin_address, (m_plugin_path + "/update").c_str(), 
	    "s", (url + "scope").c_str());
  }
}


void DSSIUIClient::send_configure(const string& key, const string& value) {
  if (m_valid) {
    lo_send(m_plugin_address, (m_plugin_path + "/configure").c_str(),
	    "ss", key.c_str(), value.c_str());
  }
}


void DSSIUIClient::send_midi(const char event[4]) {
  if (m_valid) {
    lo_send(m_plugin_address, (m_plugin_path + "/midi").c_str(), "s", event);
  }
}


void DSSIUIClient::send_exiting() {
  if (m_valid) {
    lo_send(m_plugin_address, (m_plugin_path + "/exiting").c_str(), NULL);
  }
}


void* DSSIUIClient::allocate_shared_memory(int bytes) {
  if (m_valid && m_shm_key.size() == 0) {
    char* key_str;
    m_plugin_flag = NULL;
    void* ptr = dssi_shm_allocate(bytes, &key_str, &m_plugin_flag);
    m_shm_key = key_str;
    free(key_str);
    signal_timeout().connect(mem_fun(*this, &DSSIUIClient::check_shared_memory),
			     10);
    send_configure("shm_attach", m_shm_key);
    return ptr;
  }
  return NULL;
}


bool DSSIUIClient::plugin_has_attached() {
  return (m_plugin_flag != NULL && *m_plugin_flag != 0);
}


void DSSIUIClient::update_adjustments(int port, float value) {
  
}


int DSSIUIClient::control_handler(const char *path, const char *types,
				  lo_arg **argv, int argc, 
				  void *data, void *user_data) {
  DSSIUIClient* me = static_cast<DSSIUIClient*>(user_data);
  me->m_control_queue.push(make_pair(argv[0]->i, argv[1]->f));
  me->m_control_dispatcher();
  return 0;
}


int DSSIUIClient::program_handler(const char *path, const char *types,
				  lo_arg **argv, int argc, 
				  void *data, void *user_data) {
  DSSIUIClient* me = static_cast<DSSIUIClient*>(user_data);
  me->m_program_queue.push(make_pair(argv[0]->i, argv[1]->i));
  me->m_program_dispatcher();
  return 0;
}


int DSSIUIClient::configure_handler(const char *path, const char *types,
				    lo_arg **argv, int argc, 
				    void *data, void *user_data) {
  DSSIUIClient* me = static_cast<DSSIUIClient*>(user_data);
  string key(&argv[0]->s);
  string value(&argv[1]->s);
  me->m_configure_queue.push(make_pair(key, value));
  me->m_configure_dispatcher();
  return 0;
}


int DSSIUIClient::show_handler(const char *path, const char *types,
			       lo_arg **argv, int argc, 
			       void *data, void *user_data) {
  DSSIUIClient* me = static_cast<DSSIUIClient*>(user_data);
  me->show_received();
  return 0;
}


int DSSIUIClient::hide_handler(const char *path, const char *types,
			       lo_arg **argv, int argc, 
			       void *data, void *user_data) {
  DSSIUIClient* me = static_cast<DSSIUIClient*>(user_data);
  me->hide_received();
  return 0;
}


int DSSIUIClient::quit_handler(const char *path, const char *types,
			       lo_arg **argv, int argc, 
			       void *data, void *user_data) {
  DSSIUIClient* me = static_cast<DSSIUIClient*>(user_data);
  me->m_valid = false;
  me->quit_received();
  lo_server_thread_stop(me->m_server_thread);
  return 0;
}


bool DSSIUIClient::check_shared_memory() {
  if (m_plugin_flag != NULL && *m_plugin_flag != 0) {
    plugin_attached();
    return false;
  }
  return true;
}


bool DSSIUIClient::is_valid() const {
  return m_valid;
}
