#include "TextRenderer.h"
#include "resource.h"

FT_Library g_freetype;
FT_Face g_ft_face;
unsigned char *g_font_file = NULL;
extern renbase_type *g_ui_renderer_base;

#define RGB_R(x) (x & 0x00ff0000 >> 16)
#define RGB_G(x) (x & 0x0000ff00 >> 8)
#define RGB_B(x) (x & 0x000000ff)

TextRenderer::TextRenderer(IMAGE_BUFFER_TYPE *buffer, int w, int h) : m_buffer(buffer)
{
  m_rect.left = 0;
  m_rect.top = 0;
  m_rect.right = w;
  m_rect.bottom = h;
  m_slot = g_ft_face->glyph;
  SetColor(agg::rgba8(255, 255, 255));
  m_font_size = 0;
}

TextRenderer::~TextRenderer()
{
}

void TextRenderer::init()
{
  FT_Error error;
  error = FT_Init_FreeType(&g_freetype);

  if (error)
    return;

  HRSRC hResInfo = FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_TTF1), "TTF");
  HGLOBAL hRes = LoadResource(GetModuleHandle(NULL), hResInfo);
  LPVOID memRes = LockResource(hRes);
  DWORD sizeRes = SizeofResource(GetModuleHandle(NULL), hResInfo);

  /*FILE *fp = fopen("./market_deco.TTF", "rb");
  fseek(fp, 0, SEEK_END);
  int fsize = ftell(fp);
  rewind(fp);
  g_font_file = new unsigned char[fsize];
  fread(g_font_file, 1, fsize, fp);
  fclose(fp);*/

  error = FT_New_Memory_Face(g_freetype, (FT_Byte *)memRes, sizeRes, 0, &g_ft_face);
  //FT_New_Face(g_freetype, "./EHSMB.TTF", 0, &g_ft_face);

  if (error)
    return;

  set_font_size(16);
}

void TextRenderer::deinit()
{
  FT_Done_Face(g_ft_face);

  if (g_font_file)
  {
    delete [] g_font_file;
    g_font_file = NULL;
  }

  FT_Done_FreeType(g_freetype);
}

void TextRenderer::set_font_size(int pts)
{
  FT_Set_Char_Size(g_ft_face, pts * 64, 0, 120, 120);
}

void TextRenderer::SetFontSize(int pts)
{
  m_font_size = pts;
  TextRenderer::set_font_size(pts);
}

int TextRenderer::GetCharacterHeight()
{
  FT_Size_Metrics* metrics = &g_ft_face->size->metrics;
  double yscale = metrics->y_ppem / (1.0 * g_ft_face->units_per_EM);
  int adv = g_ft_face->height * yscale;
  int diff = adv - m_font_size;

  return(m_font_size + (diff / 2));
}

int TextRenderer::GetCharacterWidth()
{
  FT_Size_Metrics* metrics = &g_ft_face->size->metrics;
  double xscale = metrics->x_ppem / (1.0 * g_ft_face->units_per_EM);
  int adv = g_ft_face->max_advance_width * xscale;

  return(adv);
}

void TextRenderer::SetText(const char *str, int x, int y)
{
  if (!g_freetype || !g_ft_face)
    return;

  FT_Error error;
  int cx = x, cy = y;
  const int len = strlen(str);

  for (int i = 0; i < len; i++)
  {
    if (str[i] == '\r')
      continue;

    error = FT_Load_Char(g_ft_face, str[i], FT_LOAD_RENDER);

    if (error)
      continue;

    if (str[i] == '\n')
    {
      cy += GetCharacterHeight();
      cx = x;
    }
    else
    {
      render_glyph(cx, cy);
      int adv = m_slot->advance.x >> 6;
      cx += adv;
    }
  }
}

void TextRenderer::render_glyph(int x, int y)
{
  x += m_slot->bitmap_left;
  //y -= m_slot->bitmap_top;

  FT_Int  i, j, p, q;
  FT_Int  x_max = x + m_slot->bitmap.width;
  FT_Int  y_max = y + m_slot->bitmap.rows;

  for ( i = x, p = 0; i < x_max; i++, p++ )
  {
    for ( j = y, q = 0; j < y_max; j++, q++ )
    {
      if ( i >= m_rect.right || j >= m_rect.bottom )
        continue;

      if (m_slot->bitmap.buffer[q * m_slot->bitmap.width + p])
      {
        agg::rgba8 clr = m_color;
        int val = RGB_R(m_slot->bitmap.buffer[q * m_slot->bitmap.width + p]);
        float ratio = (float)val / 255.0f;
        clr.r *= ratio;
        clr.g *= ratio;
        clr.b *= ratio;
        //clr.a *= ratio;
        
        g_ui_renderer_base->blend_pixel(i, j, clr, clr.a);
      }
    }
  }
}

void TextRenderer::SetColor(agg::rgba8 color)
{
  m_color = color;
}