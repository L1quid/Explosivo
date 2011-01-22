#ifndef _EXPLOSIVO_TEXTRENDERER_H
#define _EXPLOSIVO_TEXTRENDERER_H

#include "config.h"
#include <windows.h>

class TextRenderer
{
public:
  TextRenderer(IMAGE_BUFFER_TYPE *buffer, int w, int h);
  ~TextRenderer();

  static void init();
  static void deinit();

  static void set_font_size(int pts);
  void SetFontSize(int pts);
  int GetCharacterHeight();
  int GetCharacterWidth();

  void SetText(const char *str, int x, int y);

  void SetColor(agg::rgba8 color);

private:
  void render_glyph(int x, int y);
  FT_GlyphSlot m_slot;
  RECT m_rect;
  IMAGE_BUFFER_TYPE *m_buffer;
  agg::rgba8 m_color;
  int m_font_size;
};

#endif // _EXPLOSIVO_TEXTRENDERER_H