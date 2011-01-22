#define APP_NAME "Explosivo"
#define APP_VERSION "0.94"
#define APP_LABEL APP_NAME " v" APP_VERSION "\r\nby Daniel Green"

#include <windows.h>
#include <windowsx.h>
#include <assert.h>
#include <time.h>
#include <process.h>
#include <stdio.h>

#include "chipmunk.h"

cpSpace *g_space = NULL;

#include "TextRenderer.h"

#include "intel_rand.h"
#include "GetUrl.h"
#include "Menu.h"

#include "explosivo.h"
#include "config.h"
#include "scopelock.h"
//#include "Emitter.h"
//#include "Streamlet.h"
#include "../wdl/ptrlist.h"
#include "resource.h"
#include "png.h"

unsigned int WIDTH = 1024, HEIGHT = 768;
HANDLE g_mut_emitter, g_mut_render;
HANDLE g_render_thread = NULL, g_process_thread = NULL;
HANDLE g_render_wait = NULL, g_process_wait = NULL;
HANDLE g_render_done = NULL, g_process_done = NULL;
HANDLE g_paused = NULL;
HDC g_screendc = NULL, g_memdc = NULL, g_renderdc = NULL;
HANDLE g_screen_old_obj = NULL;
HBITMAP g_membmp = NULL, g_renderbmp = NULL;
volatile bool g_killswitch = false;
HWND g_mainwnd = NULL;
BITMAPINFO g_bmi;
//WDL_PtrList<Emitter> g_emitters;
WDL_PtrList<Firework> g_fireworks;
HCURSOR g_cur_open = NULL, g_cur_grab = NULL, g_cur_pointer = NULL;
unsigned int g_frames_rendered = 0;
float g_fps = 60.0f;
DWORD g_started_at = 0;
bool g_is_paused = false, g_is_resizing = false, g_is_fullscreen = false;
int g_last_x = 0, g_last_y = 0;
unsigned int g_age = 0;
bool g_is_aot = false;
BRUSH_COLOR g_base_color = BRUSH_COLOR::RED;
BRUSH_SHAPE g_base_shape = BRUSH_SHAPE::DOT;
bool g_just_reset = false;
bool g_render_stats = false;
bool g_shadows = false;
int g_opacity = 255;
int g_fade = 1;
int g_max_radius = 5;
agg::rgba8 g_bg_color = _RGB(0,0,0);
typedef struct ParticlePair { Particle *a, *b; };
WDL_PtrList<ParticlePair> g_delayed_fireworks;
agg::rgba8 g_kulers[5];
Menu *g_menu = NULL;
bool g_menu_active = true;
bool g_render_about = false;
bool g_scheduled_reset = false;

TextRenderer *g_ui_text_render = NULL;

int size_ids[] = {
  ID_SIZES_RANDOM,
  ID_SIZES_1,
  ID_SIZES_2,
  ID_SIZES_4,
  ID_SIZES_5,
  ID_SIZES_10,
  ID_SIZES_15,
  ID_SIZES_20,
  ID_SIZES_25,
  ID_SIZES_50,
  ID_SIZES_100,
};
int size_radii[] = {
  -1,
  1,
  2,
  4,
  5,
  8,
  10,
  12,
  15,
  20,
  25,
};
const int num_sizes = sizeof(size_ids) / sizeof(int);

const char *fade_titles[] = {
  "Off",
  "Slowest",
  "Slower",
  "Slow",
  "Moderate",
  "Fast",
  "Faster",
  "Fastest"
};
int fade_values[] = {
  0,
  1,
  5,
  10,
  25,
  50,
  100,
  254,
};
const int num_fades = sizeof(fade_values) / sizeof(int);

const char *color_titles[] = {
  "Random",
  "Red",
  "Green",
  "Blue",
  "Orange",
  "Yellow",
  "Black",
  "Grey",
  "White",
};
const int num_colors = sizeof(color_titles) / sizeof(char *);

const char *shape_titles[] = {
  "Random",
  "Square",
  "Diamond",
  "Circle",
  "Crosshair",
  "Bullet (L)",
  "Bullet (R)",
  "Bullet (D)",
  "Bullet (U)",
  "Triangle (L)",
  "Triangle (R)",
  "Triangle (D)",
  "Triangle (U)",
  "Four Rays",
  "Cross",
  "Xing",
  "Dash",
  "Dot",
  "Pixel",
};
const int num_shapes = sizeof(shape_titles) / sizeof(char *);

IMAGE_BUFFER_TYPE *g_buffer = NULL;
IMAGE_BUFFER_TYPE *g_ui_buffer = NULL;
agg::rendering_buffer *g_rendering_buffer = NULL, *g_ui_rendering_buffer = NULL;
pixfmt_type *g_pixfmt = NULL, *g_ui_pixfmt = NULL;
renbase_type *g_renderer_base = NULL, *g_ui_renderer_base = NULL;

int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd)
{
  MSG msg;
  DWORD render_threadid, process_threadid;

  g_cur_open = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR1));
  g_cur_grab = LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR2));
  g_cur_pointer = LoadCursor(NULL, MAKEINTRESOURCE(IDC_HAND));
  
  WNDCLASS wc;
  ZeroMemory(&wc, sizeof(WNDCLASS));

  wc.style = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
  wc.lpfnWndProc = MainWndProc;
  wc.cbClsExtra = 0;
  wc.cbWndExtra = NULL;
  wc.hInstance = hInstance;
  wc.hIcon = NULL;
  wc.hCursor = NULL;
  wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
  wc.lpszMenuName = NULL;
  wc.lpszClassName = APP_NAME;
  
  ATOM atm = RegisterClass(&wc);

  if (atm == 0)
  {
    DWORD err = GetLastError();
    MessageBox(NULL, "ATOM!", "DOH!", MB_OK);
    return(1);
  }

  g_mainwnd = CreateWindow(
    //WS_EX_APPWINDOW, 
    APP_NAME,
    APP_LABEL, 
    ( WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN ),
    CW_USEDEFAULT, 
    CW_USEDEFAULT, 
    WIDTH, 
    HEIGHT, 
    NULL, 
    NULL, 
    hInstance, 
    NULL);

  if (!g_mainwnd)
  {
    DWORD err = GetLastError();
    MessageBox(NULL, "DOH!", "DOH!", MB_OK);
    return(1);
  }

  TextRenderer::init();
  init_kuler();
  init_image(WIDTH, HEIGHT);
  cpInitChipmunk();
  g_space = cpSpaceNew();
  g_space->gravity = cpv(0, 0);
  g_space->elasticIterations = 10;
  g_space->sleepTimeThreshold = 100.0;
  cpSpaceAddCollisionHandler(g_space, 1, 1, NULL, NULL, particleCollisionHandler, NULL, NULL);

  //InitializeCriticalSection(&g_mut_emitter);
  //InitializeCriticalSection(&g_mut_render);
  g_mut_emitter = CreateMutex(NULL, false, NULL);
  g_mut_render = CreateMutex(NULL, false, NULL);
  
  //g_render_wait = CreateEvent(0, false, false, 0);
  g_process_wait = CreateEvent(0, false, false, 0);
  //g_render_done = CreateEvent(0, false, false, 0);
  g_process_done = CreateEvent(0, false, false, 0);
  g_process_thread = CreateThread(0, 0, process_thread, NULL, 0, &process_threadid);
  //g_render_thread = CreateThread(0, 0, render_thread, NULL, 0, &render_threadid);
  srand(time(0));

  unsigned long long init[5]={0x12345ULL, 0x23456ULL, 0x34567ULL, /*0x45678ULL,*/ (long long)time(0)}, length=4;
  init_by_array64(init, length);

  //Streamlet::init_tables();

  /*
  int i = 0;
  const int max_emits = 8;

  while (i < max_emits)
  {
    Emitter *p = new Emitter();
    p->set_window(WIDTH, HEIGHT);
    p->set_offset(0, (HEIGHT / 2) + (10 * ++i));
    p->init_stream();
    g_emitters.Add(p);
  }
  */

  SetEvent(g_process_wait);
  SetEvent(g_render_wait);
  Sleep(500);
  //SetEvent(g_render_wait);

  ShowWindow(g_mainwnd, SW_SHOW);
  UpdateWindow(g_mainwnd);

  while(GetMessage(&msg, NULL, 0, 0) > 0)
  {
    if (IsDialogMessage(g_mainwnd, &msg))
      continue;

    TranslateMessage(&msg);
    DispatchMessage(&msg);
  }

  WaitForSingleObject(g_process_done, INFINITE);
  //WaitForSingleObject(g_render_done, INFINITE);
  CloseHandle(g_mut_render);
  CloseHandle(g_mut_emitter);

  // shut down
  deinit_image();
  TextRenderer::deinit();

  g_fireworks.Empty(true);
  cpSpaceRemoveCollisionHandler(g_space, 1, 1);
  cpSpaceFree(g_space);
  //g_emitters.Empty(true);
  //Streamlet::deinit_tables();

  return(0);
}

static DWORD WINAPI process_thread(void* arg)
{
  WaitForSingleObject(g_process_wait, INFINITE);

  while (!g_killswitch)
  {
    WaitForSingleObject(g_paused, INFINITE);
    int runtime = 0;

    try
    {
      /*int sp, ep;
      int sr, er;
      int pdiff, rdiff;
      sp = GetTickCount();*/

      if (g_scheduled_reset)
      {
        onDoubleClick();
        g_scheduled_reset = false;
      }

      process();
      /*ep = GetTickCount();
      sr = GetTickCount();*/
      render();
      /*er = GetTickCount();
      pdiff = ep - sp;
      rdiff = er - sr;
      runtime = pdiff + rdiff;*/
    }
    catch (char *s)
    {
      MessageBox(g_mainwnd, s, "WHOA", MB_OK);
    }
    
    /*int sleep = (int)(1000.0 / (double)FPS(WIDTH));
    int sleep_diff = sleep - runtime;

    if (sleep_diff > 0)
      Sleep(sleep_diff);*/
  }
#if 0
  char buf[128];
  int runtime = GetTickCount() - g_started_at;
  sprintf(buf, "fps: %d", (int)(runtime / g_frames_rendered));
  MessageBox(NULL, buf, "Status", MB_OK);
#endif // 0
  SetEvent(g_process_done);

  return(0);
}

static DWORD WINAPI tick_thread(void *arg)
{
  ((Firework *)arg)->tick();

  return(0);
}

void process()
{
  CShupMutexLock ml(g_mut_emitter, INFINITE);
  assert(ml.HaveLocked());

  if (g_is_paused)
    return;

  cpSpaceStep(g_space, 1.0 / (float)(g_fps > 60 ? g_fps : 60.0f));

  while (g_delayed_fireworks.GetSize() > 0)
  {
    ParticlePair *pp = g_delayed_fireworks.Get(0);
    Particle *pa = pp->a, *pb = pp->b;

    pa->m_constraint = cpPinJointNew(pa->m_body, pb->m_body, cpv(0,0), cpv(0,0));
    cpSpaceAddConstraint(g_space, pa->m_constraint);

    g_delayed_fireworks.Delete(0, true);
  }

  for (int i = 0; i < g_fireworks.GetSize(); i++)
  {
    Firework *f = g_fireworks.Get(i);
    f->tick();
  }

  /*
  for (int j = 0; j < g_emitters.GetSize(); j++)
  {
    Emitter *p = NULL;
    p = g_emitters.Get(j);

    if (!p)
      continue;

    p->tick();
  }
  */

  g_age++;
}

static DWORD WINAPI render_thread(void* arg)
{
  WaitForSingleObject(g_render_wait, INFINITE);

  while (!g_killswitch)
  {
    WaitForSingleObject(g_paused, INFINITE);
    render();
    //Sleep((int)(1000.0 / (double)FPS(WIDTH)));
  }

  SetEvent(g_render_done);

  return(0);
}

void render()
{
  static char buf[128];
  static int last_fps_log = 0;

  if (g_started_at == 0)
    g_started_at = GetTickCount();

  // draw all emitters
  {
    CShupMutexLock ml(g_mut_emitter, INFINITE);
    assert(ml.HaveLocked());

    if (!g_is_paused)
    {
      agg::renderer_markers<renbase_type> marker(*g_renderer_base);
      renderer_solid r(*g_renderer_base);
      int i;
      int j, jlen;
      RGBQUAD rgb;
      agg::rasterizer_scanline_aa<> ras;
      agg::scanline_u8 sl;
      static double last_x = -1;
      static double last_y = -1;

      if (!g_just_reset)
      {
        if (g_fade > 0)
        {
          // slowly blend away old frames
          g_renderer_base->blend_bar(0, 0, WIDTH - 1, HEIGHT - 1, g_bg_color, g_fade);
        }
      }
      else
      {
        g_just_reset = false;
        g_renderer_base->clear(g_bg_color);
      }


      for (int i = 0; i < g_fireworks.GetSize(); i++)
      {
        Firework *f = g_fireworks.Get(i);
        void (agg::renderer_markers<renbase_type>::*draw_fn)(int, int, int) = NULL;

        for (int j = 0; j < f->m_particles.GetSize(); j++)
        {
          Particle *p = f->m_particles.Get(j);

          switch (f->m_base_shape)
          {
          case BRUSH_SHAPE::SQUARE:
            draw_fn = &(agg::renderer_markers<renbase_type>::square);
            break;
          case BRUSH_SHAPE::DIAMOND:
            draw_fn = &(agg::renderer_markers<renbase_type>::diamond);
            break;
          case BRUSH_SHAPE::CIRCLE:
            draw_fn = &(agg::renderer_markers<renbase_type>::circle);
            break;
          case BRUSH_SHAPE::CROSSED_CIRCLE:
            draw_fn = &(agg::renderer_markers<renbase_type>::crossed_circle);
            break;
          case BRUSH_SHAPE::SEMI_ELLIPSE_LEFT:
            draw_fn = &(agg::renderer_markers<renbase_type>::semiellipse_left);
            break;
          case BRUSH_SHAPE::SEMI_ELLIPSE_RIGHT:
            draw_fn = &(agg::renderer_markers<renbase_type>::semiellipse_right);
            break;
          case BRUSH_SHAPE::SEMI_ELLIPSE_UP:
            draw_fn = &(agg::renderer_markers<renbase_type>::semiellipse_up);
            break;
          case BRUSH_SHAPE::SEMI_ELLIPSE_DOWN:
            draw_fn = &(agg::renderer_markers<renbase_type>::semiellipse_down);
            break;
          case BRUSH_SHAPE::TRIANGLE_LEFT:
            draw_fn = &(agg::renderer_markers<renbase_type>::triangle_left);
            break;
          case BRUSH_SHAPE::TRIANGLE_RIGHT:
            draw_fn = &(agg::renderer_markers<renbase_type>::triangle_right);
            break;
          case BRUSH_SHAPE::TRIANGLE_UP:
            draw_fn = &(agg::renderer_markers<renbase_type>::triangle_up);
            break;
          case BRUSH_SHAPE::TRIANGLE_DOWN:
            draw_fn = &(agg::renderer_markers<renbase_type>::triangle_down);
            break;
          case BRUSH_SHAPE::FOUR_RAYS:
            draw_fn = &(agg::renderer_markers<renbase_type>::four_rays);
            break;
          case BRUSH_SHAPE::CROSS:
            draw_fn = &(agg::renderer_markers<renbase_type>::cross);
            break;
          case BRUSH_SHAPE::XING:
            draw_fn = &(agg::renderer_markers<renbase_type>::xing);
            break;
          case BRUSH_SHAPE::DASH:
            draw_fn = &(agg::renderer_markers<renbase_type>::dash);
            break;
          case BRUSH_SHAPE::DOT:
            draw_fn = &(agg::renderer_markers<renbase_type>::dot);
            break;
          case BRUSH_SHAPE::PIXEL:
            draw_fn = &(agg::renderer_markers<renbase_type>::pixel);
            break;
          }

          if (g_shadows)
          {
            marker.line_color(_RGBA(0,0,0,255));
            marker.fill_color(_RGBA(0,0,0,128));

            if (draw_fn)
              (marker.*draw_fn)(p->m_cx + 3, HEIGHT - p->m_cy + 3, p->m_radius);
          }

          marker.line_color(p->m_outline);
          marker.fill_color(p->m_fill);

          if (draw_fn)
            (marker.*draw_fn)(p->m_cx, HEIGHT - p->m_cy, p->m_radius);
        }
      }
    }
  }
  
  // blit to main buffer
  {
    CShupMutexLock ml2(g_mut_render, INFINITE);
    assert(ml2.HaveLocked());
    CShupMutexLock ml(g_mut_emitter, INFINITE);
    assert(ml.HaveLocked());

    int time = GetTickCount();

    if (time > last_fps_log + 1000)
    {
      g_fps = g_frames_rendered;
      g_frames_rendered = 0;
      last_fps_log = time;
    }

    g_ui_renderer_base->clear(_RGBA(0,0,0,0));
    g_ui_renderer_base->copy_from(*g_rendering_buffer);

    if (g_menu_active)
      g_menu->Draw();

    if (g_render_stats)
    {
      sprintf(buf, "fps: %d - fireworks: %d of %d (%d particles each)", (int)g_fps, g_fireworks.GetSize(), MAX_FIREWORKS, MAX_PARTICLES);
      g_ui_text_render->SetFontSize(10);
      g_ui_text_render->SetColor(_RGBA(255,255,255,255));
      g_ui_text_render->SetText(buf, 6, 6);
    }

    if (g_render_about)
    {
      about();
    }

    if (!g_is_fullscreen)
    {
      const int lw = 4;
      static agg::rgba8 grey = _RGBA(128,128,128,255);
      static agg::rgba8 white = _RGBA(255,255,255,255);
      g_ui_renderer_base->copy_bar(0, 0, WIDTH - 1, lw, grey);
      g_ui_renderer_base->copy_bar(0, 0, lw, HEIGHT - 1, grey);
      g_ui_renderer_base->copy_bar(0, HEIGHT - 1 - lw, WIDTH - 1, HEIGHT - 1, grey);
      g_ui_renderer_base->copy_bar(WIDTH - lw - 1, 0, WIDTH - 1, HEIGHT - 1, grey);
      g_ui_renderer_base->copy_bar(0, 0, WIDTH - 1, lw / 2, white);
      g_ui_renderer_base->copy_bar(0, 0, lw / 2, HEIGHT, white);
      g_ui_renderer_base->copy_bar(0, HEIGHT - 1 - (lw / 2), WIDTH - 1, HEIGHT - 1, white);
      g_ui_renderer_base->copy_bar(WIDTH - 1 - (lw / 2), 0, WIDTH - 1, HEIGHT - 1, white);
    }

    SetDIBitsToDevice(g_renderdc, 0, 0, WIDTH, HEIGHT, 0, 0, 0, HEIGHT, (const void *)(unsigned int *)g_ui_buffer, (const BITMAPINFO *)(&g_bmi), DIB_RGB_COLORS);

    // blt the other dc into the dc used for painting the window
    BitBlt(g_memdc, 0, 0, WIDTH, HEIGHT, g_renderdc, 0, 0, SRCCOPY);
  }

  g_frames_rendered++;
  InvalidateRect(g_mainwnd, NULL, true);
  UpdateWindow(g_mainwnd);
}

LRESULT APIENTRY MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  static PAINTSTRUCT ps;
  static HDC hdc;
  static bool mouse_down = false;
  static POINT pt;
  static HCURSOR def_cursor = LoadCursor(NULL, MAKEINTRESOURCE(IDC_ARROW));

  switch (msg)
  {
  case WM_CREATE:
    SetWindowText(hwnd, "AGG DEMO");
    return(0);

  case WM_NCCREATE:
    return(true);

  case WM_ERASEBKGND:
    return(true);

  case WM_LBUTTONDBLCLK:
    onDoubleClick();
    return(0);

  case WM_LBUTTONUP:
    mouse_down = false;

    if (g_is_resizing)
      pause_demo(false);

    g_is_resizing = false;

    {
      CShupMutexLock ml(g_mut_emitter, INFINITE);
      assert(ml.HaveLocked());

      if (g_menu_active && g_menu->OnLMouseUp(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
        return(0);
    }

    SetCursor((wParam & MK_CONTROL) ? g_cur_open : def_cursor);
    return(0);

  case WM_LBUTTONDOWN:
    GetCursorPos(&pt);
    mouse_down = true;

    if (!(wParam & MK_CONTROL))
    {
      CShupMutexLock ml(g_mut_emitter, INFINITE);
      assert(ml.HaveLocked());

      if (g_menu_active && g_menu && g_menu->OnLMouseDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)))
        return(0);

      bool pause_it = !is_paused();

      if (pause_it) 
        pause_demo();

      add_firework(GET_X_LPARAM(lParam), HEIGHT - GET_Y_LPARAM(lParam), WIDTH, HEIGHT, g_base_color, g_base_shape);

      if (pause_it) 
        pause_demo(false);

      return(0);
    }
    
#if 0
    create_expanding_circle(pt.x, pt.y);
    if (pt.x >= (float)WIDTH * 0.9 && pt.y >= (float)HEIGHT * 0.9)
    {
      g_is_resizing = true;
      pause_demo(true);
    }
#endif // 0

    SetCursor(g_cur_grab);
    return(0);

  case WM_RBUTTONUP:
    {
      return(0);

      POINT pt;

      if (!GetCursorPos(&pt))
        return(0);

      int cmd = 0;
      HMENU menu = LoadMenu(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_MENU1));
      HMENU sub = GetSubMenu(menu, 0);

      CheckMenuItem(sub, ID_RIGHTCLICK_PAUSE, MF_BYCOMMAND | (g_is_paused ? MF_CHECKED : MF_UNCHECKED));
      CheckMenuItem(sub, ID_RIGHTCLICK_FULLSCREEN, MF_BYCOMMAND | (g_is_fullscreen ? MF_CHECKED : MF_UNCHECKED));
      CheckMenuItem(sub, ID_RIGHTCLICK_AOT, MF_BYCOMMAND | (g_is_aot ? MF_CHECKED : MF_UNCHECKED));
      CheckMenuItem(sub, ID_RIGHTCLICK_SHOW_STATS, MF_BYCOMMAND | (g_render_stats ? MF_CHECKED : MF_UNCHECKED));
      CheckMenuItem(sub, ID_RIGHTCLICK_SHADOWS, MF_BYCOMMAND | (g_shadows ? MF_CHECKED : MF_UNCHECKED));
      CheckMenuItem(sub, ID_RIGHTCLICK_SHOWMENU, MF_BYCOMMAND | (g_menu_active ? MF_CHECKED : MF_UNCHECKED));

      /*
      for (int i = -1; i < Firework::total_colors() - 1; i++)
        CheckMenuItem(sub, i + COLORS_RESOURCE_FLOOR + 1, MF_BYCOMMAND | ((BRUSH_COLOR)i == g_base_color ? MF_CHECKED : MF_UNCHECKED));

      for (int i = -1; i < Firework::total_shapes() - 1; i++)
        CheckMenuItem(sub, i + SHAPES_RESOURCE_FLOOR + 1, MF_BYCOMMAND | ((BRUSH_SHAPE)i == g_base_shape ? MF_CHECKED : MF_UNCHECKED));

      HWND shup_hwnd = NULL;
      shup_hwnd = FindWindow("ShupHiddenWindow", "ShupHiddenWindow");
      EnableMenuItem(sub, ID_RIGHTCLICK_SHUPLOAD, shup_hwnd != NULL ? MF_ENABLED : (MF_DISABLED | MF_GRAYED));

      for (int i = 0; i < num_sizes; i++)
      {
        int size_id = size_ids[i];
        int radius = size_radii[i];
        bool checked = radius == g_max_radius;
        CheckMenuItem(sub, size_id, MF_BYCOMMAND | (checked ? MF_CHECKED : MF_UNCHECKED));
      }

      double mult = 255.0f / 10.f;

      for (int i = 0; i <= 10; i++)
      {
        int fixed_value = floor((double)i * mult);
        CheckMenuItem(sub, i + ID_OPACITY_RANDOM, MF_BYCOMMAND | (fixed_value == g_opacity ? MF_CHECKED : MF_UNCHECKED));
      }
      */

      if (!(cmd = TrackPopupMenu(sub, TPM_NONOTIFY | TPM_LEFTALIGN | TPM_TOPALIGN | TPM_RETURNCMD, pt.x, pt.y, NULL, hwnd, NULL)))
      {
        DestroyMenu(sub);
        DestroyMenu(menu);
        return(0);
      }

      /*
      // colors
      if (cmd >= COLORS_RESOURCE_FLOOR && cmd <= COLORS_RESOURCE_FLOOR + Firework::total_colors())
      {
        g_base_color = (BRUSH_COLOR)(cmd - COLORS_RESOURCE_FLOOR - 1);
      }
      // shapes
      else if (cmd >= SHAPES_RESOURCE_FLOOR && cmd <= SHAPES_RESOURCE_FLOOR + Firework::total_shapes())
      {
        g_base_shape = (BRUSH_SHAPE)(cmd - SHAPES_RESOURCE_FLOOR - 1);
      }
      else if (cmd >= ID_OPACITY_RANDOM && cmd <= ID_OPACITY_RANDOM + 10)
      {
        g_opacity = (double)(cmd - ID_OPACITY_RANDOM) * mult;
      }
      else if (cmd >= ID_SIZES_RANDOM && cmd <= ID_SIZES_RANDOM + num_sizes)
      {
        g_max_radius = size_radii[cmd - ID_SIZES_RANDOM];
      }
      else
      */
      {
        switch (cmd)
        {
        case ID_RIGHTCLICK_EXIT:
          SendMessage(hwnd, WM_CLOSE, 0, 0);
         break;

        case ID_RIGHTCLICK_PAUSE:
          pause_demo(!g_is_paused);
        break;

        case ID_RIGHTCLICK_SHOW_STATS:
          render_stats(!g_render_stats);
        break;

        case ID_RIGHTCLICK_FULLSCREEN:
          onDoubleClick();
        break;

        case ID_RIGHTCLICK_AOT:
          SetAlwaysOnTop(hwnd, !g_is_aot);
        break;

        case ID_RIGHTCLICK_CLEAR:
          init_kuler();
          reset_fireworks();
        break;

        case ID_RIGHTCLICK_ABOUT:
          about();
        break;

        case ID_RIGHTCLICK_SHADOWS:
          g_shadows = !g_shadows;
          break;

        case ID_RIGHTCLICK_SAVEAS:
          saveas();
          break;

        case ID_RIGHTCLICK_SHUPLOAD:
          shupload();
          break;

        case ID_RIGHTCLICK_SHOWMENU:
          g_menu_active = !g_menu_active;
          break;
        }
      }

      DestroyMenu(sub);
      DestroyMenu(menu);
    }
    return(0);

  case WM_MOUSEMOVE:
    if (g_menu_active && g_menu)
    {
      int x = GET_X_LPARAM(lParam), y = GET_Y_LPARAM(lParam);
      if (g_menu->OnMouseMove(x, y))
      {
        SetCursor(g_cur_pointer);
        return(0);
      }
    }
    
    if (!mouse_down)
    {
      SetCursor((wParam & MK_CONTROL) ? g_cur_open : def_cursor);
      return(0);
    }

    if (wParam & MK_CONTROL)
    {
      SetCursor(g_cur_grab);
      POINT cpt;
      RECT rect;
      GetWindowRect(hwnd, &rect);
      GetCursorPos(&cpt);

      if (false && g_is_resizing)
      {
        //WIDTH = WIDTH + (cpt.x - pt.x);
        //HEIGHT = HEIGHT + (cpt.y - pt.y);
        //MoveWindow(hwnd, rect.left, rect.top, WIDTH, HEIGHT, true);
      }
      else
        MoveWindow(hwnd, rect.left + (cpt.x - pt.x), rect.top + (cpt.y - pt.y), WIDTH, HEIGHT, true);

      pt.x = cpt.x;
      pt.y = cpt.y;
      return(0);
    }

    {
      CShupMutexLock ml(g_mut_emitter, INFINITE);
      assert(ml.HaveLocked());

      Firework *f = NULL;
      f = g_fireworks.Get(g_fireworks.GetSize() - 1);

      if (!f)
        return(0);

      int x = GET_X_LPARAM(lParam), y = HEIGHT - GET_Y_LPARAM(lParam);
      int last_dist = sqrt(pow((double)x - (double)f->m_cx, 2) + pow((double)y - (double)f->m_cy, 2));

      if (last_dist > f->m_source_radius)
      {
        bool pause_it = !is_paused();

        if (pause_it) pause_demo();

        add_firework(x, y, WIDTH, HEIGHT, g_base_color, g_base_shape);

        if (pause_it) pause_demo(false);
      }
    }

    return(0);

  case WM_QUIT:
  case WM_DESTROY:
  case WM_CLOSE:
    g_killswitch = true;

    if (g_is_paused)
    {
      pause_demo(false);
    }
    //write_ppm(g_buffer, WIDTH, HEIGHT, "c:\\lol.ppm");
    //WaitForSingleObject(g_render_done, INFINITE);
    DestroyWindow(hwnd);
    PostQuitMessage(0);
    return(0);

  case WM_PAINT:
    hdc = BeginPaint(hwnd, &ps);
    {
      CShupMutexLock ml2(g_mut_render, INFINITE);
      assert(ml2.HaveLocked());
      BitBlt(g_screendc, 0, 0, WIDTH, HEIGHT, g_memdc, 0, 0, SRCCOPY);
    }
    EndPaint(hwnd, &ps);
    return(0);

  case WM_KEYDOWN:
    switch (wParam)
    {
    case VK_CONTROL:
      SetCursor(mouse_down ? g_cur_grab : g_cur_open);
      return(0);
    }
    break;

  case WM_KEYUP:
    switch (wParam)
    {
    case VK_ESCAPE:
      SendMessage(hwnd, WM_CLOSE, 0, 0);
      return(0);

    case VK_SPACE:
      pause_demo(!g_is_paused);
      return(0);

    case VK_CONTROL:
      SetCursor(def_cursor);
      return(0);

    case VK_F1:
      g_render_stats = !g_render_stats;
      return(0);

    case 'M':
      g_menu_active = !g_menu_active;
      return(0);
    }
  break;
  }

  return(DefWindowProc(hwnd, msg, wParam, lParam));
}

bool write_ppm(const IMAGE_BUFFER_TYPE* buf, 
               unsigned width, 
               unsigned height, 
               const char* file_name)
{
  FILE* fd = fopen(file_name, "wb");
  if(fd)
  {
    fprintf(fd, "P6 %d %d 255 ", width, height);
    fwrite(buf, 1, width * height * 3, fd);
    fclose(fd);
    return true;
  }
  return false;
}


void pause_demo(bool paused)
{
  if (g_is_paused == paused)
    return;

  g_is_paused = paused;

  if (paused)
  {
    //g_paused = CreateEvent(0, false, false, 0);
  }
  else
  {
    //SetEvent(g_paused);
    g_paused = NULL;
  }
}

void render_stats(bool do_render)
{
  g_render_stats = do_render;
}

bool is_paused()
{
  return (g_is_paused);
}

void onDoubleClick()
{
  pause_demo(true);
  Sleep(500);
  RECT rect;
  GetWindowRect(g_mainwnd, &rect);
  deinit_image();

  g_is_fullscreen = !g_is_fullscreen;
  
  unsigned int oldbytes = WIDTH * HEIGHT * BPP;

  if (!g_is_fullscreen)
  {
    WIDTH = 1024;
    HEIGHT = 768;
    rect.left = g_last_x;
    rect.top = g_last_y;
    g_last_x = g_last_y = 0;
  }
  else
  {
    int sx = GetSystemMetrics(SM_CXSCREEN);
    int sy = GetSystemMetrics(SM_CYSCREEN);
    WIDTH = sx;
    HEIGHT = sy;
    g_last_x = rect.left;
    g_last_y = rect.top;
    rect.left = 0;
    rect.top = 0;
  }

//  int ne = g_emitters.GetSize();
  //double unit = ((double)HEIGHT / ((double)ne));
  //unit += unit * 0.5f;

  reset_fireworks();
  

  /*for (int i = 0; i < ne; i++)
  {
    Emitter *em = g_emitters.Get(i);
    em->set_window(WIDTH, HEIGHT);
    double yoff = (unit * (double)(i + 1));
    em->set_offset(0, yoff);
  }
  */

  init_image(WIDTH, HEIGHT, rect.left, rect.top);
  pause_demo(false);
}

void init_image(int width, int height, int x, int y)
{
  WIDTH = width;
  HEIGHT = height;
  unsigned int newbytes = WIDTH * HEIGHT * BPP;
  IMAGE_BUFFER_TYPE *buf = NULL;
  g_buffer = new IMAGE_BUFFER_TYPE[newbytes];
  g_ui_buffer = new IMAGE_BUFFER_TYPE[newbytes];
  memset(g_buffer, 0, newbytes);
  memset(g_ui_buffer, 0, newbytes);
  g_rendering_buffer = new agg::rendering_buffer(g_buffer, WIDTH, HEIGHT, -WIDTH * BPP);
  g_ui_rendering_buffer = new agg::rendering_buffer(g_ui_buffer, WIDTH, HEIGHT, -WIDTH * BPP);
  g_pixfmt = new pixfmt_type(*g_rendering_buffer);
  g_ui_pixfmt = new pixfmt_type(*g_ui_rendering_buffer);
  g_renderer_base = new renbase_type(*g_pixfmt);
  g_ui_renderer_base = new renbase_type(*g_ui_pixfmt);
  g_renderer_base->clear(g_bg_color);
  g_ui_renderer_base->clear(_RGBA(0, 0, 0, 0));
  MoveWindow(g_mainwnd, x, y, WIDTH, HEIGHT, true);
  g_screendc = GetDC(g_mainwnd);
  g_memdc = CreateCompatibleDC(g_screendc);
  g_membmp = CreateCompatibleBitmap(g_screendc, WIDTH, HEIGHT);
  SelectObject(g_memdc, g_membmp);
  g_renderdc = CreateCompatibleDC(g_screendc);
  g_renderbmp = CreateCompatibleBitmap(g_screendc, WIDTH, HEIGHT);
  SelectObject(g_renderdc, g_renderbmp);

  g_ui_text_render = new TextRenderer(g_ui_buffer, WIDTH, HEIGHT);

  memset(&g_bmi.bmiHeader, 0, sizeof(BITMAPINFOHEADER));
  g_bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
  g_bmi.bmiHeader.biWidth = WIDTH;
  g_bmi.bmiHeader.biHeight = (int)HEIGHT;
  g_bmi.bmiHeader.biPlanes = 1;
  g_bmi.bmiHeader.biBitCount = BPP * 8;
  g_bmi.bmiHeader.biCompression = BI_RGB;
  g_bmi.bmiHeader.biSizeImage = 0;
  g_bmi.bmiHeader.biXPelsPerMeter = 1;
  g_bmi.bmiHeader.biYPelsPerMeter = 1;
  g_bmi.bmiHeader.biClrUsed = 0;
  g_bmi.bmiHeader.biClrImportant = 0;

  init_menu();
}

void deinit_image()
{
  delete g_renderer_base;
  delete g_ui_renderer_base;
  delete g_pixfmt;
  delete g_ui_pixfmt;
  delete g_rendering_buffer;
  delete g_ui_rendering_buffer;
  delete [] g_buffer;
  delete [] g_ui_buffer;
  delete g_ui_text_render;
  SelectObject(g_renderdc, NULL);
  DeleteBitmap(g_renderbmp);
  SelectObject(g_memdc, NULL);
  DeleteBitmap(g_membmp);
  DeleteDC(g_memdc);
  DeleteDC(g_renderdc);
  deinit_menu();
}

void create_expanding_circle(int x, int y)
{

}

void SetAlwaysOnTop(HWND hwnd, int aot)
{
  RECT rect;
  g_is_aot = (bool)aot;
  GetWindowRect(hwnd, &rect);
  SetWindowPos(hwnd, aot ? HWND_TOPMOST : HWND_NOTOPMOST, rect.left, rect.top, rect.right - rect.left, rect.bottom - rect.top, SWP_NOACTIVATE | SWP_NOREPOSITION | SWP_NOSIZE);
}

void delete_firework(Firework *f)
{
  for (int i = 0; i < f->m_particles.GetSize(); i++)
  {
    Particle *p = f->m_particles.Get(i);

    if (p->m_constraint)
    {
      cpSpaceRemoveConstraint(g_space, p->m_constraint);
      cpConstraintFree(p->m_constraint);
      p->m_constraint = NULL;
    }

    cpSpaceRemoveBody(g_space, p->m_body);
    cpSpaceRemoveShape(g_space, p->m_shape);
  }

  delete f;
}

void reset_fireworks()
{
  CShupMutexLock ml(g_mut_emitter, INFINITE);
  assert(ml.HaveLocked());

  g_just_reset = true;

  while (g_fireworks.GetSize() > 0)
  {
    Firework *f = g_fireworks.Get(0);
    delete_firework(f);
    g_fireworks.Delete(0);
  }
}

void add_firework(int x, int y, int w, int h, BRUSH_COLOR color, BRUSH_SHAPE shape)
{
  while (g_fireworks.GetSize() >= MAX_FIREWORKS)
  {
    Firework *f = g_fireworks.Get(0);
    delete_firework(f);
    g_fireworks.Delete(0);
  }

  int opacity = g_opacity == 0 ? (1 + (genrand64_int64() % 255)) : g_opacity;
  int radius = g_max_radius <= 0 ? (1 + (genrand64_int64() % 25)) : g_max_radius;
  Firework *f = new Firework(x, y, w, h, color, shape, opacity, radius);
  int num_particles = f->m_particles.GetSize();

  for (int i = 0; i < num_particles; i++)
  {
    Particle *p = f->m_particles.Get(i);
    p->m_body->data = (void *)p;
    p->m_shape->data = (void *)p;
    cpSpaceAddBody(g_space, p->m_body);
    cpSpaceAddShape(g_space, p->m_shape);
    continue;

    if (i > 0) // i % 2 == 0 && i < num_particles - 1
    {
      Particle *last = f->m_particles.Get(i - 1);
      last->m_constraint = cpSlideJointNew(last->m_body, p->m_body, cpv(0, 0), cpv(0, 0), 5.0f, 20.0f); // cpPinJointNew(last->m_body, p->m_body, cpv(0,0), cpv(0,0));
      cpSpaceAddConstraint(g_space, last->m_constraint);
    }
  }

  g_fireworks.Insert(genrand64_int64() % 2 == 0 ? 0 : g_fireworks.GetSize(), f);
}

void about()
{
  //MessageBox(g_mainwnd, APP_LABEL, "About", MB_OK);
  g_ui_text_render->SetFontSize(24);
  g_ui_text_render->SetColor(_RGBA(255,255,255,255));
  g_ui_text_render->SetText(APP_LABEL, 200, 300);
  int y = 300 + (g_ui_text_render->GetCharacterHeight() * 2);
  g_ui_text_render->SetFontSize(12);
  g_ui_text_render->SetText("http://antimac.org/explosivo", 200, y);
}

void buffer_to_png(int *buffer, int width, int height, const char *path)
{
  savePNG(path, buffer, width, height, 0);
}

void shupload()
{
  pause_demo(true);

  CShupMutexLock ml(g_mut_emitter, INFINITE);
  assert(ml.HaveLocked());

  char buf[1024], buf2[1024];
  memset(&buf, NULL, 1024);
  GetTempPath(1023, buf);
  sprintf(buf2, "explosivo-%d.png", time(0));
  strcat(buf, buf2);
  strcat(buf, "\0");
  buffer_to_png((int *)g_buffer, WIDTH, HEIGHT, buf);
  pause_demo(false);
  HWND hwnd = NULL;
  hwnd = FindWindow("ShupHiddenWindow", "ShupHiddenWindow");

  if (hwnd)
  {
    COPYDATASTRUCT cds;
    cds.dwData = 1;
    cds.cbData = strlen(buf);
    cds.lpData = buf;
    SendMessage(hwnd, WM_COPYDATA, (WPARAM)g_mainwnd, (LPARAM)&cds);
  }
  else
    MessageBox(g_mainwnd, "Shup isn't running.", "Error!", MB_OK | MB_ICONERROR);
}

void saveas()
{

}

void particleCollisionHandler(cpArbiter *arbiter, cpSpace *space, void *data)
{
  return;
  CP_ARBITER_GET_SHAPES(arbiter, a, b);
  Particle *pa = (Particle *)a->data, *pb = (Particle *)b->data;

  if (pa->m_parent == pb->m_parent)
    return;

  if (!pa->m_constraint)
    delayed_add_firework(pa, pb);
  else if (!pb->m_constraint)
    delayed_add_firework(pb, pa);

  //delayed_add_firework(pa, pb);
}

void delayed_add_firework(Particle *pa, Particle *pb)
{
  ParticlePair *p = new ParticlePair();
  p->a = pa;
  p->b = pb;
  g_delayed_fireworks.Insert(0, p);
}

void init_kuler()
{
  const char *delimiter = "\r\n";
  const int delimiter_len = strlen(delimiter);
  std::string color_data = GetUrl::Gimme("http://antimac.org/etc/kuler.php");
  const char *base_char = color_data.c_str();

  if (!base_char || !strcmp(base_char, ""))
    return;

  int idx = 0;
  int i = -1;
  int r, g, b;
  int array_idx = 0;
  const char *res = base_char;

  while (res && array_idx < 5)
  {
    sscanf(res, "%d,%d,%d\r\n", &r, &g, &b);
    g_kulers[array_idx++] = _RGB(r, g, b);
    idx = delimiter_len + (strstr(res, delimiter) - res);
    res = strstr(res + idx, delimiter);
    idx = idx;
  }
}

void init_menu()
{
  if (g_menu)
  {
    deinit_menu();
  }

  char buf[128];
  memset(buf, 0, 128);

  g_menu = new Menu(WIDTH, HEIGHT, g_ui_buffer, g_ui_rendering_buffer, g_ui_renderer_base, g_ui_pixfmt);
  g_menu->SetTextRenderer(g_ui_text_render);
  g_menu->SetPos(6, 50);

  // COLORS MENU
  PopupMenuItem *colors = (PopupMenuItem *)(g_menu->AddPopupItem("Color", 0, popup_callback));
  Menu *colors_menu = (Menu *)(colors->GetPopup());
  colors_menu->SetParent(g_menu);
  colors_menu->AddItem("<< Back", 0, back_callback)->SetIsCheckable(false);

  for (int i = 0; i < num_colors; i++)
  {
    MenuItem *item = colors_menu->AddItem(color_titles[i], i - 1, change_color_callback);

    if ((BRUSH_COLOR)(i - 1) == g_base_color)
      colors_menu->CheckItem(item);
  }

  double mult = 255.0f / 10.f;

  // OPACITY MENU
  PopupMenuItem *opacitys = (PopupMenuItem *)(g_menu->AddPopupItem("Opacity", 0, popup_callback));
  Menu *opacitys_menu = (Menu *)(opacitys->GetPopup());
  opacitys_menu->SetParent(g_menu);
  opacitys_menu->AddItem("<< Back", 0, back_callback)->SetIsCheckable(false);
  MenuItem *item = opacitys_menu->AddItem("Random", 0, change_opacity_callback);
  
  if (g_opacity == 0)
    opacitys_menu->CheckItem(item);

  for (int i = 1; i < 11; i++)
  {
    sprintf(buf, "%d%%", i * 10);
    item = opacitys_menu->AddItem(buf, i, change_opacity_callback);
    
    if (i * mult == g_opacity)
      opacitys_menu->CheckItem(item);
  }

  // SHAPES MENU
  PopupMenuItem *shapes = (PopupMenuItem *)(g_menu->AddPopupItem("Shapes", 0, popup_callback));
  Menu *shapes_menu = (Menu *)(shapes->GetPopup());
  shapes_menu->SetParent(g_menu);
  shapes_menu->AddItem("<< Back", 0, back_callback)->SetIsCheckable(false);
  //item = shapes_menu->AddItem("Random", -1, change_shape_callback);

  for (int i = 0; i < num_shapes; i++)
  {
    item = shapes_menu->AddItem(shape_titles[i], i - 1, change_shape_callback);
    //item->SetFontSize(9);

    if ((BRUSH_SHAPE)(i - 1) == g_base_shape)
      shapes_menu->CheckItem(item);
  }

  // SIZES MENU
  PopupMenuItem *sizes = (PopupMenuItem *)(g_menu->AddPopupItem("Size", 0, popup_callback));
  Menu *sizes_menu = (Menu *)(sizes->GetPopup());
  sizes_menu->SetParent(g_menu);
  sizes_menu->AddItem("<< Back", 0, back_callback)->SetIsCheckable(false);

  for (int i = 0; i < num_sizes; i++)
  {
    int size_id = size_ids[i];
    int radius = size_radii[i];
    bool checked = radius == g_max_radius;
    sprintf(buf, "%d", radius);
    item = sizes_menu->AddItem(radius <= 0 ? "Random" : buf, i, change_size_callback);

    if (g_max_radius == radius)
      sizes_menu->CheckItem(item);
  }

  // FADE MENU
  PopupMenuItem *fades = (PopupMenuItem *)(g_menu->AddPopupItem("Fade", 0, popup_callback));
  Menu *fades_menu = (Menu *)(fades->GetPopup());
  fades_menu->SetParent(g_menu);
  fades_menu->AddItem("<< Back", 0, back_callback)->SetIsCheckable(false);

  for (int i = 0; i < num_fades; i++)
  {
    int id = fade_values[i];
    const char *title = fade_titles[i];
    bool checked = id == g_fade;
    item = fades_menu->AddItem(title, id, change_fade_callback);

    if (checked)
      fades_menu->CheckItem(item);
  }

  item = g_menu->AddItem("Show FPS", 0, main_menu_root_callback);
  item->SetIsStickyCheck(true);
  item->SetIsChecked(g_render_stats);

  item = g_menu->AddItem("Fullscreen", 2, main_menu_root_callback);
  item->SetIsStickyCheck(true);
  item->SetIsChecked(g_is_fullscreen);

  item = g_menu->AddItem("About", 4, main_menu_root_callback);
  item->SetIsStickyCheck(true);
  item->SetIsChecked(g_render_about);

  item = g_menu->AddItem("Clear Image", 1, main_menu_root_callback);
  item->SetIsCheckable(false);

  HWND hwnd = NULL;
  hwnd = FindWindow("ShupHiddenWindow", "ShupHiddenWindow");

  if (hwnd)
  {
    item = g_menu->AddItem("Shupload", 3, main_menu_root_callback);
    item->SetIsCheckable(false);
  }

  item = g_menu->AddItem("Exit", 5, main_menu_root_callback);
  item->SetIsCheckable(false);
}

void deinit_menu()
{
  delete g_menu;
  g_menu = NULL;
}

Menu *active_menu()
{
  return(g_menu);
}

void set_active_menu(Menu *menu)
{
  g_menu = menu;

  if (!g_menu)
    g_menu_active = false;
}

void popup_callback(MenuItem *item, int x, int y)
{
  Menu *menu = (Menu *)(((PopupMenuItem *)item)->GetPopup());

  if (menu)
    set_active_menu(menu);
}

void back_callback(MenuItem *item, int x, int y)
{
  Menu *item_p = NULL;
  item_p = (Menu *)(item->GetParent());
  Menu *menu_p = NULL;
  menu_p = (Menu *)(item_p->GetParent());

  if (menu_p)
    set_active_menu(menu_p);
}

void change_opacity_callback(MenuItem *item, int x, int y)
{
  double mult = 255.0f / 10.f;

  g_opacity = (double)(item->GetId()) * mult;
}

void change_color_callback(MenuItem *item, int x, int y)
{
  g_base_color = (BRUSH_COLOR)(item->GetId());
}

void change_size_callback(MenuItem *item, int x, int y)
{
  g_max_radius = size_radii[item->GetId()];
}

void change_shape_callback(MenuItem *item, int x, int y)
{
  g_base_shape = (BRUSH_SHAPE)(item->GetId());
}

void change_fade_callback(MenuItem *item, int x, int y)
{
  g_fade = item->GetId();
}

void main_menu_root_callback(MenuItem *item, int x, int y)
{
  CShupMutexLock ml(g_mut_emitter, INFINITE);
  assert(ml.HaveLocked());

  switch (item->GetId())
  {
  case 0:
    render_stats(!g_render_stats);
    break;

  case 1:
    init_kuler();
    reset_fireworks();
    break;

  case 2:
    g_scheduled_reset = true;
    break;

  case 3:
    shupload();
    break;

  case 4:
    g_render_about = !g_render_about;
    break;

  case 5:
    SendMessage(g_mainwnd, WM_CLOSE, 0, 0);
    break;
  }
}