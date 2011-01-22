#include "PopupMenuItem.h"
#include "Menu.h"

PopupMenuItem::PopupMenuItem(void *parent, std::string label, int id, menuItemClickCallback callback)
: POPUP_PARENT(parent, label, id, callback)
{
  m_popup = new Menu((Menu *)parent);
  m_checkable = false;
  m_checked = false;
}

PopupMenuItem::~PopupMenuItem()
{
  delete m_popup;
}

void *PopupMenuItem::GetPopup()
{
  return(m_popup);
}

void PopupMenuItem::draw_decoration(renbase_type *ren_base)
{
  POINT pt = {0,0,};
  decoration_coords(&pt);
  agg::renderer_markers<renbase_type> marker(*ren_base);

  marker.line_color(agg::rgba8(255,255,255,192));
  marker.fill_color(agg::rgba8(255,255,255,128));
  marker.triangle_right(pt.x, pt.y, m_font_size / 2);
}