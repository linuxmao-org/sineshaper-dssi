/****************************************************************************
    
    skindial_gtkmm.cpp - A skinnable knob widget for gtkmm
    
    Copyright (C) 2005  Lars Luthman <larsl@users.sourceforge.net>
    Based on the Qt SkinDial widget by Benno Senoner <sbenno@gardena.net>
    
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

#include <cmath>
#include <iostream>

#include "skindial_gtkmm.hpp"


SkinDial::SkinDial(Adjustment& adj, RefPtr<Pixbuf> pm, 
		   Mapping mapping, double center, int num_position)
  : m_popup(WINDOW_POPUP) {
  init(&adj, pm, mapping, center, num_position);
}


SkinDial::SkinDial(double min, double max, 
		   RefPtr<Pixbuf> pm, Mapping mapping, 
		   double center, int num_position) 
  : m_popup(WINDOW_POPUP) {
  init(manage(new Adjustment(min, min, max)), pm, mapping, center,num_position);
}


SkinDial::SkinDial(BaseObjectType* cobject, const RefPtr<Xml>& refGlade) 
  : DrawingArea(cobject) {
  init(manage(new Adjustment(0, 0, 1)), 
       Pixbuf::create_from_file(INSTALL_DIR"/sineshaper/dial.png"), 
       Linear, 0.5, -1);
}


void SkinDial::init(Adjustment* adj, RefPtr<Pixbuf> pm, 
		    Mapping mapping, double center, int num_position) {
  m_adj = adj;
  m_pixbuf = pm;
  m_num_pos = num_position;
  m_dragging = false;
  m_mapping = mapping;
  m_center = center;
  
  int w = pm->get_width();
  int h = pm->get_height();
  if (m_num_pos == -1) {
    m_num_pos = w / h;
    m_dial_width = h;
  }
  else {
    m_dial_width = w / m_num_pos;
  }
  
  set_size_request(m_dial_width, h);
  
  m_adj->signal_value_changed().connect(mem_fun(*this, &SkinDial::queue_draw));
  
  add_events(BUTTON_PRESS_MASK | BUTTON_RELEASE_MASK | BUTTON1_MOTION_MASK |
	     BUTTON_MOTION_MASK| ENTER_NOTIFY_MASK | LEAVE_NOTIFY_MASK);
  set_events(get_events() & ~POINTER_MOTION_HINT_MASK);
  
  m_popup.set_resizable(false);
  m_popup.set_modal(true);
  m_popup.signal_key_press_event().
    connect(mem_fun(*this, &SkinDial::key_pressed_in_popup));
  m_popup.add_events(KEY_PRESS_MASK);
  m_spin.set_adjustment(*adj);
  m_spin.set_numeric(true);
  m_spin.set_digits(5);
  m_spin.set_increments(0.001, 0.1);
  Frame* tmp_frame = manage(new Frame());
  HBox* tmp_box = manage(new HBox());
  m_popup.add(*tmp_frame);
  tmp_frame->add(*tmp_box);
  tmp_box->set_border_width(3);
  tmp_box->add(m_spin);
}


Adjustment* SkinDial::get_adjustment() {
  return m_adj;
}


void SkinDial::set_adjustment(Adjustment& adj) {
  m_adj = &adj;
}


void SkinDial::on_realize() {
  DrawingArea::on_realize();
}


bool SkinDial::on_expose_event(GdkEventExpose* event) {
  if (!m_gc) {
    m_win = get_window();
    m_gc = GC::create(m_win);
  }

  int i = int(unmap_value(m_adj->get_value()) * (m_num_pos - 0.001));
  if (i >= m_num_pos)
    i = m_num_pos - 1;
  
  m_win->draw_pixbuf(m_gc, m_pixbuf, m_dial_width * i, 0, 0, 0, 
		     m_dial_width, m_pixbuf->get_height(),
		     RGB_DITHER_NONE, 0, 0);
  return true;
}


bool SkinDial::on_button_press_event(GdkEventButton* event) {
  if (event->button == 1) {
    m_drag_x = int(event->x);
    m_drag_y = int(event->y);
    m_drag_value = unmap_value(m_adj->get_value());
    m_dragging = true;
  }
  else if (event->button == 3) {
    m_popup.set_position(WIN_POS_MOUSE);
    m_popup.set_focus(m_spin);
    m_popup.show_all();
  }
  return true;
}


bool SkinDial::on_button_release_event(GdkEventButton* event) {
  if (event->button == 1)
    m_dragging = false;
  return true;
}


bool SkinDial::on_motion_notify_event(GdkEventMotion* event) {
  if (m_dragging) {
    int y_diff = int(event->y) - m_drag_y;
    m_unmapped = m_drag_value - y_diff / 200.0;
    m_unmapped = (m_unmapped < 0 ? 0 : (m_unmapped > 1 ? 1 : m_unmapped));
    m_adj->set_value(map_value(m_unmapped));
  }
  return true;
}


bool SkinDial::key_pressed_in_popup(GdkEventKey* event) {
  if (event->keyval == GDK_Escape)
    m_popup.hide();
  return true;
}


double SkinDial::log_map(double value, double min, double max, double k) {
  double y = (value - min) / (max - min);
  return log(1 + y * (exp(k) - 1)) / k;
}


double SkinDial::exp_map(double value, double min, double max, double k) {
  double x = value;
  return min + (max - min) * ((exp(k * x) - 1) / (exp(k) - 1));
}


double SkinDial::map_value(double value) {
  
  double d;
  
  if (m_mapping == Logarithmic) {
    d = exp_map(value, m_adj->get_lower(), m_adj->get_upper(), 5);
  }

  else if (m_mapping == DoubleLog) {
    if (value >= 0.5)
      d =  exp_map(2 * (value - 0.5), m_center, m_adj->get_upper(), 5);
    else
      d =  m_center - exp_map(1 - 2 * value, 0, 
			      m_center - m_adj->get_lower(), 5);
  }  
  
  else
    d = m_adj->get_lower() + (m_adj->get_upper() - m_adj->get_lower()) * value;
  
  return d;
}


double SkinDial::unmap_value(double value) {
  
  double d;
  
  if (m_mapping == Logarithmic) {
    d = log_map(value, m_adj->get_lower(), m_adj->get_upper(), 5);
  }
  
  else if (m_mapping == DoubleLog) {
    if (value >= m_center)
      d = 0.5 + 0.5 * log_map(value, m_center, m_adj->get_upper(), 5);
    else
      d = 0.5 - 0.5 * log_map(m_center - value, 0,
			      m_center - m_adj->get_lower(), 5);
  }
  
  else
    d = value / (m_adj->get_upper() - m_adj->get_lower()) - m_adj->get_lower();

  return d;
}
