#include <windows.h>
#include <math.h>
#include "Emitter.h"
#include "config.h"
#include "explosivo.h"

extern unsigned int WIDTH, HEIGHT;

Emitter::Emitter()
{
  reset();
}

Emitter::~Emitter()
{
  m_streamlets.Empty(true);
}

void Emitter::reset()
{
  m_age = 0.0;
  m_cx = m_cy = 0;
  m_x_offset = m_y_offset = 0;
}

void Emitter::init_stream()
{
  int i;

  for (i = 0; i < STREAMLETS_PER_EMITTER; i++)
  {
    Streamlet *p = NULL;
    p = create_streamlet();

    if (!p)
      break;
  }
}

Streamlet *Emitter::create_streamlet()
{
  Streamlet *p = NULL;
  p = new Streamlet();
  m_streamlets.Add(p);
  m_inactive_streamlets.Add(p);

  return(p);
}

void Emitter::tick()
{
  bool needs_activation = m_inactive_streamlets.GetSize() > 0 && (int)m_age % FRAMES_BETWEEN_STREAMLETS == 0;

  if (needs_activation)
    emit();

  int i, num_streamlets = m_active_streamlets.GetSize();
  Streamlet *p = NULL;
  POINT pos;

  for (i = 0; i < num_streamlets; i++)
  {
    p = m_active_streamlets.Get(i);

    if (!p)
      continue;

    p->tick();
    p->get_pos(&pos);

    if (pos.x >= m_win_width || pos.y >= m_win_height)
    {
      p->deactivate();
      m_active_streamlets.Delete(m_active_streamlets.Find(p));
      m_inactive_streamlets.Add(p);
    }
  }

  m_age += AGE_INCREMENT(WIDTH);
}

bool Emitter::emit()
{
  Streamlet *p = NULL;

  if (m_inactive_streamlets.GetSize() == 0)
  {
    if (m_streamlets.GetSize() < STREAMLETS_PER_EMITTER)
      p = create_streamlet();
  }
  else
    p = m_inactive_streamlets.Get(0);

  if (!p)
    return(false);

  p->activate();
  m_inactive_streamlets.Delete(m_inactive_streamlets.Find(p));
  m_active_streamlets.Add(p);

  return(true);
}

void Emitter::set_window(int w, int h)
{
  m_win_width = w;
  m_win_height = h;
}

WDL_PtrList<Streamlet> *Emitter::get_streamlets()
{
  //sort_streamlets();
  
  return(&m_active_streamlets);
}

int STREAMLET_COMPARE(const void *a, const void *b)
{
  Streamlet *aa = (Streamlet *)a;
  Streamlet *bb = (Streamlet *)b;

  return ((bb->get_width() + bb->get_height()) * 2 - (aa->get_width() + aa->get_height()) * 2);
}

void Emitter::sort_streamlets()
{
  m_sorted_streamlets.Empty();
  qsort(m_active_streamlets.GetList(), m_active_streamlets.GetSize(), sizeof(Streamlet *), STREAMLET_COMPARE);
}