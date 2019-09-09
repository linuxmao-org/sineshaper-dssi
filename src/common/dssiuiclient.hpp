/****************************************************************************
    
    dssiuiclient.hpp - a class that makes writing DSSI GUIs easier
    
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

#ifndef DSSIUICLIENT_HPP
#define DSSIUICLIENT_HPP

#include <queue>
#include <string>
#include <utility>
#include <vector>

#include <glibmm.h>
#include <gtkmm.h>
#include <libglademm.h>
#include <lo/lo.h>
#include <sigc++/sigc++.h>


using namespace Glib;
using namespace Gnome::Glade;
using namespace Gtk;
using namespace sigc;
using namespace std;


/** This class can be used by a DSSI plugin UI to communicate with the plugin
    host. It has public functions for all the messages that the UI can send
    to the host, and public signals for all the messages that the host can send
    to the UI, which will be emitted when a message is received. It also handles
    all the required communication with the host, such as sending the first
    /update request and sending an /exiting message when the DSSIUIClient
    object is destroyed. It also has a function that will handle all the
    nastyness involved in setting up a shared memory segment that the plugin
    and the UI can use to communicate messages that are not specified by DSSI.
*/
class DSSIUIClient {
public:
  
  /** This constructor starts an OSC receiver thread and sends an update
      request to the plugin host. The parameters should be the @c argc and
      @c argv from the main() function. */
  DSSIUIClient(int argc, char** argv, bool wait = false);
  /** This destructor will mark any allocated shared memory for destruction,
      send an /exiting message to the plugin host, and stop and deallocate
      the OSC receiver thread. */
  ~DSSIUIClient();
  
  /** Returns @c true if the initialisation went OK and we haven't received
      a /quit message yet. */
  bool is_valid() const;
  /** Returns the identifier string given by the plugin host. */
  const string& get_identifier() const;
  /** Searches the given Glade XML tree for DSSI control widgets and 
      connects those widgets to the corresponding DSSI controls. */
  void connect_gui(RefPtr<Xml> xml);
  /** Connects a Gtk::Adjustment to a DSSI port. */
  void connect_adjustment(Adjustment* adj, int port);
  
  // C++ wrappers for OSC methods specified in the DSSI RFC:
  // UI to host
  /** Sends the control value @c value to input port number @c port in the 
      plugin. */
  void send_control(int port, float value);
  /** Change the plugin's program. */
  void send_program(int bank, int program);
  /** Tell the host that we want an update of all the controls, program and
      configuration values for the plugin. This is called automatically
      when this DSSIUIClient object is created. */
  void send_update_request();
  /** Send a configuration value to the plugin. */
  void send_configure(const string& key, const string& value);
  /** Send a MIDI event to the plugin. The effect will be exactly the same
      as if it had been sent by the plugin host. */
  void send_midi(const char event[4]);
  /** Tell the plugin host that the GUI is about to quit (you shouldn't have
      to call this explicitly, it is called when this DSSIUIClient object
      is destroyed. */
  void send_exiting();
  
  // Host to UI
  /** This signal is emitted when the host sends a new control value.
      The parameters are the control port number and the new control value. */
  signal<void, int, float> control_received;
  /** Emitted when the host sends a program change. The parameters are the 
      bank and program numbers. */
  signal<void, int, int> program_received;
  /** Emitted when the host sends a configuration value. The parameters are
      the configuration key and the configuration value. */
  signal<void, const string, const string> configure_received;
  /** Emitted when the host wants the UI to be visible. A DSSI GUI should not
      show any windows until this signal is emitted. */
  Dispatcher show_received;
  /** Emitted when the host wants to hide the UI. */
  Dispatcher hide_received;
  /** Emitted when the host wants the UI to exit. This DSSIUIClient object
      will not send or receive any OSC messages after it has received this
      message, but you still have to quit the program yourself. */
  Dispatcher quit_received;
  
  /** This function allocates a segment of shared memory and tells the plugin
      about it using a configure() call. The plugin should use dssi_shm_attach()
      to attach to the shared memory segment - this function handles all the 
      nasty problems that can arise if the host decides to save the configure()
      message and send it again later, or send it to another instance of the
      plugin. This function can only be used once during the lifetime of this
      DSSIUIClient object, if you try to use it a second time it will simply
      return NULL. If you need to allocate more memory segments after the first
      one you can use the normal shm functions (see the man page for 
      shm_get(2)) and send the segment IDs directly to the plugin using a 
      ringbuffer in the initial shared segment (for example).
      The memory segment allocated using this function will be marked for
      deallocation when this DSSIUIClient object is destroyed. The segment won't
      actually be deallocated until the plugin detaches from it.
  */
  void* allocate_shared_memory(int bytes);
  
  /** Emitted when the plugin has attached to the shared memory segment. */
  signal<void> plugin_attached;
  
  /** Returns true if the plugin has attached to the shared memory segment. */
  bool plugin_has_attached();
  
private:
  
  // Adjustment connection handling (for host->UI messages)
  void update_adjustments(int port, float value);
  
  // Static callbacks for OSC method calls (liblo is not C++)
  static int control_handler(const char *path, const char *types,
			     lo_arg **argv, int argc, 
			     void *data, void *user_data);
  static int program_handler(const char *path, const char *types,
			     lo_arg **argv, int argc, 
			     void *data, void *user_data);
  static int configure_handler(const char *path, const char *types,
			       lo_arg **argv, int argc, 
			       void *data, void *user_data);
  static int show_handler(const char *path, const char *types,
			  lo_arg **argv, int argc, 
			  void *data, void *user_data);
  static int hide_handler(const char *path, const char *types,
			  lo_arg **argv, int argc, 
			  void *data, void *user_data);
  static int quit_handler(const char *path, const char *types,
			  lo_arg **argv, int argc, 
			  void *data, void *user_data);
  
  // Dispatchers that get the signals from the OSC thread to the GUI thread
  // (the queues and receiver functions are needed for passing data since 
  // Dispatchers don't take parameters)
  Dispatcher m_control_dispatcher;
  queue<pair<int, float> > m_control_queue;
  void control_receiver() {
    int port = m_control_queue.front().first;
    float value = m_control_queue.front().second;
    m_control_queue.pop();
    if (unsigned(port) < m_adjustments.size() && m_adjustments[port] != NULL)
      m_adjustments[port]->set_value(value);
    control_received(port, value);
  }
  Dispatcher m_program_dispatcher;
  queue<pair<int, int> > m_program_queue;
  void program_receiver() {
    /* The plugin should update all it's control widgets when it receives
       a program change, but it should NOT send all those control changes
       back to the host. We block all /control and /program messages
       while the UI is handling the program change. */
    m_blocking = true;
    int bank = m_program_queue.front().first;
    int program = m_program_queue.front().second;
    m_program_queue.pop();
    program_received(bank, program);
    m_blocking = false;
  }
  Dispatcher m_configure_dispatcher;
  queue<pair<string, string> > m_configure_queue;
  void configure_receiver() {
    string key = m_configure_queue.front().first;
    string value = m_configure_queue.front().second;
    m_configure_queue.pop();
    configure_received(key, value);
  }
  
  bool check_shared_memory();
  
  lo_address m_plugin_address;
  string m_plugin_path;
  lo_server_thread m_server_thread;
  string m_shm_key;
  char* m_plugin_flag;
  
  bool m_valid;
  string m_identifier;
  
  vector<Adjustment*> m_adjustments;
  bool m_blocking;
};


#endif

