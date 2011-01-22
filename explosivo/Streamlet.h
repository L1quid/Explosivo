#ifndef _DEMOS_STREAMLET_H
#define _DEMOS_STREAMLET_H

#include <windows.h>
#include "../wdl/ptrlist.h"
#include "config.h"

class Streamlet
{
public:
  Streamlet();
  ~Streamlet();

  void reset();
  void tick();
  void activate();
  void deactivate();
  int get_x() { return(m_cx); }
  int get_y() { return(m_cy); }
  void get_pos(POINT *pt) { pt->x = m_cx; pt->y = m_cy; }
  int get_width() { return(m_width); }
  int get_height() { return(m_height); }
  void get_color(RGBQUAD *rgb) { rgb->rgbBlue = m_b; rgb->rgbGreen = m_g; rgb->rgbRed = m_r; }
  double get_sin_val();
  int get_fluxitude() { return(m_rand_fluxitude); }

  static void init_tables();
  static void deinit_tables();

protected:
  unsigned int m_color;
  char m_r, m_g, m_b;
  double m_age;
  int m_cx, m_cy, m_width, m_height;
  double m_amplitude;
  int m_amplitude_rand, m_rand_fluxitude;
  bool m_active;
  
};

#endif // _DEMOS_STREAMLET_H