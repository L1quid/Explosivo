#include <math.h>
#include <stdlib.h>
#include "Streamlet.h"
#include "config.h"
#include "explosivo.h"

extern unsigned int HEIGHT, WIDTH;

WDL_PtrList<double> g_tables;
#define table_size WIDTH
const COLORREF g_colors[] = {
  RGB(58,84,58),
  RGB(79,114,79),
  RGB(100,144,100),
  RGB(144,213,144),
  RGB(181,227,181),
  RGB(237,248,237),
  RGB(58,71,84),
  RGB(79,96,114),
  RGB(77,102,128),
  RGB(65,79,93),
  RGB(110,135,162),
  RGB(140,158,176),
  RGB(188,192,197),
};

Streamlet::Streamlet()
{
  reset();
}

Streamlet::~Streamlet()
{
  m_active = false;
}

void Streamlet::reset()
{
  m_amplitude_rand = (rand() % AMP_HEIGHT) + 1;
  m_amplitude = ((double)m_amplitude_rand) * AMP_MOD;
  m_width = (rand() % AMP_HEIGHT) + 1;
  m_height = (rand() % AMP_HEIGHT) + 1;
  m_active = false;
  m_age = 0.0;
  int len = sizeof(g_colors) / sizeof(COLORREF);
  COLORREF c = g_colors[rand() % len];
  m_r = (char)((c >> 16) & 0xff);
  m_g = (char)((c >> 8) & 0xff);
  m_b = (char)((c) & 0xff);
  m_rand_fluxitude = (rand() % 20) + 1;//(AMP_HEIGHT / 4);

  //if (rand() % 2 == 0)
   // m_rand_fluxitude = -m_rand_fluxitude;
}

void Streamlet::tick()
{
  if (m_active)
  {
    double val = m_amplitude * sin(PI * (m_age * AMP_MOD) + THETA); //atan(((int)m_age % (HEIGHT / 8)) * 0.01); //

    if (m_rand_fluxitude % 5 == 0)
      val = -val;

    double px = (1.0f / AGE_INCREMENT(WIDTH));
    m_cx = (int)(m_age * px);
    m_cy = (int)(val * (double)HEIGHT);
  }

  m_age += AGE_INCREMENT(WIDTH);
}

void Streamlet::activate()
{
  m_active = true;
}

void Streamlet::deactivate()
{
  m_active = false;
  reset();
}

double Streamlet::get_sin_val()
{
  double *t = g_tables.Get(m_amplitude_rand);

  if (!t)
    return(0.0);

  int index = ((int)m_age) % table_size;
  double ret = t[index];

  return(ret);
}

void Streamlet::init_tables()
{
  int i;

  for (i = 0; i <= AMP_HEIGHT; i++)
  {
    double amplitude = (double)i * AMP_MOD;

    double *t = NULL;
    t = new double[table_size];
    g_tables.Add(t);
    int j = 0;
    
    for (j = 0; j < (int)(table_size); j++)
    {
      t[j] = amplitude * sin(PI * ((double)j * AMP_MOD) + THETA);
    }
  }
}

void Streamlet::deinit_tables()
{
  int i;

  for (i = 0; i < AMP_HEIGHT; i++)
  {
    double *t = NULL;
    t = g_tables.Get(i);
    delete t;
  }
}