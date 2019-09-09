/****************************************************************************
    
    skindial_gtkmm.hpp - A skinnable knob widget for gtkmm
    
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

#ifndef SKINDIAL_HPP
#define SKINDIAL_HPP

#include <gtkmm.h>
#include <libglademm.h>

using namespace Gtk;
using namespace Gdk;
using namespace sigc;
using namespace std;
using namespace Glib;
using namespace Gnome::Glade;

/**
   SkinDial is a knob widget that uses a sequence of user supplied pixmaps 
   to provide eye candy knobs (dials). The pixmap must contain all the dial
   positions layed out horizontally. The dial positions sequence must be from
   the minimum value to the maximum value. We suggest to supply at least 50
   positions to make the movement of the dial smooth.
   \n
   For example if the knob has a dimension of 40x40 pixels and 50 positions, 
   the resulting pixmap must be (40*50) x 50 pixels  =  2000 x 50 pixels
   \n
   The supplied pixmap can have an alpha channel which allows for smooth 
   blending with the background.
*/   
class SkinDial : public DrawingArea {
public:
  
  enum Mapping {
    Linear,
    Logarithmic,
    DoubleLog
  };
  
  /** Create a new SkinDial with the adjustment @c adj and the pixmap in @c pm.
      @c num_positions give the number of dial images in @c pm. If it is -1
      it will be assumed that the images have the same width as height, and
      the number of different images will be calculated automatically. */
  SkinDial(Adjustment& adj, RefPtr<Pixbuf> pm, 
	   Mapping = Linear, double center = 0.5, int num_position = -1);
  /** Same as the other constructor, except this one creates an @c Adjustment 
      automatically with the given @c min and @c max values. */
  SkinDial(double min, double max, RefPtr<Pixbuf> pm, 
	   Mapping = Linear, double center = 0.5, int num_position = -1);
  /** This constructor is needed for get_widget_derived() in libglademm. */
  SkinDial(BaseObjectType* cobject, const RefPtr<Xml>& refGlade);
  
  /** Returns the @c Gtk::Adjustment of this dial. */
  Adjustment* get_adjustment();
  /** Set the @c Gtk::Adjustment used in this dial. */
  void set_adjustment(Adjustment& adj);
  
protected:

  void on_realize();
  bool on_expose_event(GdkEventExpose* event);
  bool on_button_press_event(GdkEventButton* event);
  bool on_button_release_event(GdkEventButton* event);
  bool on_motion_notify_event(GdkEventMotion* event);
  
  bool key_pressed_in_popup(GdkEventKey* event);
  
  void init(Adjustment* adj, RefPtr<Pixbuf> pm, 
	    Mapping mapping, double center, int num_position);
  
  double map_value(double value);
  double unmap_value(double value);
  
  static double log_map(double value, double min, double max, double k);
  static double exp_map(double value, double min, double max, double k);
  
  RefPtr<GC> m_gc;
  RefPtr<Gdk::Window> m_win;
  RefPtr<Pixbuf> m_pixbuf;
  int m_num_pos;
  int m_dial_width;
  int m_drag_x;
  int m_drag_y;
  bool m_dragging;
  double m_drag_value;
  Adjustment* m_adj;
  Mapping m_mapping;
  double m_unmapped;
  double m_center;
  Gtk::Window m_popup;
  SpinButton m_spin;
};


#endif
