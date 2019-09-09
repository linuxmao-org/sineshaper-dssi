#include <cmath>
#include <iostream>
#include <fstream>
#include <map>

#include "presetmanager.hpp"
#include "sineshapergui.hpp"
#include "sineshaperports.hpp"

using namespace std;


SineShaperGUI::SineShaperGUI(BaseObjectType* cobject, 
			     const RefPtr<Xml>& refGlade)
  : Gtk::Window(cobject), m_xml(refGlade), m_setting_program(false) {
  
  // load the presets - factory presets in bank 0, user presets in bank 1
  m_pm.load_bank(INSTALL_DIR "/sineshaper/presets", INPUT_PORTS);
  m_pm.load_bank(string(getenv("HOME")) + "/.sineshaperpresets", INPUT_PORTS);
  
  /* XXX This is all horrible and should be replaced by some code-fu in 
     DSSIUIClient and by inheriting SkinDial from Gtk::Range */

  m_adjs.resize(INPUT_PORTS);
  
  // the Tuning controls
  m_tune_knob = create_knob("tune_box", "Tune the whole synth",
			    0.5, 2, 1, SkinDial::DoubleLog, 1);
  m_adjs[TUN] = m_tune_knob->get_adjustment();
  m_oct_spin = w<SpinButton>("oct_spin");
  m_tips.set_tip(*m_oct_spin, "Shift the synth up or down by whole octaves");
  m_adjs[OCT] = m_oct_spin->get_adjustment();
  
  // the Oscillator 2 controls
  m_sub_tune_knob = create_knob("sub_tune_box", 
				"Tune the second oscillator", 0.5, 2, 1, 
				SkinDial::DoubleLog, 1);
  m_adjs[SUB_TUN] = m_sub_tune_knob->get_adjustment();
  m_sub_oct_spin = w<SpinButton>("sub_oct_spin");
  m_tips.set_tip(*m_sub_oct_spin, 
		 "Shift the second oscillator up or down by whole octaves");
  m_adjs[SUB_OCT] = m_sub_oct_spin->get_adjustment();
  m_osc_mix_knob = create_knob("osc_mix_box", 
			       "Set the relative volume of the oscillators",
			       0, 1, 0.5);
  m_adjs[OSC_MIX] = m_osc_mix_knob->get_adjustment();
  
  // the Portamento controls
  m_porta_check = w<CheckButton>("porta_check");
  m_tips.set_tip(*m_porta_check, "Turn portamento on");
  m_adjs[PRT_ON] = get_adjustment(m_porta_check);
  m_tie_check = w<CheckButton>("tie_check");
  m_tips.set_tip(*m_tie_check, "Slide between overlapping notes");
  m_adjs[PRT_TIE] = get_adjustment(m_tie_check);
  m_porta_time_knob = create_knob("porta_time_box", "Set the portamento time",
				  0.001, 1, 0.4999, SkinDial::Logarithmic);
  m_adjs[PRT_TIM] = m_porta_time_knob->get_adjustment();
  
  // the Vibrato controls
  m_vibra_freq_knob = create_knob("vibra_freq_box", "Set the vibrato frequency",
				  0, 10, 0);
  m_adjs[VIB_FRQ] = m_vibra_freq_knob->get_adjustment();
  m_vibra_depth_knob = create_knob("vibra_depth_box", "Set the vibrato depth",
				   0, 0.25, 0);
  m_adjs[VIB_DPT] = m_vibra_depth_knob->get_adjustment();
  
  // the Tremolo controls
  m_trem_freq_knob = create_knob("trem_freq_box", "Set the tremolo frequency",
				 0, 10, 0);
  m_adjs[TRM_FRQ] = m_trem_freq_knob->get_adjustment();
  m_trem_depth_knob = create_knob("trem_depth_box", "Set the tremolo depth",
				  0, 1, 0);
  m_adjs[TRM_DPT] = m_trem_depth_knob->get_adjustment();
  
  // the Shaper controls
  m_shp_env_knob = create_knob("shp_env_box", 
			       "Set the envelope sensitivity for the shaper",
			       0, 1, 1);
  m_adjs[SHP_ENV] = m_shp_env_knob->get_adjustment();
  m_shp_total_knob = create_knob("shp_total_box", 
				 "Set the total amount of shaping", 0, 6, 0.19);
  m_adjs[SHP_TOT] = m_shp_total_knob->get_adjustment();
  m_shp_split_knob = create_knob("shp_split_box", 
				 "Set the relative shaping amount for the "
				 "two waveshapers", 0, 1, 0.93);
  m_adjs[SHP_SPL] = m_shp_split_knob->get_adjustment();
  m_shp_shift_knob = create_knob("shp_shift_box", 
				 "Set the shift of the waveshaping function "
				 "(0 = sine, 1 = cosine)", 0, 1, 0);
  m_adjs[SHP_SHF] = m_shp_shift_knob->get_adjustment();
  m_shp_lfo_freq_knob = create_knob("shp_lfo_freq_box", 
				    "Set the shaper LFO frequency",
				    0, 10, 0.4999);
  m_adjs[LFO_FRQ] = m_shp_lfo_freq_knob->get_adjustment();
  m_shp_lfo_depth_knob = create_knob("shp_lfo_depth_box", 
				     "Set the shaper LFO depth", 0, 1, 0);
  m_adjs[LFO_DPT] = m_shp_lfo_depth_knob->get_adjustment();
  
  // the Envelope controls
  m_att_knob = create_knob("att_box", "Set the envelope attack time", 
			   0.0005, 1, 0.03, SkinDial::Logarithmic);
  m_adjs[ATT] = m_att_knob->get_adjustment();
  m_dec_knob = create_knob("dec_box", "Set the envelope decay time", 
			   0.0005, 1, 0.3, SkinDial::Logarithmic);
  m_adjs[DEC] = m_dec_knob->get_adjustment();
  m_sus_knob = create_knob("sus_box", "Set the envelope sustain level",
			   0, 1, 0.6);
  m_adjs[SUS] = m_sus_knob->get_adjustment();
  m_rel_knob = create_knob("rel_box", "Set the envelope release time", 
			   0.0005, 3, 0.5, SkinDial::Logarithmic);
  m_adjs[REL] = m_rel_knob->get_adjustment();
  
  // the Amp controls
  m_amp_env_knob = create_knob("amp_env_box", 
			       "Set the envelope sensitivity for the amplifier",
			       0, 1, 0);
  m_adjs[AMP_ENV] = m_amp_env_knob->get_adjustment();
  m_gain_knob = create_knob("drive_box", "Set the overdrive level", 0, 1, 0);
  m_adjs[DRIVE] = m_gain_knob->get_adjustment();
  m_gain_knob = create_knob("gain_box", "Set the total gain level", 0, 2, 1);
  m_adjs[GAIN] = m_gain_knob->get_adjustment();
  
  // the Delay controls
  m_delay_time_knob = create_knob("delay_time_box", "Set the delay time",
				  0, 4, 1, SkinDial::Logarithmic);
  m_adjs[DEL_TIM] = m_delay_time_knob->get_adjustment();
  m_delay_fb_knob = create_knob("delay_fb_box", "Set the feedback level",
				0, 1, 0.3);
  m_adjs[DEL_FB] = m_delay_fb_knob->get_adjustment();
  m_delay_mix_knob = create_knob("delay_mix_box", "Set the delay mix amount",
				 0, 1, 0.5);
  m_adjs[DEL_MIX] = m_delay_mix_knob->get_adjustment();
    
  // the factory preset list view
  m_factory_preset_list = w<TreeView>("factory_preset_list");
  m_factory_preset_model = ListStore::create(m_preset_columns);
  m_factory_preset_list->set_model(m_factory_preset_model);
  m_factory_preset_list->append_column("No", m_preset_columns.col_number);
  m_factory_preset_list->append_column("Name", m_preset_columns.col_name);
  m_factory_preset_list->get_selection()->signal_changed().
    connect(mem_fun(*this, &SineShaperGUI::factory_preset_selected));
  for (unsigned i = 0; i < m_pm.get_bank(0).size(); ++i) {
    ListStore::iterator iter = m_factory_preset_model->append();
    (*iter)[m_preset_columns.col_number] = m_pm.get_bank(0)[i].number;
    (*iter)[m_preset_columns.col_index] = i;
    (*iter)[m_preset_columns.col_name] = m_pm.get_bank(0)[i].name;
  }
  
  // the user preset list view
  m_user_preset_list = w<TreeView>("user_preset_list");
  m_user_preset_model = ListStore::create(m_preset_columns);
  m_user_preset_list->set_model(m_user_preset_model);
  m_user_preset_list->append_column("No", m_preset_columns.col_number);
  m_user_preset_list->append_column("Name", m_preset_columns.col_name);
  m_user_preset_list->get_selection()->signal_changed().
    connect(mem_fun(*this, &SineShaperGUI::user_preset_selected));
  for (unsigned i = 0; i < m_pm.get_bank(1).size(); ++i) {
    ListStore::iterator iter = m_user_preset_model->append();
    (*iter)[m_preset_columns.col_number] = m_pm.get_bank(1)[i].number;
    (*iter)[m_preset_columns.col_index] = i;
    (*iter)[m_preset_columns.col_name] = m_pm.get_bank(1)[i].name;
  }
  
  // the save button and dialogs
  Button* save_button = w<Button>("save_button");
  save_button->signal_clicked().
    connect(mem_fun(*this, &SineShaperGUI::save_preset));
  m_save_dlg = w<Dialog>("dlg_save");
  m_save_warning_dlg = w<Dialog>("dlg_save_warning");
  m_save_name = w<Entry>("dlg_save_name");
  m_save_number = w<SpinButton>("dlg_save_number");
  
  // turn on tooltips 
  // (maybe we need a checkbutton for this if it's too annoying)
  m_tips.enable();
  
  // connect the about dialog
  Button* about_button = w<Button>("about_button");
  AboutDialog* dlg_about = w<AboutDialog>("dlg_about");
  dlg_about->set_version(VERSION);
  dlg_about->set_modal(true);
  dlg_about->set_transient_for(*this);
  about_button->signal_clicked().
    connect(hide_return(mem_fun(*dlg_about, &Dialog::show)));
}


/* As I said, horrible. */
void SineShaperGUI::connect_all(DSSIUIClient& dssi) {
  dssi.control_received.connect(mem_fun(*this, &SineShaperGUI::control_slot));
  for (unsigned i = 0; i < m_adjs.size(); ++i) {
    dssi.connect_adjustment(m_adjs[i], i);
    m_adjs[i]->signal_value_changed().
      connect(mem_fun(*this, &SineShaperGUI::knob_changed));
  }
}


void SineShaperGUI::program_selected(int bank, int program) {
  m_setting_program = true;
  
  // find the program and update all knobs
  if (bank == 0) {
    ListStore::const_iterator iter = m_factory_preset_model->children().begin();
    for ( ; iter != m_factory_preset_model->children().end(); ++iter) {
      if ((*iter)[m_preset_columns.col_number] == (unsigned long)program) {
	m_factory_preset_list->get_selection()->select(iter);
	int index = (*iter)[m_preset_columns.col_index];
	for (unsigned i = 0; i < INPUT_PORTS; ++i)
	  m_adjs[i]->set_value(m_pm.get_bank(bank)[index].values[i]);
	break;
      }
    }
  }
  else if (bank == 1) {
    ListStore::const_iterator iter = m_user_preset_model->children().begin();
    for ( ; iter != m_user_preset_model->children().end(); ++iter) {
      if ((*iter)[m_preset_columns.col_number] == (unsigned long)program) {
	m_user_preset_list->get_selection()->select(iter);
	int index = (*iter)[m_preset_columns.col_index];
	for (unsigned i = 0; i < INPUT_PORTS; ++i)
	  m_adjs[i]->set_value(m_pm.get_bank(bank)[index].values[i]);
	break;
      }
    }
  }
  
  m_setting_program = false;
}


SkinDial* SineShaperGUI::create_knob(const string& name, const string& tooltip,
				     double min, double max, double val, 
				     SkinDial::Mapping mapping, double center) {
  Box* box = w<Box>(name);
  if (!box) {
    cerr<<"The box '"<<name<<"' was not found!"<<endl;
    return NULL;
  }
  RefPtr<Pixbuf> pixbuf = 
    Pixbuf::create_from_file(INSTALL_DIR "/sineshaper/dial.png");
  SkinDial* knob = manage(new SkinDial(min, max, pixbuf, mapping, center));
  box->children().clear();
  box->pack_start(*knob, PACK_EXPAND_PADDING);
  knob->get_adjustment()->set_value(val);
  m_tips.set_tip(*knob, tooltip);
  knob->show();
  return knob;
}


void SineShaperGUI::control_slot(int port, float value) {
  if ((unsigned)port >= 0 && (unsigned)port < m_adjs.size() && m_adjs[port]) {
    if (m_adjs[port]->get_value() != value)
      m_adjs[port]->set_value(value);
  }
}


static bool double2bool(double v) {
  return v > 0;
}


Adjustment* SineShaperGUI::get_adjustment(CheckButton* cb) {
  Adjustment* adj = manage(new Adjustment(-1, -1, 1));
  cb->signal_toggled().
    connect(compose(mem_fun(*adj, &Adjustment::set_value),
		    mem_fun(*cb, &CheckButton::get_active)));
  adj->signal_value_changed().
    connect(compose(mem_fun(*cb, &CheckButton::set_active),
		    compose(&double2bool, 
			    mem_fun(*adj, &Adjustment::get_value))));
  return adj;
}


void SineShaperGUI::save_preset() {
  m_save_name->set_text("");
  
  // suggest an empty preset number
  unsigned long first_free = 0;
  for (unsigned i = 0; i < m_pm.get_bank(1).size(); ++i) {
    if (m_pm.get_bank(1)[i].number >= first_free)
      first_free = m_pm.get_bank(1)[i].number + 1;
  }
  m_save_number->set_value(first_free);
  
  // run the dialog
  while (m_save_dlg->run() == RESPONSE_OK) {
    
    // check if the preset number is alread used
    bool ok = true;
    for (unsigned i = 0; i < m_pm.get_bank(1).size(); ++i) {
      if (m_pm.get_bank(1)[i].number == m_save_number->get_value()) {
	ok = false;
	break;
      }
    }
    
    // if it is not used, or if the user wants to overwrite the old preset, go
    if (ok || m_save_warning_dlg->run() == RESPONSE_YES) {
      vector<double> values;
      for (unsigned i = 0; i < m_adjs.size(); ++i)
	values.push_back(m_adjs[i]->get_value());
      m_pm.add_preset(1, int(m_save_number->get_value()), 
		      m_save_name->get_text(), values, true);
      m_user_preset_model->clear();
      for (unsigned i = 0; i < m_pm.get_bank(1).size(); ++i) {
	ListStore::iterator iter = m_user_preset_model->append();
	(*iter)[m_preset_columns.col_number] = m_pm.get_bank(1)[i].number;
	(*iter)[m_preset_columns.col_index] = i;
	(*iter)[m_preset_columns.col_name] = m_pm.get_bank(1)[i].name;
      }
      m_pm.save_bank(1, string(getenv("HOME")) + "/.sineshaperpresets");
      signal_programs_changed();
      m_save_warning_dlg->hide();
      break;
    }
    m_save_warning_dlg->hide();
  }
  m_save_dlg->hide();
}


void SineShaperGUI::factory_preset_selected() {
  // factory preset is selected, we need to unselect all user presets
  ListStore::const_iterator iter = 
    m_factory_preset_list->get_selection()->get_selected();
  if (iter) {
    signal_select_program(0, (*iter)[m_preset_columns.col_number]);
    m_user_preset_list->get_selection()->unselect_all();
  }
}


void SineShaperGUI::user_preset_selected() {
  // user preset is selected, we need to unselect all factory presets
  ListStore::const_iterator iter = 
    m_user_preset_list->get_selection()->get_selected();
  if (iter) {
    signal_select_program(1, (*iter)[m_preset_columns.col_number]);
    m_factory_preset_list->get_selection()->unselect_all();
  }
}


void SineShaperGUI::knob_changed() {
  // knob changed, we are no longer using a preset
  if (!m_setting_program) {
    m_factory_preset_list->get_selection()->unselect_all();
    m_user_preset_list->get_selection()->unselect_all();
  }
}
