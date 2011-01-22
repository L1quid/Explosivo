#ifndef _DEMOS_EMITTER_H
#define _DEMOS_EMITTER_H

#include "Streamlet.h"
#include "../../wdl/ptrlist.h"

class Emitter
{
public:
  Emitter();
  virtual ~Emitter();

  void reset();
  void init_stream();
  Streamlet *create_streamlet();
  void tick();
  bool emit();
  void set_window(int w, int h);
  WDL_PtrList<Streamlet> *get_streamlets();
  void sort_streamlets();
  void set_offset(int x, int y) { m_x_offset = x; m_y_offset = y; }
  int get_x_offset() const { return(m_x_offset); }
  int get_y_offset() const { return(m_y_offset); }
  double get_age() const { return(m_age); }

protected:
  double m_age;
  int m_cx, m_cy, m_win_width, m_win_height, m_x_offset, m_y_offset;
  WDL_PtrList<Streamlet> m_streamlets, m_active_streamlets, m_inactive_streamlets, m_sorted_streamlets;
};

#endif // _DEMOS_EMITTER_H