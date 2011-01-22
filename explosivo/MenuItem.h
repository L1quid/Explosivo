#ifndef _EXPLOSIVO_MENUITEM_H
#define _EXPLOSIVO_MENUITEM_H

#include <Windows.h>
#include <string>

#include "TextRenderer.h"

#define ITEM_HEIGHT 50
#define ITEM_WIDTH 150

class MenuItem
{
public:
  typedef void (*menuItemClickCallback)(MenuItem *target, int x, int y);

  MenuItem(void *parent, std::string label, int id, menuItemClickCallback callback = NULL);
  virtual ~MenuItem();

  virtual void onMouseOver(int x, int y);
  virtual void onMouseOut();
  virtual void onLMouseDown(int x, int y);
  virtual void onLMouseUp(int x, int y);

  bool IsHovered();

  void *GetParent();
  void SetParent(void *parent);

  int GetId();
  void SetId(int id);

  void SetOnClickCallback(menuItemClickCallback callback);
  void SetTextRenderer(TextRenderer *txt_ren);

  void SetPos(int x1, int y1, int x2, int y2);
  RECT *GetPos();

  bool IsPointWithin(int x, int y);

  void Draw(renbase_type *ren_base);

  void SetColor(agg::rgba8 color, agg::rgba8 on_hover, agg::rgba8 on_click, agg::rgba8 on_check);
  void SetBackgroundColor(agg::rgba8 color, agg::rgba8 on_hover, agg::rgba8 on_click, agg::rgba8 on_check);
  void SetBorderColor(agg::rgba8 color, agg::rgba8 on_hover, agg::rgba8 on_click, agg::rgba8 on_check);

  void SetFontSize(int size);

  void SetIsCheckable(bool checkable) { m_checkable = checkable;  if (!checkable) m_checked = false; }
  bool IsCheckable() { return(m_checkable); }
  void SetIsChecked(bool checked) { if (!m_checkable) return; m_checked = checked; }
  bool IsChecked() { return(m_checked); }
  void SetIsStickyCheck(bool sticky) { if (!m_checkable) return; m_sticky_check = sticky; }
  bool IsStickyCheck() { return(m_sticky_check); }

protected:
  virtual void draw_borders(renbase_type *ren_base);
  virtual void draw_background(renbase_type *ren_base);
  virtual void draw_label(renbase_type *ren_base);
  virtual void draw_decoration(renbase_type *ren_base);

  virtual void decoration_coords(POINT *pt);

  RECT m_rect;
  std::string m_label;
  bool m_mouse_over, m_mouse_down;
  void *m_parent;
  int m_id;
  int m_font_size;
  menuItemClickCallback m_callback;
  TextRenderer *m_txt_ren;
  bool m_checkable, m_checked, m_sticky_check;

  agg::rgba8 m_color, m_hover_color, m_click_color, m_check_color;
  agg::rgba8 m_bg_color, m_bg_hover_color, m_bg_click_color, m_bg_check_color;
  agg::rgba8 m_border_color, m_border_hover_color, m_border_click_color, m_border_check_color;
};

#endif // _EXPLOSIVO_MENUITEM_H