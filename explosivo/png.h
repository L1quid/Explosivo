#ifndef _SHUP_PNG_H
#define _SHUP_PNG_H

#define png_infopp_NULL (png_infopp)NULL
#define int_p_NULL (int*)NULL

#include "../libpng/png.h"
#include "./memblock.h"

int savePNG(const char *filename, int *bits, int w, int h, int wantalpha);

#endif // _SHUP_PNG_H