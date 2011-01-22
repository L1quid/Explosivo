#ifndef _EXPLOSIVO_POPUPMENUITEM_H
#define _EXPLOSIVO_POPUPMENUITEM_H

#include "menuitem.h"

#define POPUP_PARENT MenuItem

class PopupMenuItem : public POPUP_PARENT
{
public:
  PopupMenuItem(void *parent, std::string label, int id, menuItemClickCallback callback);
  virtual ~PopupMenuItem();

  void *GetPopup();

protected:
  void *m_popup;
  void draw_decoration(renbase_type *ren_base);
};

#endif // _EXPLOSIVO_POPUPMENUITEM_H