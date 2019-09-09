#ifndef SINESHAPERGUI_HPP
#define SINESHAPERGUI_HPP

#include <libglademm.h>
#include <gtkmm.h>

#include "presetmanager.hpp"
#include "dssiuiclient.hpp"
#include "skindial_gtkmm.hpp"


using namespace std;
using namespace Gdk;
using namespace Gtk;
using namespace Gnome::Glade;
using namespace Glib;


class SineShaperGUI : public Gtk::Window {
public:
  
  /** This constructor is needed so we can 'take over' the window built
      in Glade. */
  SineShaperGUI(BaseObjectType* cobject, const RefPtr<Xml>& refGlade);
  
  /** Connect all knobs and spinbuttons in the GUI to their DSSI control
      ports. */
  void connect_all(DSSIUIClient& dssi);
  
  /** This should get called when the DSSI host has changed the program. */
  void program_selected(int bank, int program);
  
  /** This signal is emitted when the user selects a program in the GUI. */
  sigc::signal<void, unsigned long, unsigned long> signal_select_program;
  
  /** This signal is emitted when the user has edited the programs and 
      the plugin needs to reload the file with the user presets. */
  sigc::signal<void> signal_programs_changed;
  
protected:
  
  /** Create a SkinDial widget and put it in the box with the name @c name. */
  SkinDial* create_knob(const string& name, const string& tooltip,
			double min, double max, double val, 
			SkinDial::Mapping mapping = SkinDial::Linear,
			double center = 0.5);
  
  /** Call this when the DSSI host has changed a control port value. */
  void control_slot(int port, float value);
  
  /** Convenience function to connect and return an Adjustment for a 
      CheckButton widget. */
  Adjustment* get_adjustment(CheckButton* cb);
  
  /** This is called when the user clicks the "Save preset" button. */
  void save_preset();
  
  /** This is called when the user selects a preset from the list of 
      factory presets. */
  void factory_preset_selected();
  /** This is called when the user selects a preset from the list of
      user presets. */
  void user_preset_selected();
  /** This is called when the value of a knob changes. */
  void knob_changed();
  
  /** Return a casted widget from the Glade tree. */
  template <class T> T* w(const string& name) {
    return dynamic_cast<T*>(m_xml->get_widget(name));
  }
  
  RefPtr<Xml> m_xml;
  
  SkinDial* m_tune_knob;
  SpinButton* m_oct_spin;
  SkinDial* m_sub_tune_knob;
  SpinButton* m_sub_oct_spin;
  SkinDial* m_osc_mix_knob;
  
  CheckButton* m_porta_check;
  SkinDial* m_porta_time_knob;
  SkinDial* m_vibra_freq_knob;
  SkinDial* m_vibra_depth_knob;
  SkinDial* m_trem_freq_knob;
  SkinDial* m_trem_depth_knob;
  
  SkinDial* m_shp_env_knob;
  SkinDial* m_shp_total_knob;
  SkinDial* m_shp_split_knob;
  SkinDial* m_shp_shift_knob;
  SkinDial* m_shp_lfo_freq_knob;
  SkinDial* m_shp_lfo_depth_knob;
  
  SkinDial* m_att_knob;
  SkinDial* m_dec_knob;
  SkinDial* m_sus_knob;
  SkinDial* m_rel_knob;

  SkinDial* m_amp_env_knob;
  SkinDial* m_gain_knob;

  SkinDial* m_delay_time_knob;
  SkinDial* m_delay_fb_knob;
  SkinDial* m_delay_mix_knob;

  CheckButton* m_tie_check;
  
  vector<Adjustment*> m_adjs;
  
  /** TreeModel column record for the preset lists. */
  class PresetColumns : public TreeModel::ColumnRecord {
  public:
    PresetColumns(){ 
      add(col_number);
      add(col_index);
      add(col_name); 
    }
    TreeModelColumn<unsigned long> col_number;
    TreeModelColumn<unsigned long> col_index;
    TreeModelColumn<string> col_name;
  } m_preset_columns;

  RefPtr<ListStore> m_factory_preset_model;
  RefPtr<ListStore> m_user_preset_model;
  TreeView* m_factory_preset_list;
  TreeView* m_user_preset_list;
  
  PresetManager m_pm;
  
  Tooltips m_tips;
  
  Dialog* m_save_dlg;
  Dialog* m_save_warning_dlg;
  Entry* m_save_name;
  SpinButton* m_save_number;
  
  bool m_setting_program;
};


#endif
