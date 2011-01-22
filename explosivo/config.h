#ifndef _DEMOS_CONFIG_H
#define _DEMOS_CONFIG_H

#define PI 3.141592653589793f
//#define AGE_INCREMENT 2.0f
#define AMP_MOD 0.007f
#define AMP_HEIGHT 6
#define THETA (PI / AMP_HEIGHT)
#define STREAMLETS_PER_EMITTER 400
#define FRAMES_BETWEEN_STREAMLETS 64
//#define FPS ((800.0f / (float)WIDTH) * 60.0f)
//#define WIDTH 800
//#define HEIGHT 600
#define BPP 4
#define IMAGE_BUFFER_TYPE unsigned char

#include "agg_basics.h"
#include "agg_scanline_u.h"
#include "agg_rendering_buffer.h"
#include "agg_rasterizer_scanline_aa.h"
#include "agg_scanline_u.h"
#include "agg_scanline_p.h"
#include "agg_renderer_scanline.h"
#include "agg_image_accessors.h"
#include "agg_pixfmt_rgb.h"
#include "agg_pixfmt_rgba.h"

#include "agg_gamma_lut.h"
#include "agg_span_allocator.h"
#include "agg_span_image_filter_rgba.h"
#include "agg_span_interpolator_linear.h"
#include "agg_image_accessors.h"

#include "agg_ellipse.h"
#include "agg_rounded_rect.h"
#include "agg_conv_stroke.h"
#include "agg_conv_dash.h"
#include "agg_conv_curve.h"
#include "agg_conv_contour.h"
#include "agg_conv_smooth_poly1.h"
#include "agg_conv_marker.h"
#include "agg_arrowhead.h"
#include "agg_path_storage.h"
#include "agg_vcgen_markers_term.h"

#include "platform/agg_platform_support.h"
#include "agg_renderer_primitives.h"
#include "agg_renderer_markers.h"
#include "agg_conv_transform.h"
#include "agg_trans_affine.h"
#include "agg_trans_warp_magnifier.h"
#include "agg_conv_segmentator.h"

#include "agg_span_image_filter_rgb.h"

#define span_image_filter          span_image_filter_rgb
#define span_image_filter_nn       span_image_filter_rgb_nn
#define span_image_filter_bilinear span_image_filter_rgb_bilinear_clip
#define span_image_filter_2x2      span_image_filter_rgb_2x2
#define AGG_ACCURATE_TIME  
#define pix_format agg::pix_format_bgra32

#define _RGB(r,g,b) agg::rgba8(r,g,b,255)
#define _RGBA(r,g,b,a) agg::rgba8(r,g,b,a)

typedef agg::pixfmt_bgra32 pixfmt_type;
typedef agg::pixfmt_rgb24_pre pixfmt_pre_type;
typedef agg::renderer_base<pixfmt_type> renbase_type;
typedef agg::renderer_base<pixfmt_pre_type> renbase_pre_type;
typedef agg::renderer_scanline_aa_solid<renbase_type> renderer_solid;
typedef pixfmt_type::color_type color_type;
typedef agg::row_accessor<pixfmt_type> row_type;

#include "ft2build.h"
#include FT_FREETYPE_H

#endif // _DEMOS_CONFIG_H