#ifndef _SHUP_PNG_H
#define _SHUP_PNG_H

#include "../../pnglib/png.h"
#include "./memblock.h"

int savePNG(const char *filename, int *bits, int w, int h, int wantalpha);

#endif // _SHUP_PNG_H