#include "Menu.h"
#include "TextRenderer.h"
#include "windows.h"

Menu::Menu(int parent_w, int parent_h, IMAGE_BUFFER_TYPE *buffer, agg::rendering_buffer *ren_buf, renbase_type *ren_base, pixfmt_type *pix_fmt) 
  : m_pw(parent_w), m_ph(parent_h), m_display_buffer(buffer), m_ren_base(ren_base), m_ren_buf(ren_buf),
    m_pixfmt(pix_fmt), m_text_render(NULL), m_cx(0), m_cy(0), m_parent(NULL), m_checked_item(NULL), m_invalidated(true)
{
  reset_buffer(m_pw, m_ph);
  m_mouse_move.x = -1;
  m_mouse_move.y = -1;
}

Menu::Menu(Menu *menu)
{
  m_display_buffer = menu->m_display_buffer;
  m_pw = menu->m_pw;
  m_ph = menu->m_ph;
  m_cx = menu->m_cx;
  m_cy = menu->m_cy;
  m_mouse_move.x = menu->m_mouse_move.x;
  m_mouse_move.y = menu->m_mouse_move.y;
  m_text_render = menu->m_text_render;
  m_ren_buf = menu->m_ren_buf;
  m_ren_base = menu->m_ren_base;
  m_pixfmt = menu->m_pixfmt;
  m_checked_item = NULL;
}

Menu::~Menu()
{
  reset_buffer(0, 0);
  m_menu_items.Empty(true);
}

void Menu::reset_buffer(int w, int h)
{
  m_pw = w;
  m_ph = h;

  /*
  if (m_display_buffer)
  {
    delete [] m_display_buffer;
    m_display_buffer = NULL;
    delete m_ren_base;
    delete m_pixfmt;
    delete m_ren_buf;
  }
  */

  if (m_pw > 0 && m_ph > 0)
  {
    /*
    int bytes = m_pw * m_ph * BPP;
    m_display_buffer = new IMAGE_BUFFER_TYPE[bytes];
    memset(m_display_buffer, 0, bytes);
    m_ren_buf = new agg::rendering_buffer(m_display_buffer, m_pw, m_ph, -m_pw * BPP);
    m_pixfmt = new pixfmt_type(*m_ren_buf);
    m_ren_base = new renbase_type(*m_pixfmt);
    m_ren_base->clear(agg::rgba8(0, 0, 0, 0));
    */
  }
}

MenuItem *Menu::AddItem(std::string label, int id, MenuItem::menuItemClickCallback callback)
{
  MenuItem *ret = new MenuItem(this, label, id, callback);
  finish_add_item(ret);

  return(ret);
}

MenuItem *Menu::AddPopupItem(std::string label, int id, MenuItem::menuItemClickCallback callback)
{
  MenuItem *ret = new PopupMenuItem(this, label, id, callback);
  finish_add_item(ret);

  return(ret);
}

bool Menu::OnLMouseDown(int x, int y)
{
  for (int i = 0; i < m_menu_items.GetSize(); i++)
  {
    MenuItem *item = m_menu_items.Get(i);

    if (item->IsPointWithin(x, y))
    {
      item->onLMouseDown(x, y);
      Invalidate();
      return(true);
    }
  }

  return(false);
}

bool Menu::OnLMouseUp(int x, int y)
{
  for (int i = 0; i < m_menu_items.GetSize(); i++)
  {
    MenuItem *item = m_menu_items.Get(i);

    if (item->IsPointWithin(x, y))
    {
      item->onLMouseUp(x, y);
      Invalidate();
      return(true);
    }
  }

  return(false);
}

bool Menu::OnMouseMove(int x1, int y1)
{
  bool ret = false;

  for (int i = 0; i < m_menu_items.GetSize(); i++)
  {
    MenuItem *item = m_menu_items.Get(i);

    if (item->IsPointWithin(x1, y1))
    {
      if (!item->IsHovered())
        item->onMouseOver(x1, y1);

      ret = true;
    }
    else
    {
      if (item->IsHovered())
        item->onMouseOut();
    }
  }

  m_mouse_move.x = x1;
  m_mouse_move.y = y1;

  if (ret)
    Invalidate();

  return(ret);
}

void Menu::Draw()
{
  //if (!m_invalidated)
  //  return;

  for (int i = 0; i < m_menu_items.GetSize(); i++)
  {
    MenuItem *item = m_menu_items.Get(i);
    item->Draw(m_ren_base);
  }

  Invalidate(false);
}

void Menu::Invalidate(bool invalidate)
{
  m_invalidated = invalidate;
}

void Menu::SetTextRenderer(TextRenderer *tr)
{
  m_text_render = tr;
}

void Menu::SetPos(int x, int y)
{
  m_cx = x;
  m_cy = y;
  Invalidate();
}

void Menu::GetParentDimensions(POINT *pt)
{
  pt->x = m_pw;
  pt->y = m_ph;
}

void Menu::SetParent(void *parent)
{
  m_parent = parent;
}

void *Menu::GetParent()
{
  return(m_parent);
}

int Menu::MaxItemsPerCol()
{
  int ret = floor((float)m_ph / (float)(ITEM_HEIGHT + ITEM_SPACING));

  return(ret - 1);
}

int Menu::ItemXOffset(int i)
{
  int col = i / MaxItemsPerCol();
  
  return(col * (ITEM_WIDTH + ITEM_SPACING));
}

int Menu::ItemYOffset(int i)
{
  int row = i % MaxItemsPerCol();

  return(row * (ITEM_HEIGHT + ITEM_SPACING));
}

void Menu::CheckItem(MenuItem *item)
{
  if (!item->IsCheckable())
    return;

  if (m_checked_item)
  {
    m_checked_item->SetIsChecked(false);
    m_checked_item = NULL;
  }

  item->SetIsChecked(!item->IsChecked());

  if (!item->IsStickyCheck())
    m_checked_item = item;

  Invalidate();
}

void Menu::finish_add_item(MenuItem *item)
{
  TextRenderer *txt_ren = m_text_render;// ? m_text_render : new TextRenderer(m_display_buffer, m_pw, m_ph);
  item->SetTextRenderer(txt_ren);
  MenuItem *last = NULL;
  int newidx = m_menu_items.GetSize();
  int x = m_cx + ItemXOffset(newidx), y = m_cy + ItemYOffset(newidx);

  /*
  if (newidx > 0)
  {
    last = m_menu_items.Get(newidx - 1);
    RECT *r = last->GetPos();
    y = r->bottom + ITEM_SPACING;
  }
  */

  int x1 = x, x2 = x + ITEM_WIDTH;
  int y1 = y, y2 = y + ITEM_HEIGHT;

  /*
  if (y2 >= m_ph)
  {
    y1 = m_cy;
    x1 = x + ITEM_WIDTH + ITEM_SPACING;
    x2 = x1 + ITEM_WIDTH;
    y2 = y1 + ITEM_HEIGHT;
  }
*/

  item->SetPos(x1, y1, x2, y2);
  m_menu_items.Add(item);
  Invalidate();
}