#ifndef _EXPLOSIVO_MENU_H
#define _EXPLOSIVO_MENU_H

#include "config.h"
#include "MenuItem.h"
#include "PopupMenuItem.h"
#include "../wdl/ptrlist.h"

#define ITEM_SPACING 10

class Menu
{
public:
  Menu(int parent_w, int parent_h, IMAGE_BUFFER_TYPE *buffer, agg::rendering_buffer *ren_buf, renbase_type *ren_base, pixfmt_type *pix_fmt);
  Menu(Menu *menu);
  virtual ~Menu();

  void reset_buffer(int w, int h);

  MenuItem *AddItem(std::string label, int id, MenuItem::menuItemClickCallback callback = NULL);
  MenuItem *AddPopupItem(std::string label, int id, MenuItem::menuItemClickCallback callback = NULL);

  bool OnLMouseDown(int x, int y);
  bool OnLMouseUp(int x, int y);
  bool OnMouseMove(int x1, int y1);

  void Draw();
  void Invalidate(bool invalidate = true);

  void SetTextRenderer(TextRenderer *tr);
  void SetPos(int x, int y);
  void GetParentDimensions(POINT *pt);
  
  void SetParent(void *parent);
  void *GetParent();

  int MaxItemsPerCol();
  int ItemXOffset(int i);
  int ItemYOffset(int i);

  void CheckItem(MenuItem *item);

protected:
  void finish_add_item(MenuItem *item);

  IMAGE_BUFFER_TYPE *m_display_buffer;
  WDL_PtrList<MenuItem> m_menu_items;
  int m_pw, m_ph;
  int m_cx, m_cy;
  POINT m_mouse_move;
  TextRenderer *m_text_render;
  void *m_parent;
  MenuItem *m_checked_item;
  bool m_invalidated;

public:
  // agg
  agg::rendering_buffer *m_ren_buf;
  pixfmt_type *m_pixfmt;
  renbase_type *m_ren_base;
};

#endif // _EXPLOSIVO_MENU_H