#ifndef _DEMOS_FIREWORK_H
#define _DEMOS_FIREWORK_H

#define MAX_FIREWORK_AGE 0
#define EXPLODE_AFTER (0 * 60)
#define CHANGE_DIRECTION_AFTER (1 * 60)
#define MAX_PARTICLES 12

#ifdef _DEBUG
#define MAX_FIREWORKS 10
#else
#define MAX_FIREWORKS 250
#endif

#include "chipmunk.h"
#include "config.h"
#include "../wdl/ptrlist.h"
#include "intel_rand.h"

extern agg::rgba8 g_kulers[5];

typedef enum BRUSH_SHAPE {
  RANDOM_BRUSH = -1,
  SQUARE,
  DIAMOND,
  CIRCLE,
  CROSSED_CIRCLE,
  SEMI_ELLIPSE_LEFT,
  SEMI_ELLIPSE_RIGHT,
  SEMI_ELLIPSE_UP,
  SEMI_ELLIPSE_DOWN,
  TRIANGLE_LEFT,
  TRIANGLE_RIGHT,
  TRIANGLE_UP,
  TRIANGLE_DOWN,
  FOUR_RAYS,
  CROSS,
  XING,
  DASH,
  DOT,
  PIXEL,
} ;

typedef enum BRUSH_COLOR {
  RANDOM_COLOR = -1,
  RED,
  GREEN,
  BLUE,
  ORANGE,
  YELLOW,
  BLACK,
  GREY,
  WHITE,
};

class Particle
{
public:
  Particle(int x, int y, int base_color, int opacity, int max_radius) : 
      m_cx(x), m_cy(y), m_base_color(base_color), m_opacity(opacity), 
      m_body(NULL), m_shape(NULL), m_max_radius(max_radius), m_constraint(NULL),
      m_split_age(0), m_parent(NULL)
  {
    reset(); 
  }

  Particle(Particle *p)
  {
    m_dx = -p->m_dx;
    m_dy = -p->m_dy;
    m_cx = p->m_cx;
    m_cy = p->m_cy;
    m_move_after = p->m_move_after;
    m_radius = p->m_radius;
    m_outline = p->m_outline;
    m_fill = p->m_fill;
    m_base_color = p->m_base_color;
    m_opacity = p->m_opacity;
  }

  virtual ~Particle() 
  { 
    cpShapeFree(m_shape);
    cpBodyFree(m_body); 
  }

  void reset()
  {
    change_size(); 
    change_color();
    change_direction();
    
    if (!m_body)
      m_body = cpBodyNew(m_radius * 20.0, INFINITY);

    m_body->p = cpv(m_cx, m_cy);
    m_body->v = cpvzero;
    m_body->v_limit = 100.0f;
    cpBodySetMoment(m_body, INFINITY);

    if (!m_shape)
      m_shape = cpCircleShapeNew(m_body, m_radius, cpvzero);

    ((cpCircleShape *)m_shape)->c = cpvzero;
    ((cpCircleShape *)m_shape)->r = m_radius;
    m_shape->e = 2.5;
    m_shape->u = 1.8;
    m_shape->data = this;
    m_shape->collision_type = 1;
    m_split_age = 0;
  }

  void change_direction()
  {
    m_move_after = 1 + (genrand64_int64() % 4);
    m_split_after = 1 + (genrand64_int64() % 20);
    m_dx = ((genrand64_int64() % 5) + 1) * (genrand64_int64() % 2 == 0 ? -1 : 1);
    m_dy = ((genrand64_int64() % 5) + 1) * (genrand64_int64() % 2 == 0 ? -1 : 1);
    
    if (m_body)
      cpBodyUpdateVelocity(m_body, cpv(m_dx * 10, m_dy * 10), 1.0, 1.0);
  }

  void change_color()
  {
    switch (m_base_color)
    {
    case -1:
      {
        int num_kulers = 5;
        int i = genrand64_int64() % num_kulers;
        m_outline = g_kulers[i];
        m_outline.a = m_opacity;
        i = genrand64_int64() % num_kulers;
        m_fill = g_kulers[i];
        m_fill.a = m_opacity;
      }
      break;
    case 0: // red
      m_outline = _RGBA(128 + genrand64_int64() % 128, genrand64_int64() % 32, genrand64_int64() % 32, m_opacity);
      m_fill = _RGBA(128 + genrand64_int64() % 128, genrand64_int64() % 32, genrand64_int64() % 32, m_opacity);
      break;
    case 1: // green
      m_outline = _RGBA(genrand64_int64() % 32, 128 + genrand64_int64() % 128, genrand64_int64() % 32, m_opacity);
      m_fill = _RGBA(genrand64_int64() % 32, 128 + genrand64_int64() % 128, genrand64_int64() % 32, m_opacity);
      break;
    case 2: // blue
      m_outline = _RGBA(genrand64_int64() % 64, genrand64_int64() % 128, 255, m_opacity);
      m_fill = _RGBA(genrand64_int64() % 64, genrand64_int64() % 128, 255, m_opacity);
      break;
    case 3: // orange
      m_outline = _RGBA(255, 128 + genrand64_int64() % 32, 0, m_opacity);
      m_fill = _RGBA(255, 128 + genrand64_int64() % 32, 0, m_opacity);
      break;
    case 4: // yellow
      m_outline = _RGBA(255, 192 + genrand64_int64() % 62, 0, m_opacity);
      m_fill = _RGBA(255, 192 + genrand64_int64() % 62, 0, m_opacity);
      break;
    case 5: // black
      {
        int rnd = genrand64_int64() % 128;
        m_outline = _RGBA(rnd, rnd, rnd, m_opacity);
        rnd = genrand64_int64() % 128;
        m_fill = _RGBA(rnd, rnd, rnd, m_opacity);
        
      }
      break;
    case 6: // grey
      {
        int rnd = 128 + genrand64_int64() % 128;
        m_outline = _RGBA(rnd, rnd, rnd, m_opacity);
        rnd = 128 + genrand64_int64() % 128;
        m_fill = _RGBA(rnd, rnd, rnd, m_opacity);
      }
      break;

    case 7: // white
      {
        int rnd = 224 + genrand64_int64() % 32;
        m_outline = _RGBA(rnd, rnd, rnd, m_opacity);
        rnd = 224 + genrand64_int64() % 32;
        m_fill = _RGBA(rnd, rnd, rnd, m_opacity);
      }
      break;
    }
  }

  void change_size()
  {
    m_radius = /*1 + genrand64_int64() %*/ m_max_radius;

    if (m_shape)
      ((cpCircleShape *)m_shape)->r = m_radius;
  }

  Particle *split()
  {
    Particle *p = NULL;
    p = new Particle(this);

    return(p);
  }

  void tick()
  {
    m_cx = m_shape->body->p.x;
    m_cy = m_shape->body->p.y;
    
    if (m_split_age > 0)
      m_split_age--;

    /*
    m_cx += m_dx;
    m_cy += m_dy;
    */
  }

  bool can_split()
  {
    return(m_split_age == 0);
  }

  void set_has_split()
  {
    m_split_age = 60;
  }

  int m_dx, m_dy;
  int m_cx, m_cy;
  int m_move_after, m_split_after;
  int m_radius;
  int m_base_color;
  int m_opacity;
  int m_max_radius;
  int m_split_age;
  cpConstraint *m_constraint;
  agg::rgba8 m_outline, m_fill;
  cpBody *m_body;
  cpShape *m_shape;
  void *m_parent;
};

class Firework
{
public:
  Firework(int cx, int cy, int maxw, int maxh, BRUSH_COLOR base_color, BRUSH_SHAPE base_shape, int opacity, int max_radius);
  virtual ~Firework();

  void reset();
  void tick();
  void free_particles();

  static int total_colors();
  static int total_shapes();
  void set_max_radius(int max_radius);
  int get_max_radius();

  int m_age;
  int m_cx, m_cy;
  int m_ww, m_wh;
  int m_source_radius;
  int m_opacity;
  BRUSH_COLOR m_base_color;
  BRUSH_SHAPE m_base_shape;
  WDL_PtrList<Particle> m_particles;
  int m_max_radius;
};

#endif // _DEMOS_FIREWORK_H