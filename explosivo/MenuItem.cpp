#include "MenuItem.h"
#include "Menu.h"

MenuItem::MenuItem(void *parent, std::string label, int id, menuItemClickCallback callback) 
  : m_parent(parent), m_label(label), m_id(id), m_callback(callback), m_mouse_over(false), m_mouse_down(false),
    m_color(agg::rgba8(255,255,255,224)), m_hover_color(agg::rgba8(255,255,255,255)), m_click_color(agg::rgba8(255,255,255,255)), m_check_color(agg::rgba8(255,255,0,255)),
    m_bg_color(agg::rgba8(255,255,255,64)), m_bg_hover_color(agg::rgba8(255, 255, 255, 92)), m_bg_click_color(agg::rgba8(255,255,255,128)), m_bg_check_color(agg::rgba8(255,255,255,128)),
    m_border_color(agg::rgba8(255,255,255,255)), m_border_hover_color(agg::rgba8(255,255,255,255)), m_border_click_color(agg::rgba8(255,255,255,255)), m_border_check_color(agg::rgba8(255,0,0,255)),
    m_font_size(12), m_checkable(true), m_checked(false), m_sticky_check(false)
{
}

MenuItem::~MenuItem()
{
  /*
  if (m_txt_ren)
  {
    delete m_txt_ren;
    m_txt_ren = NULL;
  }
  */
}

void MenuItem::onMouseOver(int x, int y)
{
  m_mouse_over = true;
}

void MenuItem::onMouseOut()
{
  m_mouse_over = false;
  m_mouse_down = false;
}

void MenuItem::onLMouseDown(int x, int y)
{
  m_mouse_down = true;
}

void MenuItem::onLMouseUp(int x, int y)
{
  if (!m_mouse_down)
    return;

  if (m_callback && (!m_checked || m_sticky_check))
    (*m_callback)(this, x, y);

  ((Menu *)m_parent)->CheckItem(this);
  m_mouse_down = false;
}

bool MenuItem::IsHovered()
{
  return(m_mouse_over);
}

void *MenuItem::GetParent()
{
  return(m_parent);
}

int MenuItem::GetId()
{
  return(m_id);
}

void MenuItem::SetId(int id)
{
  m_id = id;
}

void MenuItem::SetOnClickCallback(menuItemClickCallback callback)
{
  m_callback = callback;
}

void MenuItem::SetTextRenderer(TextRenderer *txt_ren)
{
  m_txt_ren = txt_ren;
}

void MenuItem::SetPos(int x1, int y1, int x2, int y2)
{
  m_rect.left = x1;
  m_rect.top = y1;
  m_rect.right = x2;
  m_rect.bottom = y2;
}

RECT *MenuItem::GetPos()
{
  return(&m_rect);
}

bool MenuItem::IsPointWithin(int x, int y)
{
  return (x >= m_rect.left && x <= m_rect.right &&
      y >= m_rect.top && y <= m_rect.bottom);
}

void MenuItem::Draw(renbase_type *ren_base)
{
  POINT pt;
  ((Menu *)m_parent)->GetParentDimensions(&pt);
  int h = 0;

  if (ren_base)
  {
    draw_background(ren_base);
    draw_borders(ren_base);
    draw_decoration(ren_base);

    if (m_txt_ren)
      draw_label(ren_base);
  }
}

void MenuItem::SetColor(agg::rgba8 color, agg::rgba8 on_hover, agg::rgba8 on_click, agg::rgba8 on_check)
{
  m_color = color;
  m_hover_color = on_hover;
  m_click_color = on_click;
  m_check_color = on_check;
}

void MenuItem::SetBackgroundColor(agg::rgba8 color, agg::rgba8 on_hover, agg::rgba8 on_click, agg::rgba8 on_check)
{
  m_bg_color = color;
  m_bg_hover_color = on_hover;
  m_bg_click_color = on_click;
  m_bg_check_color = on_check;
}

void MenuItem::SetBorderColor(agg::rgba8 color, agg::rgba8 on_hover, agg::rgba8 on_click, agg::rgba8 on_check)
{
  m_border_color = color;
  m_border_hover_color = on_hover;
  m_border_click_color = on_click;
  m_border_check_color = on_check;
}

void MenuItem::SetFontSize(int size)
{
  m_font_size = size;
}

void MenuItem::draw_borders(renbase_type *ren_base)
{
  agg::rgba8 clr;

  if (m_checked)
  {
    clr = m_border_check_color;
  }
  else
  {
    if (m_mouse_over)
      clr = m_mouse_down ? m_border_click_color : m_border_hover_color;
    else
      clr = m_border_color;
  }
  

  // LEFT
  ren_base->blend_bar(m_rect.left, m_rect.top, m_rect.left + 5, m_rect.top, clr, clr.a); // top-h
  ren_base->blend_bar(m_rect.left, m_rect.bottom, m_rect.left + 5, m_rect.bottom, clr, clr.a); // bottom-h
  ren_base->blend_bar(m_rect.left, m_rect.top, m_rect.left, m_rect.top + 5, clr, clr.a); // top-v
  ren_base->blend_bar(m_rect.left, m_rect.bottom, m_rect.left, m_rect.bottom - 5, clr, clr.a); // bottom-v

  // RIGHT
  ren_base->blend_bar(m_rect.right, m_rect.top, m_rect.right - 5, m_rect.top, clr, clr.a); // top-h
  ren_base->blend_bar(m_rect.right, m_rect.bottom, m_rect.right - 5, m_rect.bottom, clr, clr.a); // bottom-h
  ren_base->blend_bar(m_rect.right, m_rect.top, m_rect.right, m_rect.top + 5, clr, clr.a); // top-v
  ren_base->blend_bar(m_rect.right, m_rect.bottom, m_rect.right, m_rect.bottom - 5, clr, clr.a); // bottom-v
}

void MenuItem::draw_background(renbase_type *ren_base)
{
  agg::rgba8 clr;

  if (m_checked)
    clr = m_bg_check_color;
  else
  {
    if (m_mouse_over)
      clr = m_mouse_down ? m_bg_click_color : m_bg_hover_color;
    else
      clr = m_bg_color;
  }

  ren_base->blend_bar(m_rect.left, m_rect.top, m_rect.right, m_rect.bottom, clr, clr.a);
}

void MenuItem::draw_label(renbase_type *ren_base)
{
  agg::rgba8 clr;

  if (m_checked)
    clr = m_check_color;
  else
  {
    if (m_mouse_over)
      clr = m_mouse_down ? m_click_color : m_hover_color;
    else
      clr = m_color;
  }

  m_txt_ren->SetFontSize(m_font_size);
  m_txt_ren->SetColor(clr);
  int w = m_rect.right - m_rect.left;
  int tw = m_label.length() * m_txt_ren->GetCharacterWidth();
  int h = m_rect.bottom - m_rect.top;
  int th = m_txt_ren->GetCharacterHeight();
  int x = (w / 2) - (tw / 2);
  int y = (h / 2) - (th / 2); // THIS SUCKS AND ONLY SUPPORTS SINGLE LINE TEXT
  m_txt_ren->SetText(m_label.c_str(), m_rect.left + 10, m_rect.top + y);
}

void MenuItem::draw_decoration(renbase_type *ren_base)
{
  if (!m_checkable)
    return;

  agg::rgba8 clr;
  POINT pt = {0,0,};
  decoration_coords(&pt);
  agg::renderer_markers<renbase_type> marker(*ren_base);

  if (m_checked)
    clr = m_check_color;
  else
  {
    if (m_mouse_over)
      clr = m_mouse_down ? m_click_color : m_hover_color;
    else
      clr = m_color;
  }
  clr.a = 192;
  marker.line_color(clr);
  clr.a = 128;
  marker.fill_color(clr);
  marker.dot(pt.x, pt.y, m_font_size / 2);
}

void MenuItem::decoration_coords(POINT *pt)
{
  pt->x = m_rect.right - 25;
  pt->y = m_rect.bottom - ((m_rect.bottom - m_rect.top) / 2);
}