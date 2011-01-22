#ifndef _DEMOS_DEMOS_H
#define _DEMOS_DEMOS_H

#include "Firework.h"
#include "Menu.h"

#define COLORS_RESOURCE_FLOOR 5000
#define SHAPES_RESOURCE_FLOOR 6000

inline int FPS(int width)
{
  return(1000);
}

inline int AGE_INCREMENT(int width)
{
  return((int)(((double)width / 800.0f) * 2.0f));
}

int WINAPI WinMain(__in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in_opt LPSTR lpCmdLine, __in int nShowCmd);
static DWORD WINAPI process_thread(void* arg);
void process();
static DWORD WINAPI render_thread(void* arg);
void render();
LRESULT APIENTRY MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
bool write_ppm(const IMAGE_BUFFER_TYPE* buf, unsigned width, unsigned height, const char* file_name);
void pause_demo(bool paused = true);
void render_stats(bool do_render = true);
bool is_paused();
void onDoubleClick();
void init_image(int width, int height, int x = 0, int y = 0);
void deinit_image();
void create_expanding_circle(int x, int y);
void SetAlwaysOnTop(HWND hwnd, int aot);
void delete_firework(Firework *f);
void reset_fireworks();
void add_firework(int x, int y, int w, int h, BRUSH_COLOR color, BRUSH_SHAPE shape);
void about();
void buffer_to_png(int *buffer, int width, int height, const char *path);
void shupload();
void saveas();
void particleCollisionHandler(cpArbiter *arbiter, cpSpace *space, void *data);
void delayed_add_firework(Particle *pa, Particle *pb);
void init_kuler();
void init_menu();
void deinit_menu();
Menu *active_menu();
void set_active_menu(Menu *menu);
void popup_callback(MenuItem *item, int x, int y);
void back_callback(MenuItem *item, int x, int y);
void change_opacity_callback(MenuItem *item, int x, int y);
void change_color_callback(MenuItem *item, int x, int y);
void change_size_callback(MenuItem *item, int x, int y);
void change_shape_callback(MenuItem *item, int x, int y);
void change_fade_callback(MenuItem *item, int x, int y);
void main_menu_root_callback(MenuItem *item, int x, int y);

#endif // _DEMOS_DEMOS_H