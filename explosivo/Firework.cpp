#include "Firework.h"
#include "config.h"
#include "intel_rand.h"
#include <math.h>

Firework::Firework(int cx, int cy, int maxw, int maxh, BRUSH_COLOR base_color, BRUSH_SHAPE base_shape, int opacity, int max_radius) : 
  m_cx(cx), m_cy(cy), m_ww(maxw), m_wh(maxh), m_base_color(base_color), m_base_shape(base_shape), m_opacity(opacity), m_max_radius(max_radius)
{
  reset();
}

Firework::~Firework()
{
  free_particles();
}

void Firework::reset()
{
  m_source_radius = m_max_radius * PI;
  m_age = MAX_FIREWORK_AGE;
  double step = (double)360.0 / (double)MAX_PARTICLES;
  BRUSH_COLOR base_color = m_base_color;
  m_base_shape = m_base_shape == BRUSH_SHAPE::RANDOM_BRUSH ? (BRUSH_SHAPE)(genrand64_int64() % Firework::total_shapes() - 1) : m_base_shape;

#if 1
  bool reused = m_particles.GetSize() > 0;
  int j = 0;

  for (double i = 0.0; i <= 360.0; i += step)
  {
    if ((!reused ? m_particles.GetSize() : j) >= MAX_PARTICLES)
      break;

    double deginrad = i * PI / 180.0;
    int x = cos(deginrad) * m_source_radius;
    int y = sin(deginrad) * m_source_radius;
    Particle *p = NULL;

    if (!reused)
    {
      p = new Particle(m_cx + x, m_cy + y, base_color, m_opacity, m_max_radius);

      if (!p)
        continue;

      p->m_parent = (void *)this;
      m_particles.Insert(0, p);
    }
    else
    {
      p = m_particles.Get(j++);

      if (!p)
        continue;

      p->m_cx = m_cx + x;
      p->m_cy = m_cy + y;
      p->m_base_color = base_color;
      p->m_opacity = m_opacity;
      p->reset();
    }
    
  }
#else
  double a = 1.15, b = 0.15;
  double pidiv = (PI / 720.0f);

  for (int i = 0; i < MAX_PARTICLES; i++)
  {
    double ang = pidiv * i;
    int x = m_cx + (a * cos(ang) * (pow(b * ang, 2.718281828)));
    int y = m_cy + (a * sin(ang) * (pow(b * ang, 2.718281828)));
    Particle *p = NULL;
    p = new Particle(x, y, base_color);
    m_particles.Insert(0, p);
  }
#endif
}

void Firework::tick()
{
  if (m_age-- > MAX_FIREWORK_AGE - EXPLODE_AFTER)
    return;

  if (MAX_FIREWORK_AGE > 0 && m_age == 0)
    reset();

  bool went_offscreen = false;

  for (int i = 0; i < m_particles.GetSize(); i++)
  {
    Particle *p = m_particles.Get(i);

    if (m_age % p->m_move_after == 0)
    {
      p->change_direction();
      //p->change_size();
    }

    p->tick();

    if ((p->m_cx < 0 || p->m_cx >= m_ww) ||
        (p->m_cy < 0 || p->m_cy >= m_wh))
    {
      went_offscreen = true;
    }
  }

  if (went_offscreen)
    reset();
}

void Firework::free_particles()
{
  m_particles.Empty(true);
}

int Firework::total_colors()
{
  return(9);
}

int Firework::total_shapes()
{
  return(19);
}

void Firework::set_max_radius(int max_radius)
{
  m_max_radius = max_radius;

  for (int i = 0; i < m_particles.GetSize(); i++)
  {
    Particle *p = m_particles.Get(i);
    p->m_max_radius = m_max_radius;
    p->change_size();
  }
}

int Firework::get_max_radius()
{
  return(m_max_radius);
}

