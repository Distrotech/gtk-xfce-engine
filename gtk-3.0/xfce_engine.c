/*  $Id$
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 *
 *  Copyright (C) 1999-2004 Olivier Fourdan (fourdan@xfce.org)
 *  Copyright (C) 2011 Peter de Ridder (peter@xfce.org)
 *
 *  Portions based Thinice port by
 *                       Tim Gerla <timg@rrv.net>,
 *                       Tomas Ögren <stric@ing.umu.se,
 *                       Richard Hult <rhult@codefactory.se>
 *  Portions based on Smooth theme by
 *                       Andrew Johnson <ajgenius@ajgenius.us>
 *  Portions based on IceGradient theme by
 *                       Tim Gerla <timg@means.net>
 *                       Tomas Ögren <stric@ing.umu.se>
 *                       JM Perez <jose.perez@upcnet.es>
 *  Portions based on Wonderland theme by
 *                       Garrett LeSage
 *                       Alexander Larsson
 *                       Owen Taylor <otaylor@redhat.com>
 *  Portions based on Raleigh theme by
 *                       Owen Taylor <otaylor@redhat.com>
 *  Portions based on Notif theme
 *  Portions based on Notif2 theme
 *  Portions based on original GTK theme
 */

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <gtk/gtk.h>

#include <math.h>

#include "xfce_style_types.h"
#include "xfce_typebuiltin.h"
#include "xfce_engine.h"

/* css properties */
#define XFCE_NAMESPACE "xfce"
#define SMOOTH_EDGE "smooth-edge"
#define XFCE_SMOOTH_EDGE "-"XFCE_NAMESPACE"-"SMOOTH_EDGE
#define GRIP_STYLE "grip-style"
#define XFCE_GRIP_STYLE "-"XFCE_NAMESPACE"-"GRIP_STYLE

/* macros to make sure that things are sane ... */
#define GE_CAIRO_INIT                               \
    cairo_set_line_width (cr, 1.0);                 \
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE); \
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);

G_DEFINE_DYNAMIC_TYPE(XfceEngine, xfce_engine, GTK_TYPE_THEMING_ENGINE)

/* Taken from raleigh theme engine */
typedef enum
{
    CHECK_LIGHT,
    CHECK_DARK,
    CHECK_BASE,
    CHECK_TEXT,
    CHECK_CROSS,
    CHECK_DASH,
    RADIO_LIGHT,
    RADIO_DARK,
    RADIO_BASE,
    RADIO_TEXT
}
Part;

#define PART_SIZE 13

static const guint32 check_light_bits[] = {
    0x0000, 0x0000, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800,
    0x0800, 0x0800, 0x0ffc, 0x0000,
};
static const guint32 check_dark_bits[] = {
    0x0000, 0x0ffe, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
    0x0002, 0x0002, 0x0002, 0x0000,
};
static const guint32 check_base_bits[] = {
    0x0000, 0x0000, 0x07fc, 0x07fc, 0x07fc, 0x07fc, 0x07fc, 0x07fc, 0x07fc,
    0x07fc, 0x07fc, 0x0000, 0x0000,
};
static const guint32 check_text_bits[] = {
    0x0000, 0x0000, 0x1c00, 0x0f00, 0x0380, 0x01c0, 0x00e0, 0x0073, 0x003f,
    0x003e, 0x001c, 0x0018, 0x0008
};
static const guint32 check_cross_bits[] = {
    0x0000, 0x0000, 0x0000, 0x0300, 0x0380, 0x01d8, 0x00f8, 0x0078, 0x0038,
    0x0018, 0x0000, 0x0000, 0x0000,
};
static const guint32 check_dash_bits[] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x0000, 0x03f8, 0x03f8, 0x03f8, 0x0000,
    0x0000, 0x0000, 0x0000, 0x0000,
};
static const guint32 radio_light_bits[] = {
    0x0000, 0x0000, 0x0000, 0x0400, 0x0800, 0x0800, 0x0800, 0x0800, 0x0800,
    0x0400, 0x0208, 0x01f0, 0x0000,
};
static const guint32 radio_dark_bits[] = {
    0x0000, 0x01f0, 0x0208, 0x0004, 0x0002, 0x0002, 0x0002, 0x0002, 0x0002,
    0x0004, 0x0000, 0x0000, 0x0000,
};
static const guint32 radio_base_bits[] = {
    0x0000, 0x0000, 0x01f0, 0x03f8, 0x07fc, 0x07fc, 0x07fc, 0x07fc, 0x07fc,
    0x03f8, 0x01f0, 0x0000, 0x0000,
};
static const guint32 radio_text_bits[] = {
    0x0000, 0x0000, 0x0000, 0x0000, 0x00e0, 0x01f0, 0x01f0, 0x01f0, 0x00e0,
    0x0000, 0x0000, 0x0000, 0x0000,
};

static struct
{
    const guint32 *bits;
    cairo_surface_t *bmap;
}
parts[] =
{
    { check_light_bits, NULL },
    { check_dark_bits,  NULL },
    { check_base_bits,  NULL },
    { check_text_bits,  NULL },
    { check_cross_bits, NULL },
    { check_dash_bits,  NULL },
    { radio_light_bits, NULL },
    { radio_dark_bits,  NULL },
    { radio_base_bits,  NULL },
    { radio_text_bits,  NULL }
};

/* internal functions */
static void xfce_draw_grips(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height, GtkOrientation orientation);

static void render_line(GtkThemingEngine * engine, cairo_t * cr, gdouble x1, gdouble y1, gdouble x2, gdouble y2);
static void render_frame(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height);

static void render_check(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height);
static void render_option(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height);
static void render_frame_gap(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height, GtkPositionType gap_side, gdouble gap_s, gdouble gap_e);
static void render_extension(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height, GtkPositionType gap_side);
static void render_slider(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height, GtkOrientation orientation);
static void render_handle(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height);
static void render_activity(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height);

static void color_dark2light(const GdkRGBA * color, GdkRGBA * color_return)
{
    GtkSymbolicColor *literal, *shade;

    literal = gtk_symbolic_color_new_literal(color);
    shade = gtk_symbolic_color_new_shade(literal, 1.3 / 0.7);
    gtk_symbolic_color_unref(literal);

    gtk_symbolic_color_resolve(shade, NULL, color_return);
    gtk_symbolic_color_unref(shade);
}

static void color_dark2light_mid(const GdkRGBA * color_dark_, GdkRGBA * color_light_, GdkRGBA * color_mid_)
{
    GtkSymbolicColor *dark, *light, *mid;

    dark = gtk_symbolic_color_new_literal(color_dark_);
    light = gtk_symbolic_color_new_shade(dark, 1.3 / 0.7);

    gtk_symbolic_color_resolve(light, NULL, color_light_);
    mid = gtk_symbolic_color_new_mix(light, dark, 0.5);
    gtk_symbolic_color_unref(light);
    gtk_symbolic_color_unref(dark);

    gtk_symbolic_color_resolve(mid, NULL, color_mid_);
    gtk_symbolic_color_unref(mid);
}

static void xfce_draw_grip_rough (GtkThemingEngine * engine, cairo_t * cr, GtkStateFlags state, gdouble x, gdouble y, gdouble width, gdouble height, GtkOrientation orientation)
{
    gint xx, yy;
    gint xthick, ythick;
    GdkRGBA light, dark;
    GtkBorder border;

    gtk_theming_engine_get_border(engine, state, &border);

    xthick = border.left;
    ythick = border.top;

    gtk_theming_engine_get_border_color(engine, state, &dark);
    color_dark2light(&dark, &light);

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
        if (width > 15 + xthick)
        {
            gint len = (height - 2 * (ythick + 2) > 4 ? height - 2 * (ythick + 2) :  height - 2 * ythick);
            gint delta = x + (width / 2) - 5;
            yy = y + (height - len) / 2;
            for(xx = 0; xx < 10; xx += 2)
            {
                gdk_cairo_set_source_rgba(cr, &dark);   
                cairo_move_to(cr, xx + delta + 0.5, yy + 0.5);
                cairo_line_to(cr, xx + delta + 0.5, yy + len - 0.5);
                cairo_stroke(cr);
                gdk_cairo_set_source_rgba(cr, &light);   
                cairo_move_to(cr, xx + delta + 1.5, yy + 0.5);
                cairo_line_to(cr, xx + delta + 1.5, yy + len - 0.5);
                cairo_stroke(cr);
            }
        }
    }
    else
    {
        if (height > 15 + ythick)
        {
            gint len = (width - 2 * (xthick + 2) > 4 ? width - 2 * (xthick + 2) :  width - 2 * xthick);
            gint delta = y + (height / 2) - 5;
            xx = x + (width - len) / 2;
            for(yy = 0; yy < 10; yy += 2)
            {
                gdk_cairo_set_source_rgba(cr, &dark);   
                cairo_move_to(cr, xx + 0.5, yy + delta + 0.5);
                cairo_line_to(cr, xx + len - 0.5, yy + delta + 0.5);
                cairo_stroke(cr);
                gdk_cairo_set_source_rgba(cr, &light);   
                cairo_move_to(cr, xx + 0.5, yy + delta + 1.5);
                cairo_line_to(cr, xx + len - 0.5, yy + delta + 1.5);
                cairo_stroke(cr);
            }
        }
    }
}

static void xfce_draw_grip_slide (GtkThemingEngine * engine, cairo_t * cr, GtkStateFlags state, gdouble x, gdouble y, gdouble width, gdouble height, GtkOrientation orientation)
{
    gint xthick, ythick;
    gint gx, gy, gwidth, gheight;
    GdkRGBA light, dark, mid, bg;
    GtkBorder border;

    gtk_theming_engine_get_border(engine, state, &border);

    xthick = border.left;
    ythick = border.top;

    gtk_theming_engine_get_border_color(engine, state, &dark);
    color_dark2light_mid(&dark, &light, &mid);
    gtk_theming_engine_get_background_color(engine, GTK_STATE_FLAG_SELECTED, &bg);

    gx = gy = gwidth = gheight = 0;

    if (orientation == GTK_ORIENTATION_HORIZONTAL)
    {
        gint delta = ((gint)((height - 3) / 2));
        gx = x + delta;
        gy = y + delta;
        gwidth = width - 2 * delta - 1;
        gheight = height - 2 * delta - 1;
    }
    else
    {
        gint delta = ((gint)((width - 3) / 2));
        gx = x + delta;
        gy = y + delta;
        gwidth = width - 2 * delta - 1;
        gheight = height - 2 * delta - 1;
    }

    if ((gheight > 1) && (gwidth > 1))
    {
        gdk_cairo_set_source_rgba(cr, &bg);     
        cairo_rectangle(cr, gx + 1, gy + 1, gwidth - 1, gheight - 1);
        cairo_fill(cr);

        gdk_cairo_set_source_rgba(cr, &dark);   
        cairo_move_to(cr, gx + 0.5, gy + gheight + 0.5);
        cairo_line_to(cr, gx + 0.5, gy + 0.5);
        cairo_line_to(cr, gx + gwidth + 0.5, gy + 0.5);
        cairo_stroke(cr);

        gdk_cairo_set_source_rgba(cr, &light);  
        cairo_move_to(cr, gx + 0.5, gy + gheight + 0.5);
        cairo_line_to(cr, gx + gwidth + 0.5, gy + gheight + 0.5);
        cairo_line_to(cr, gx + gwidth + 0.5, gy + 0.5);
        cairo_stroke(cr);

        gdk_cairo_set_source_rgba(cr, &mid);    
        cairo_rectangle(cr, gx, gy, 1, 1);
        cairo_rectangle(cr, gx + gwidth, gy, 1, 1);
        cairo_rectangle(cr, gx, gy + gheight, 1, 1);
        cairo_rectangle(cr, gx + gwidth, gy + gheight, 1, 1);
        cairo_fill(cr);
    }
}

static void xfce_draw_grips(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height, GtkOrientation orientation)
{
    GtkStateFlags state;
    XfceGripStyle grip_style = XFCE_GRIP_ROUGH;

    state = gtk_theming_engine_get_state(engine);
    gtk_theming_engine_get(engine, state, XFCE_GRIP_STYLE, &grip_style, NULL);

    switch (grip_style)
    {
        case XFCE_GRIP_DISABLED:
            break;
        case XFCE_GRIP_ROUGH:
            xfce_draw_grip_rough (engine, cr, state, x, y, width, height, orientation);
            break;
        case XFCE_GRIP_SLIDE:
            xfce_draw_grip_slide (engine, cr, state, x, y, width, height, orientation);
            break;
    }
}

static void render_line(GtkThemingEngine * engine, cairo_t * cr, gdouble x1, gdouble y_1, gdouble x2, gdouble y_2)
{
    gint16 xthick, ythick;
    gint16 thickness_light;
    gint16 thickness_dark;
    GtkStateFlags state;
    GdkRGBA light, dark;
    GtkBorder border;
    GtkBorderStyle border_style;

    state = gtk_theming_engine_get_state(engine);

    gtk_theming_engine_get(engine, state, GTK_STYLE_PROPERTY_BORDER_STYLE, &border_style, NULL);

#if 0
    if (border_style == GTK_BORDER_STYLE_NONE)
        return;
#endif

    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);

    gtk_theming_engine_get_border_color(engine, state, &dark);
    gtk_theming_engine_get_border(engine, state, &border);

    xthick = border.left;
    ythick = border.top;

#if 0
    switch (border_style)
    {
        case GTK_BORDER_STYLE_NONE:
            break;
        case GTK_BORDER_STYLE_SOLID:
            light = dark;
            break;
        case GTK_BORDER_STYLE_INSET:
            break;
        case GTK_BORDER_STYLE_OUTSET:
            light = dark;
            color_dark2light(&light, &dark);
            break;
    }
#endif
    color_dark2light(&dark, &light);

    /* Vertical */
    if (floor(x1 - x2) == 0)
    {
        thickness_light = xthick / 2;
        thickness_dark = xthick - thickness_light;

	/* Compensation for the way x and y are caclculated */
	x1 += 1 + thickness_dark - thickness_light;
        y_2 += 1;

        cairo_set_line_width (cr, thickness_dark);
        gdk_cairo_set_source_rgba(cr, &dark);
        cairo_move_to(cr, x1 - (thickness_dark / 2.0), y_1);
        cairo_line_to(cr, x1 - (thickness_dark / 2.0), y_2);
        cairo_stroke(cr);

        cairo_set_line_width (cr, thickness_light);
        gdk_cairo_set_source_rgba(cr, &light);
        cairo_move_to(cr, x1 + (thickness_light / 2.0), y_1);
        cairo_line_to(cr, x1 + (thickness_light / 2.0), y_2);
        cairo_stroke(cr);
    }
    else
    {
        thickness_light = ythick / 2;
        thickness_dark = ythick - thickness_light;

	/* Compensation for the way x and y are caclculated */
	y_1 += 1 + thickness_dark - thickness_light;
        x2 += 1;

        cairo_set_line_width (cr, thickness_dark);
        gdk_cairo_set_source_rgba(cr, &dark);
        cairo_move_to(cr, x1, y_1 - (thickness_dark / 2.0));
        cairo_line_to(cr, x2, y_1 - (thickness_dark / 2.0));
        cairo_stroke(cr);

        cairo_set_line_width (cr, thickness_light);
        gdk_cairo_set_source_rgba(cr, &light);
        cairo_move_to(cr, x1, y_1 + (thickness_light / 2.0));
        cairo_line_to(cr, x2, y_1 + (thickness_light / 2.0));
        cairo_stroke(cr);
    }
}

static void render_frame(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height)
{
    gint xt, yt;
    gint xthick, ythick;
    GtkStateFlags state;
    GtkBorderStyle border_style;
    GdkRGBA dark, light, mid, bg;
    GdkRGBA black = {0.0, 0.0, 0.0, 1.0}; /* black */
    gboolean smooth_edge;
    GtkBorder border;
    GtkJunctionSides junction;

    state = gtk_theming_engine_get_state(engine);
    gtk_theming_engine_get(engine, state, GTK_STYLE_PROPERTY_BORDER_STYLE, &border_style, NULL);

    if (border_style == GTK_BORDER_STYLE_NONE)
        return;

    GE_CAIRO_INIT;

    gtk_theming_engine_get_border_color(engine, state, &dark);
    gtk_theming_engine_get_border(engine, state, &border);

    xthick = border.left;
    ythick = border.top;

    /* Spin buttons are a special case */
    if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_SPINBUTTON))
    {
        junction = gtk_theming_engine_get_junction_sides(engine);
        if ((junction & GTK_JUNCTION_TOP) == GTK_JUNCTION_TOP || (junction & GTK_JUNCTION_BOTTOM) == GTK_JUNCTION_BOTTOM)
        {
            if (!(state & GTK_STATE_FLAG_ACTIVE))
            {
                border_style = GTK_BORDER_STYLE_OUTSET;
            }

            if (state == GTK_STATE_FLAG_NORMAL || state & GTK_STATE_FLAG_INSENSITIVE)
            {
                gtk_theming_engine_get_border_color(engine, state, &dark);
            }
            else
            {
                gtk_theming_engine_get_border_color(engine, GTK_STATE_FLAG_NORMAL, &dark);
            }

            y = floor(y);
            height = ceil(height);

            xt = MIN(xthick, width - 1);
            yt = MIN(ythick, height - 1);

            gtk_theming_engine_get(engine, state, XFCE_SMOOTH_EDGE, &smooth_edge, NULL);
            color_dark2light_mid(&dark, &light, &mid);
            if (smooth_edge)
            {
                if ((xt > 1) && (yt > 1))
                {
                    gdk_cairo_set_source_rgba(cr, &dark);
                    if ((junction & GTK_JUNCTION_TOP) != GTK_JUNCTION_TOP)
                    {
                        cairo_rectangle(cr, x - 2, y, 1, 1);
                    }
                    if ((junction & GTK_JUNCTION_BOTTOM) != GTK_JUNCTION_BOTTOM)
                    {
                        cairo_rectangle(cr, x - 2, y + height - 1, 1, 1);
                    }
                    cairo_fill(cr);

                    if ((junction & GTK_JUNCTION_TOP) != GTK_JUNCTION_TOP)
                    {
                        cairo_move_to(cr, x - 0.5, y + 1.5);
                    }
                    else
                    {
                        cairo_move_to(cr, x - 0.5, y + 0.5);
                    }
                    if ((junction & GTK_JUNCTION_BOTTOM) != GTK_JUNCTION_BOTTOM)
                    {
                        cairo_line_to(cr, x - 0.5, y + height - 1.5);
                    }
                    else
                    {
                        cairo_line_to(cr, x - 0.5, y + height - 0.5);
                    }
                    cairo_stroke(cr);

                    if ((junction & GTK_JUNCTION_TOP) != GTK_JUNCTION_TOP)
                    {
                        gdk_cairo_set_source_rgba(cr, &mid);
                        cairo_rectangle(cr, x - 2, y + 1, 1, 1);
                        cairo_fill(cr);
                    }

                    gdk_cairo_set_source_rgba(cr, &light);
                    if ((junction & GTK_JUNCTION_TOP) != GTK_JUNCTION_TOP)
                    {
                        cairo_move_to(cr, x - 1.5, y + 2.5);
                    }
                    else
                    {
                        cairo_move_to(cr, x - 1.5, y + 0.5);
                    }
                    if ((junction & GTK_JUNCTION_BOTTOM) != GTK_JUNCTION_BOTTOM)
                    {
                        cairo_line_to(cr, x - 1.5, y + height - 1.5);
                    }
                    else
                    {
                        cairo_line_to(cr, x - 1.5, y + height - 0.5);
                    }
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &mid);
                    if ((junction & GTK_JUNCTION_TOP) != GTK_JUNCTION_TOP)
                    {
                        cairo_rectangle(cr, x - 1, y, 1, 1);
                    }
                    if ((junction & GTK_JUNCTION_BOTTOM) != GTK_JUNCTION_BOTTOM)
                    {
                        cairo_rectangle(cr, x - 1, y + height - 1, 1, 1);
                    }
                    cairo_fill(cr);
                }
                else if ((xt > 0) && (yt > 0))
                {
                    gdk_cairo_set_source_rgba(cr, &light);
                    cairo_move_to(cr, x - 0.5, y + 0.5);
                    cairo_line_to(cr, x - 0.5, y + height - 0.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &mid);
                    if ((junction & GTK_JUNCTION_TOP) != GTK_JUNCTION_TOP)
                    {
                        cairo_rectangle(cr, x - 1, y, 1, 1);
                    }
                    if ((junction & GTK_JUNCTION_BOTTOM) != GTK_JUNCTION_BOTTOM)
                    {
                        cairo_rectangle(cr, x - 1, y + height - 1, 1, 1);
                    }
                    cairo_fill(cr);
                }
            }
            else
            {
                if ((xt > 1) && (yt > 1))
                {
                    if ((junction & GTK_JUNCTION_TOP) != GTK_JUNCTION_TOP)
                    {
                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_move_to(cr, x - 1.5, y + 0.5);
                        cairo_line_to(cr, x - 0.5, y + 0.5);
                        cairo_stroke(cr);
                    }

                    gdk_cairo_set_source_rgba(cr, &light);
                    if ((junction & GTK_JUNCTION_TOP) != GTK_JUNCTION_TOP)
                    {
                        cairo_move_to(cr, x - 0.5, y + 1.5);
                    }
                    else
                    {
                        cairo_move_to(cr, x - 0.5, y + 0.5);
                    }
                    cairo_line_to(cr, x - 0.5, y + height - 0.5);
                    if ((junction & GTK_JUNCTION_BOTTOM) != GTK_JUNCTION_BOTTOM)
                    {
                        cairo_line_to(cr, x - 1.5, y + height - 0.5);
                    }
                    cairo_stroke(cr);

                    if ((junction & GTK_JUNCTION_TOP) != GTK_JUNCTION_TOP)
                    {
                        gdk_cairo_set_source_rgba(cr, &black);
                        cairo_rectangle(cr, x - 2, y + 1, 1, 1);
                        cairo_fill(cr);
                    }

                    gdk_cairo_set_source_rgba(cr, &dark);
                    if ((junction & GTK_JUNCTION_TOP) != GTK_JUNCTION_TOP)
                    {
                        cairo_move_to(cr, x - 1.5, y + 2.5);
                    }
                    else
                    {
                        cairo_move_to(cr, x - 1.5, y + 0.5);
                    }
                    if ((junction & GTK_JUNCTION_BOTTOM) != GTK_JUNCTION_BOTTOM)
                    {
                        cairo_line_to(cr, x - 1.5, y + height - 1.5);
                    }
                    else
                    {
                        cairo_line_to(cr, x - 1.5, y + height - 0.5);
                    }
                    cairo_stroke(cr);
                }
                else if ((xt > 0) && (yt > 0))
                {
                    if ((junction & GTK_JUNCTION_TOP) != GTK_JUNCTION_TOP)
                    {
                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_rectangle(cr, x - 1, y, 1, 1);
                        cairo_fill(cr);
                    }

                    gdk_cairo_set_source_rgba(cr, &light);
                    if ((junction & GTK_JUNCTION_TOP) != GTK_JUNCTION_TOP)
                    {
                        cairo_move_to(cr, x - 0.5, y + 1.5);
                    }
                    else
                    {
                        cairo_move_to(cr, x - 0.5, y + 0.5);
                    }
                    cairo_line_to(cr, x - 0.5, y + height - 0.5);
                    cairo_stroke(cr);
                }
            }

            if (state != GTK_STATE_FLAG_NORMAL && !(state & GTK_STATE_FLAG_INSENSITIVE))
            {
                gtk_theming_engine_get_border_color(engine, state, &dark);
            }
        }
    }

    xt = MIN(xthick, width - 1);
    yt = MIN(ythick, height - 1);

    switch (border_style)
    {
        case GTK_BORDER_STYLE_NONE:
            break;
        case GTK_BORDER_STYLE_SOLID:
	    gdk_cairo_set_source_rgba(cr, &dark);
            if ((xt > 1) && (yt > 1))
            {
		cairo_set_line_width(cr, 2.0);
                cairo_rectangle(cr, x + 1, y + 1, width - 2, height - 2);
	    }
            else if ((xt > 0) && (yt > 0))
            {
                cairo_rectangle(cr, x + 0.5, y + 0.5, width - 1, height - 1);
            }
	    cairo_stroke(cr);
            break;
        case GTK_BORDER_STYLE_INSET:
            gtk_theming_engine_get(engine, state, XFCE_SMOOTH_EDGE, &smooth_edge, NULL);
            color_dark2light_mid(&dark, &light, &mid);
            if (smooth_edge)
            {
                if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_TROUGH))
                {
                    /* Do nothing */
                }
                else if ((xt > 1) && (yt > 1))
                {
                    gdk_cairo_set_source_rgba(cr, &dark);
                    cairo_move_to(cr, x + 1.5, y + 0.5);
                    cairo_line_to(cr, x + width - 1.5, y + 0.5);
                    cairo_move_to(cr, x + 0.5, y + 1.5);
                    cairo_line_to(cr, x + 0.5, y + height - 1.5);

                    cairo_move_to(cr, x + 1.5, y + height - 0.5);
                    cairo_line_to(cr, x + width - 1.5, y + height - 0.5);
                    cairo_move_to(cr, x + width - 0.5, y + 1.5);
                    cairo_line_to(cr, x + width - 0.5, y + height - 1.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &mid);
                    cairo_move_to(cr, x + width - 1.5, y + 1.5);
                    cairo_line_to(cr, x + 1.5, y + 1.5);
                    cairo_line_to(cr, x + 1.5, y + height - 1.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &light);
                    cairo_move_to(cr, x + 2.5, y + height - 1.5);
                    cairo_line_to(cr, x + width - 1.5, y + height - 1.5);
                    cairo_line_to(cr, x + width - 1.5, y + 2.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &mid);
                    cairo_rectangle(cr, x, y, 1, 1);
                    cairo_rectangle(cr, x + width - 1, y, 1, 1);
                    cairo_rectangle(cr, x, y + height - 1, 1, 1);
                    cairo_rectangle(cr, x + width - 1, y + height - 1, 1, 1);
                    cairo_fill(cr);
                }
                else if ((xt > 0) && (yt > 0))
                {
                    gdk_cairo_set_source_rgba(cr, &dark);
                    cairo_move_to(cr, x + 1.5, y + 0.5);
                    cairo_line_to(cr, x + width - 0.5, y + 0.5);
                    cairo_move_to(cr, x + 0.5, y + 1.5);
                    cairo_line_to(cr, x + 0.5, y + height - 0.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &light);
                    cairo_move_to(cr, x + 1.5, y + height - 0.5);
                    cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                    cairo_line_to(cr, x + width - 0.5, y + 1.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &mid);
                    cairo_rectangle(cr, x, y, 1, 1);
                    cairo_rectangle(cr, x + width - 1, y, 1, 1);
                    cairo_rectangle(cr, x, y + height - 1, 1, 1);
                    cairo_rectangle(cr, x + width - 1, y + height - 1, 1, 1);
                    cairo_fill(cr);
                }
            }
            else
            {
                if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_TROUGH))
                {
		    gtk_theming_engine_get_border_color(engine, GTK_STATE_FLAG_ACTIVE, &dark);
                    gdk_cairo_set_source_rgba(cr, &dark);
                    cairo_rectangle(cr, x + 0.5, y + 0.5, width - 1, height - 1);
                    cairo_stroke(cr);
                }
                else if ((xt > 1) && (yt > 1))
                {
                    gdk_cairo_set_source_rgba(cr, &dark);
                    cairo_move_to(cr, x + width - 0.5, y + 0.5);
                    cairo_line_to(cr, x + 0.5, y + 0.5);
                    cairo_line_to(cr, x + 0.5, y + height - 0.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &light);
                    cairo_move_to(cr, x + 1.5, y + height - 0.5);
                    cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                    cairo_line_to(cr, x + width - 0.5, y + 1.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &black);
                    cairo_move_to(cr, x + width - 1.5, y + 1.5);
                    cairo_line_to(cr, x + 1.5, y + 1.5);
                    cairo_line_to(cr, x + 1.5, y + height - 1.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &dark);
                    cairo_move_to(cr, x + 2.5, y + height - 1.5);
                    cairo_line_to(cr, x + width - 1.5, y + height - 1.5);
                    cairo_line_to(cr, x + width - 1.5, y + 2.5);
                    cairo_stroke(cr);
                }
                else if ((xt > 0) && (yt > 0))
                {
                    gdk_cairo_set_source_rgba(cr, &dark);
                    cairo_move_to(cr, x + width - 0.5, y + 0.5);
                    cairo_line_to(cr, x + 0.5, y + 0.5);
                    cairo_line_to(cr, x + 0.5, y + height - 0.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &light);
                    cairo_move_to(cr, x + 1.5, y + height - 0.5);
                    cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                    cairo_line_to(cr, x + width - 0.5, y + 1.5);
                    cairo_stroke(cr);
                }
            }
            break;
        case GTK_BORDER_STYLE_OUTSET:
            gtk_theming_engine_get(engine, state, XFCE_SMOOTH_EDGE, &smooth_edge, NULL);
            color_dark2light_mid(&dark, &light, &mid);
            if (smooth_edge)
            {
                if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_SPINBUTTON) && !(state & GTK_STATE_FLAG_PRELIGHT))
		{
                    /* Do nothing */
		}
		else if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_PROGRESSBAR))
                {
                    gdk_cairo_set_source_rgba(cr, &dark);
                    cairo_rectangle(cr, x + 0.5, y + 0.5, width - 1, height - 1);
                    cairo_stroke(cr);

		    gtk_theming_engine_get_border_color(engine, GTK_STATE_FLAG_NORMAL, &dark);
		    color_dark2light_mid(&dark, &light, &mid);

                    gdk_cairo_set_source_rgba(cr, &mid);
                    cairo_rectangle(cr, x, y, 1, 1);
                    cairo_rectangle(cr, x + width - 1, y, 1, 1);
                    cairo_rectangle(cr, x, y + height - 1, 1, 1);
                    cairo_rectangle(cr, x + width - 1, y + height - 1, 1, 1);
                    cairo_fill(cr);
                }
                else if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_MENU))
                {
                    if ((xt > 0) && (yt > 0))
                    {
                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_rectangle(cr, x + 0.5, y + 0.5, width - 1, height - 1);
                        cairo_stroke(cr);
                    }
                }
                else if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_MENUBAR))
                {
                    if ((xt > 1) && (yt > 1))
                    {
                        gdk_cairo_set_source_rgba(cr, &mid);
                        cairo_move_to(cr, x + 0.5, y + height - 1.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 1.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_move_to(cr, x + 0.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                        cairo_stroke(cr);
                    }
                    else if ((xt > 0) && (yt > 0))
                    {
                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_move_to(cr, x + 0.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                        cairo_stroke(cr);
                    }
                }
                else if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_DOCK) ||
                         gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_TOOLBAR))
                {
                    if ((xt > 1) && (yt > 1))
                    {
                        gdk_cairo_set_source_rgba(cr, &light);
                        cairo_move_to(cr, x + 0.5, y + 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + 0.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &mid);
                        cairo_move_to(cr, x + 0.5 , y + height - 1.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 1.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_move_to(cr, x + 0.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                        cairo_stroke(cr);
                    }
                    else if ((xt > 0) && (yt > 0))
                    {
                        gdk_cairo_set_source_rgba(cr, &light);
                        cairo_move_to(cr, x + 0.5, y + 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + 0.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_move_to(cr, x + 0.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                        cairo_stroke(cr);
                    }
                }
                else if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_SLIDER))
                {
                    if ((xt > 1) && (yt > 1))
                    {
                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_move_to(cr, x + width - 0.5, y + 0.5);
                        cairo_line_to(cr, x + 0.5, y + 0.5);
                        cairo_line_to(cr, x + 0.5, y + height - 0.5);

                        cairo_move_to(cr, x + 1.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + 1.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &light);
                        cairo_move_to(cr, x + width - 2.5, y + 1.5);
                        cairo_line_to(cr, x + 1.5, y + 1.5);
                        cairo_line_to(cr, x + 1.5, y + height - 2.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &mid);
                        cairo_move_to(cr, x + 1.5, y + height - 1.5);
                        cairo_line_to(cr, x + width - 1.5, y + height - 1.5);
                        cairo_line_to(cr, x + width - 1.5, y + 1.5);
                        cairo_stroke(cr);
                    }
                    else if ((xt > 0) && (yt > 0))
                    {
                        gdk_cairo_set_source_rgba(cr, &light);
                        cairo_move_to(cr, x + width - 0.5, y + 0.5);
                        cairo_line_to(cr, x + 0.5, y + 0.5);
                        cairo_line_to(cr, x + 0.5, y + height - 0.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_move_to(cr, x + 1.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + 1.5);
                        cairo_stroke(cr);
                    }
                }
                else if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_FRAME))
                {
                    if ((xt > 1) && (yt > 1))
                    {
                        gdk_cairo_set_source_rgba(cr, &light);
                        cairo_move_to(cr, x + width - 1.5, y + 0.5);
                        cairo_line_to(cr, x + 0.5, y + 0.5);
                        cairo_line_to(cr, x + 0.5, y + height - 1.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_move_to(cr, x + 0.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + 0.5);
                        cairo_stroke(cr);

                        gtk_theming_engine_get_background_color(engine, state, &bg);
                        gdk_cairo_set_source_rgba(cr, &bg);
                        cairo_move_to(cr, x + width - 1.5, y + 1.5);
                        cairo_line_to(cr, x + 1.5, y + 1.5);
                        cairo_line_to(cr, x + 1.5, y + height - 1.5);

                        cairo_move_to(cr, x + 2.5, y + height - 1.5);
                        cairo_line_to(cr, x + width - 1.5, y + height - 1.5);
                        cairo_line_to(cr, x + width - 1.5, y + 2.5);
                        cairo_stroke(cr);
                    }
                    else if ((xt > 0) && (yt > 0))
                    {
                        gdk_cairo_set_source_rgba(cr, &light);
                        cairo_move_to(cr, x + width - 0.5, y + 0.5);
                        cairo_line_to(cr, x + 0.5, y + 0.5);
                        cairo_line_to(cr, x + 0.5, y + height - 0.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_move_to(cr, x + 1.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + 1.5);
                        cairo_stroke(cr);
                    }
                }
                else
                {
                    if ((xt > 1) && (yt > 1))
                    {
                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_move_to(cr, x + 1.5, y + 0.5);
                        cairo_line_to(cr, x + width - 1.5, y + 0.5);
                        cairo_move_to(cr, x + 0.5, y + 1.5);
                        cairo_line_to(cr, x + 0.5, y + height - 1.5);

                        cairo_move_to(cr, x + 1.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 1.5, y + height - 0.5);
                        cairo_move_to(cr, x + width - 0.5, y + 1.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 1.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &light);
                        cairo_move_to(cr, x + width - 2.5, y + 1.5);
                        cairo_line_to(cr, x + 1.5, y + 1.5);
                        cairo_line_to(cr, x + 1.5, y + height - 2.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &mid);
                        cairo_move_to(cr, x + 1.5, y + height - 1.5);
                        cairo_line_to(cr, x + width - 1.5, y + height - 1.5);
                        cairo_line_to(cr, x + width - 1.5, y + 1.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &mid);
                        cairo_rectangle(cr, x, y, 1, 1);
                        cairo_rectangle(cr, x + width - 1, y, 1, 1);
                        cairo_rectangle(cr, x, y + height - 1, 1, 1);
                        cairo_rectangle(cr, x + width - 1, y + height - 1, 1, 1);
                        cairo_fill(cr);
                    }
                    else if ((xt > 0) && (yt > 0))
                    {
                        gdk_cairo_set_source_rgba(cr, &light);
                        cairo_move_to(cr, x + 1.5, y + 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + 0.5);
                        cairo_move_to(cr, x + 0.5, y + 1.5);
                        cairo_line_to(cr, x + 0.5, y + height - 0.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_move_to(cr, x + 1.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + 1.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &mid);
                        cairo_rectangle(cr, x, y, 1, 1);
                        cairo_rectangle(cr, x + width - 1, y, 1, 1);
                        cairo_rectangle(cr, x, y + height - 1, 1, 1);
                        cairo_rectangle(cr, x + width - 1, y + height - 1, 1, 1);
                        cairo_fill(cr);
                    }
                }
            }
            else
            {
                if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_SPINBUTTON) && !(state & GTK_STATE_FLAG_PRELIGHT))
		{
                    /* Do nothing */
		}
                else if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_MENUBAR))
                {
                    if ((xt > 1) && (yt > 1))
                    {
                        gdk_cairo_set_source_rgba(cr, &mid);
                        cairo_move_to(cr, x + 0.5, y + height - 1.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 1.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_move_to(cr, x + 0.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                        cairo_stroke(cr);
                    }
                    else if ((xt > 0) && (yt > 0))
                    {
                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_move_to(cr, x + 0.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                        cairo_stroke(cr);
                    }
                }
                else if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_DOCK) ||
                         gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_TOOLBAR))
                {
                    if ((xt > 1) && (yt > 1))
                    {
                        gdk_cairo_set_source_rgba(cr, &light);
                        cairo_move_to(cr, x + 0.5, y + 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + 0.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &mid);
                        cairo_move_to(cr, x + 0.5 , y + height - 1.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 1.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_move_to(cr, x + 0.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                        cairo_stroke(cr);
                    }
                    else if ((xt > 0) && (yt > 0))
                    {
                        gdk_cairo_set_source_rgba(cr, &light);
                        cairo_move_to(cr, x + 0.5, y + 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + 0.5);
                        cairo_stroke(cr);

                        gdk_cairo_set_source_rgba(cr, &dark);
                        cairo_move_to(cr, x + 0.5, y + height - 0.5);
                        cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                        cairo_stroke(cr);
                    }
                }
                else if ((xt > 1) && (yt > 1))
                {
                    gdk_cairo_set_source_rgba(cr, &dark);
                    cairo_move_to(cr, x + width - 0.5, y + 0.5);
                    cairo_line_to(cr, x + 0.5, y + 0.5);
                    cairo_line_to(cr, x + 0.5, y + height - 0.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &black);
                    cairo_move_to(cr, x + 1.5, y + height - 0.5);
                    cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                    cairo_line_to(cr, x + width - 0.5, y + 1.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &light);
                    cairo_move_to(cr, x + width - 1.5, y + 1.5);
                    cairo_line_to(cr, x + 1.5, y + 1.5);
                    cairo_line_to(cr, x + 1.5, y + height - 1.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &dark);
                    cairo_move_to(cr, x + 2.5, y + height - 1.5);
                    cairo_line_to(cr, x + width - 1.5, y + height - 1.5);
                    cairo_line_to(cr, x + width - 1.5, y + 2.5);
                    cairo_stroke(cr);
                }
                else if ((xt > 0) && (yt > 0))
                {
                    gdk_cairo_set_source_rgba(cr, &light);
                    cairo_move_to(cr, x + width - 0.5, y + 0.5);
                    cairo_line_to(cr, x + 0.5, y + 0.5);
                    cairo_line_to(cr, x + 0.5, y + height - 0.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &dark);
                    cairo_move_to(cr, x + 1.5, y + height - 0.5);
                    cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                    cairo_line_to(cr, x + width - 0.5, y + 1.5);
                    cairo_stroke(cr);
                }
            }
            break;
    }
}

static cairo_surface_t *get_part_bmap (Part part)
{
    if (!parts[part].bmap)
    {
        parts[part].bmap = cairo_image_surface_create_for_data((guchar*)parts[part].bits, CAIRO_FORMAT_A1, PART_SIZE, PART_SIZE, sizeof(guint32));
    }
    return parts[part].bmap;
}

static void draw_part(cairo_t * cr, const GdkRGBA * c, gdouble x, gdouble y, Part part)
{
    gdk_cairo_set_source_rgba(cr, c);

    cairo_mask_surface(cr, get_part_bmap (part), x, y);
}

static void render_check(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height)
{
    GtkStateFlags state;
    GdkRGBA bg, border, fg;

    x -= (1 + PART_SIZE - width) / 2;
    y -= (1 + PART_SIZE - height) / 2;

    state = gtk_theming_engine_get_state(engine);
    gtk_theming_engine_get_background_color(engine, state, &bg);
    gtk_theming_engine_get_border_color(engine, state, &border);
    gtk_theming_engine_get_color(engine, state, &fg);

    if (!gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_MENUITEM))
        draw_part(cr, &bg, x, y, CHECK_BASE);
    draw_part(cr, &border, x, y, CHECK_LIGHT);
    draw_part(cr, &border, x, y, CHECK_DARK);

    if (state & GTK_STATE_FLAG_INCONSISTENT)
    {
        draw_part(cr, &fg, x, y, CHECK_DASH);
    }
    else if (state & GTK_STATE_FLAG_ACTIVE)
    {
        draw_part(cr, &fg, x, y, CHECK_CROSS);
    }
}

static void render_option(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height)
{
    GtkStateFlags state;
    GdkRGBA bg, border, fg;

    x -= (1 + PART_SIZE - width) / 2;
    y -= (1 + PART_SIZE - height) / 2;

    state = gtk_theming_engine_get_state(engine);
    gtk_theming_engine_get_background_color(engine, state, &bg);
    gtk_theming_engine_get_border_color(engine, state, &border);
    gtk_theming_engine_get_color(engine, state, &fg);

    if (!gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_MENUITEM))
        draw_part(cr, &bg, x, y, RADIO_BASE);
    draw_part(cr, &border, x, y, RADIO_LIGHT);
    draw_part(cr, &border, x, y, RADIO_DARK);

    if (state & GTK_STATE_FLAG_INCONSISTENT)
    {
        draw_part(cr, &fg, x, y, CHECK_DASH);
    }
    else if (state & GTK_STATE_FLAG_ACTIVE)
    {
        draw_part(cr, &fg, x, y, RADIO_TEXT);
    }
}

static void render_frame_gap(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height, GtkPositionType gap_side, gdouble gap_s, gdouble gap_e)
{
    GtkStateFlags state;
    GtkBorderStyle border_style;
    gdouble x0, y_0, x1, y_1;
    gdouble ex, ey, ew, eh;

    state = gtk_theming_engine_get_state(engine);

    if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_NOTEBOOK))
    {
        GdkRGBA c1;
        GdkRGBA c2;
        GdkRGBA c3;
        GdkRGBA c4 = {0.0, 0.0, 0.0, 1.0}; /* black */
        gboolean smooth_edge;

        GE_CAIRO_INIT;

        gtk_theming_engine_get(engine, state, XFCE_SMOOTH_EDGE, &smooth_edge, NULL);
        if (smooth_edge)
        {
            gtk_theming_engine_get_background_color(engine, state, &c2);
            gtk_theming_engine_get_border_color(engine, state, &c1);
            c3 = c2;
            c4 = c1;
        }
        else
        {
            gtk_theming_engine_get_border_color(engine, state, &c1);
            color_dark2light(&c1, &c2);
            c3 = c1;
        }

        switch (gap_side)
        {
            case GTK_POS_TOP:
                gdk_cairo_set_source_rgba(cr, &c1);
                cairo_move_to(cr, x + 0.5, y + 0.5);
                cairo_line_to(cr, x + 0.5, y + height - 0.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &c2);
                cairo_move_to(cr, x + 1.5, y + 1.5);
                cairo_line_to(cr, x + 1.5, y + height - 1.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &c3);
                cairo_move_to(cr, x + 2.5, y + height - 1.5);
                cairo_line_to(cr, x + width - 1.5, y + height - 1.5);
                cairo_line_to(cr, x + width - 1.5, y + 0.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &c4);
                cairo_move_to(cr, x + 1.5, y + height - 0.5);
                cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                cairo_line_to(cr, x + width - 0.5, y + 0.5);
                cairo_stroke(cr);

                cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
                if (gap_s > 0)
                {
                    gdk_cairo_set_source_rgba(cr, &c1);
                    cairo_move_to(cr, x, y + 0.5);
                    cairo_line_to(cr, x + gap_s, y + 0.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &c2);
                    cairo_move_to(cr, x + 1, y + 1.5);
                    cairo_line_to(cr, x + gap_s, y + 1.5);
                    cairo_stroke(cr);

                    cairo_rectangle(cr, x + gap_s, y + 0.5, 1, 1);
                    cairo_move_to(cr, x + gap_s, y);
                    cairo_line_to(cr, x + gap_s + 1, y + 0.5);
                    cairo_fill(cr);
                }
                if ((width - gap_e) > 0)
                {
                    gdk_cairo_set_source_rgba(cr, &c1);
                    cairo_move_to(cr, x + gap_e, y + 0.5);
                    cairo_line_to(cr, x + width - 1, y + 0.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &c2);
                    cairo_move_to(cr, x + gap_e, y + 1.5);
                    cairo_line_to(cr, x + width - 1, y + 1.5);

                    cairo_move_to(cr, x + gap_e - 1, y + 0.5);
                    cairo_line_to(cr, x + gap_e, y + 0.5);
                    cairo_stroke(cr);
                }
                break;
            case GTK_POS_BOTTOM:
                gdk_cairo_set_source_rgba(cr, &c1);
                cairo_move_to(cr, x + width - 0.5, y + 0.5);
                cairo_line_to(cr, x + 0.5, y + 0.5);
                cairo_line_to(cr, x + 0.5, y + height - 0.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &c2);
                cairo_move_to(cr, x + width - 1.5, y + 1.5);
                cairo_line_to(cr, x + 1.5, y + 1.5);
                cairo_line_to(cr, x + 1.5, y + height - 0.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &c3);
                cairo_move_to(cr, x + width - 1.5, y + 1.5);
                cairo_line_to(cr, x + width - 1.5, y + height - 0.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &c4);
                cairo_move_to(cr, x + width - 0.5, y + 0.5);
                cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                cairo_stroke(cr);

                cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
                if (gap_s > 0)
                {
                    gdk_cairo_set_source_rgba(cr, &c4);
                    cairo_move_to(cr, x, y + height - 0.5);
                    cairo_line_to(cr, x + gap_s, y + height - 0.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &c3);
                    cairo_move_to(cr, x + 1, y + height - 1.5);
                    cairo_line_to(cr, x + gap_s, y + height - 1.5);

                    cairo_move_to(cr, x + gap_s, y + height - 0.5);
                    cairo_line_to(cr, x + gap_s + 1, y + height - 0.5);
                    cairo_stroke(cr);
                }
                if ((width - gap_e) > 0)
                {
                    gdk_cairo_set_source_rgba(cr, &c4);
                    cairo_move_to(cr, x + gap_e, y + height - 0.5);
                    cairo_line_to(cr, x + width - 1, y + height - 0.5);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &c3);
                    cairo_move_to(cr, x + gap_e, y + height - 1.5);
                    cairo_line_to(cr, x + width - 1, y + height - 1.5);

                    cairo_move_to(cr, x + gap_e - 1, y + height - 0.5);
                    cairo_line_to(cr, x + gap_e, y + height - 0.5);
                    cairo_stroke(cr);
                }
                break;
            case GTK_POS_LEFT:
                gdk_cairo_set_source_rgba(cr, &c1);
                cairo_move_to(cr, x + 0.5, y + 0.5);
                cairo_line_to(cr, x + width - 0.5, y + 0.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &c2);
                cairo_move_to(cr, x + 0.5, y + 1.5);
                cairo_line_to(cr, x + width - 1.5, y + 1.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &c3);
                cairo_move_to(cr, x + 0.5, y + height - 1.5);
                cairo_line_to(cr, x + width - 1.5, y + height - 1.5);
                cairo_line_to(cr, x + width - 1.5, y + 1.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &c4);
                cairo_move_to(cr, x + 1.5, y + height - 0.5);
                cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                cairo_line_to(cr, x + width - 0.5, y + 0.5);
                cairo_stroke(cr);

                cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
                if (gap_s > 0)
                {
                    gdk_cairo_set_source_rgba(cr, &c1);
                    cairo_move_to(cr, x + 0.5, y);
                    cairo_line_to(cr, x + 0.5, y + gap_s);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &c2);
                    cairo_move_to(cr, x + 1.5, y + 1);
                    cairo_line_to(cr, x + 1.5, y + gap_s);

                    cairo_move_to(cr, x + 0.5, y + gap_s);
                    cairo_line_to(cr, x + 0.5, y + gap_s + 1);
                    cairo_stroke(cr);
                }
                if ((width - gap_e) > 0)
                {
                    gdk_cairo_set_source_rgba(cr, &c1);
                    cairo_move_to(cr, x + 0.5, y + gap_e);
                    cairo_line_to(cr, x + 0.5, y + height - 1);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &c2);
                    cairo_move_to(cr, x + 1.5, y + gap_e);
                    cairo_line_to(cr, x + 1.5, y + height - 1);

                    cairo_move_to(cr, x + 0.5, y + gap_e - 1);
                    cairo_line_to(cr, x + 0.5, y + gap_e);
                    cairo_stroke(cr);
                }
                break;
            case GTK_POS_RIGHT:
                gdk_cairo_set_source_rgba(cr, &c1);
                cairo_move_to(cr, x + width - 0.5, y + 0.5);
                cairo_line_to(cr, x + 0.5, y + 0.5);
                cairo_line_to(cr, x + 0.5, y + height - 0.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &c2);
                cairo_move_to(cr, x + width - 0.5, y + 1.5);
                cairo_line_to(cr, x + 1.5, y + 1.5);
                cairo_line_to(cr, x + 1.5, y + height - 1.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &c3);
                cairo_move_to(cr, x + 1.5, y + height - 1.5);
                cairo_line_to(cr, x + width - 0.5, y + height - 1.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &c4);
                cairo_move_to(cr, x + 1.5, y + height - 0.5);
                cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                cairo_stroke(cr);

                cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
                if (gap_s > 0)
                {
                    gdk_cairo_set_source_rgba(cr, &c4);
                    cairo_move_to(cr, x + width - 0.5, y);
                    cairo_line_to(cr, x + width - 0.5, y + gap_s);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &c3);
                    cairo_move_to(cr, x + width - 1.5, y + 1);
                    cairo_line_to(cr, x + width - 1.5, y + gap_s);

                    cairo_move_to(cr, x + width - 0.5, y + gap_s);
                    cairo_line_to(cr, x + width - 0.5, y + gap_s + 1);
                    cairo_stroke(cr);
                }
                if ((width - gap_e) > 0)
                {
                    gdk_cairo_set_source_rgba(cr, &c4);
                    cairo_move_to(cr, x + width - 0.5, y + gap_e);
                    cairo_line_to(cr, x + width - 0.5, y + height - 1);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &c3);
                    cairo_move_to(cr, x + width - 1.5, y + gap_e);
                    cairo_line_to(cr, x + width - 1.5, y + height - 1);

                    cairo_move_to(cr, x + width - 0.5, y + gap_e - 1);
                    cairo_line_to(cr, x + width - 0.5, y + gap_e);
                    cairo_stroke(cr);
                }
                break;
        }

        return;
    }

    gtk_theming_engine_get(engine, state, GTK_STYLE_PROPERTY_BORDER_STYLE, &border_style, NULL);

    if (border_style == GTK_BORDER_STYLE_NONE)
        return;

    switch (gap_side)
    {
        case GTK_POS_TOP:
            ex = x + gap_s;
            ey = y;
            ew = gap_e - gap_s;
            eh = 2;
            break;
        case GTK_POS_BOTTOM:
            ex = x + gap_s;
            ey = y + height - 2;
            ew = gap_e - gap_s;
            eh = 2;
            break;
        case GTK_POS_LEFT:
            ex = x;
            ey = y + gap_s;
            ew = 2;
            eh = gap_e - gap_s;
            break;
        case GTK_POS_RIGHT:
            ex = x + width - 2;
            ey = y + gap_s;
            ew = 2;
            eh = gap_e - gap_s;
            break;
    }

    cairo_save (cr);

    cairo_clip_extents (cr, &x0, &y_0, &x1, &y_1);
    cairo_rectangle (cr, x0, y_0, x1 - x0, ey - y_0);
    cairo_rectangle (cr, x0, ey, ex - x0, eh);
    cairo_rectangle (cr, ex + ew, ey, x1 - (ex + ew), eh);
    cairo_rectangle (cr, x0, ey + eh, x1 - x0, y_1 - (ey + eh));
    cairo_clip (cr);

    render_frame (engine, cr, x, y, width, height);

    cairo_restore (cr);
}

static void render_extension(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height, GtkPositionType gap_side)
{
    GtkStateFlags state;
    gboolean smooth_edge = FALSE;
    GdkRGBA c1;
    GdkRGBA c2;
    GdkRGBA c3;
    GdkRGBA c4 = {0.0, 0.0, 0.0, 1.0}; /* black */
    GtkBorder border;

    GE_CAIRO_INIT;

    state = gtk_theming_engine_get_state(engine);
    gtk_theming_engine_get_border(engine, state, &border);

    GTK_THEMING_ENGINE_GET_CLASS(engine)->render_background(engine, cr, x - border.left + 1, y - border.top + 1, width + border.left + border.right - 2, height + border.top + border.bottom - 2);

    gtk_theming_engine_get(engine, state, XFCE_SMOOTH_EDGE, &smooth_edge, NULL);

    if (smooth_edge)
    {
        gtk_theming_engine_get_background_color(engine, state, &c2);
        gtk_theming_engine_get_border_color(engine, state, &c1);
        c3 = c2;
        c4 = c1;
    }
    else
    {
        gtk_theming_engine_get_border_color(engine, state, &c1);
        color_dark2light(&c1, &c2);
        c3 = c1;
    }

    switch (gap_side)
    {
        case GTK_POS_TOP:
            gdk_cairo_set_source_rgba(cr, &c1);
            cairo_move_to(cr, x + 0.5, y + 0.5);
            cairo_line_to(cr, x + 0.5, y + height - 1.5);
            cairo_stroke(cr);

            gdk_cairo_set_source_rgba(cr, &c2);
            cairo_move_to(cr, x + 1.5, y + 1.5);
            cairo_line_to(cr, x + 1.5, y + height - 1.5);
            cairo_stroke(cr);

            gdk_cairo_set_source_rgba(cr, &c3);
            cairo_move_to(cr, x + 2.5, y + height - 1.5);
            cairo_line_to(cr, x + width - 1.5, y + height - 1.5);
            cairo_line_to(cr, x + width - 1.5, y + 0.5);
            cairo_stroke(cr);

            gdk_cairo_set_source_rgba(cr, &c4);
            cairo_move_to(cr, x + 1.5, y + height - 0.5);
            cairo_line_to(cr, x + width - 1.5, y + height - 0.5);
            cairo_move_to(cr, x + width - 0.5, y + 0.5);
            cairo_line_to(cr, x + width - 0.5, y + height - 1.5);
            cairo_stroke(cr);
            break;
        case GTK_POS_BOTTOM:
            gdk_cairo_set_source_rgba(cr, &c1);
            cairo_move_to(cr, x + 1.5, y + 0.5);
            cairo_line_to(cr, x + width - 1.5, y + 0.5);
            cairo_move_to(cr, x + 0.5, y + 1.5);
            cairo_line_to(cr, x + 0.5, y + height - 0.5);
            cairo_stroke(cr);

            gdk_cairo_set_source_rgba(cr, &c2);
            cairo_move_to(cr, x + width - 1.5, y + 1.5);
            cairo_line_to(cr, x + 1.5, y + 1.5);
            cairo_line_to(cr, x + 1.5, y + height - 0.5);
            cairo_stroke(cr);

            gdk_cairo_set_source_rgba(cr, &c3);
            cairo_move_to(cr, x + width - 1.5, y + 2.5);
            cairo_line_to(cr, x + width - 1.5, y + height - 0.5);
            cairo_stroke(cr);

            gdk_cairo_set_source_rgba(cr, &c4);
            cairo_move_to(cr, x + width - 0.5, y + 1.5);
            cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
            cairo_stroke(cr);
            break;
        case GTK_POS_LEFT:
            gdk_cairo_set_source_rgba(cr, &c1);
            cairo_move_to(cr, x + 0.5, y + 0.5);
            cairo_line_to(cr, x + width - 1.5, y + 0.5);
            cairo_stroke(cr);

            gdk_cairo_set_source_rgba(cr, &c2);
            cairo_move_to(cr, x + 1.5, y + 1.5);
            cairo_line_to(cr, x + width - 1.5, y + 1.5);
            cairo_stroke(cr);

            gdk_cairo_set_source_rgba(cr, &c3);
            cairo_move_to(cr, x + 0.5, y + height - 1.5);
            cairo_line_to(cr, x + width - 1.5, y + height - 1.5);
            cairo_line_to(cr, x + width - 1.5, y + 2.5);
            cairo_stroke(cr);

            gdk_cairo_set_source_rgba(cr, &c4);
            cairo_move_to(cr, x + 0.5, y + height - 0.5);
            cairo_line_to(cr, x + width - 1.5, y + height - 0.5);
            cairo_move_to(cr, x + width - 0.5, y + 1.5);
            cairo_line_to(cr, x + width - 0.5, y + height - 1.5);
            cairo_stroke(cr);
            break;
        case GTK_POS_RIGHT:
            gdk_cairo_set_source_rgba(cr, &c1);
            cairo_move_to(cr, x + 1.5, y + 0.5);
            cairo_line_to(cr, x + width - 0.5, y + 0.5);
            cairo_move_to(cr, x + 0.5, y + 1.5);
            cairo_line_to(cr, x + 0.5, y + height - 1.5);
            cairo_stroke(cr);

            gdk_cairo_set_source_rgba(cr, &c2);
            cairo_move_to(cr, x + width - 0.5, y + 1.5);
            cairo_line_to(cr, x + 1.5, y + 1.5);
            cairo_line_to(cr, x + 1.5, y + height - 1.5);
            cairo_stroke(cr);

            gdk_cairo_set_source_rgba(cr, &c3);
            cairo_move_to(cr, x + 2.5, y + height - 1.5);
            cairo_line_to(cr, x + width - 0.5, y + height - 1.5);
            cairo_stroke(cr);

            gdk_cairo_set_source_rgba(cr, &c4);
            cairo_move_to(cr, x + 1.5, y + height - 0.5);
            cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
            cairo_stroke(cr);
            break;
    }
}

static void render_slider(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height, GtkOrientation orientation)
{
    GE_CAIRO_INIT;

    GTK_THEMING_ENGINE_GET_CLASS(engine)->render_background(engine, cr, x, y, width, height);
    GTK_THEMING_ENGINE_GET_CLASS(engine)->render_frame(engine, cr, x, y, width, height);
    xfce_draw_grips(engine, cr, x, y, width, height, orientation);
}

static void render_handle(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height)
{
    GtkOrientation orientation;

    if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_GRIP))
    {
        GTK_THEMING_ENGINE_CLASS(xfce_engine_parent_class)->render_handle(engine, cr, x, y, width, height);
        return;
    }

    GE_CAIRO_INIT;

    orientation = GTK_ORIENTATION_HORIZONTAL;
    if (height > width)
        orientation = GTK_ORIENTATION_VERTICAL;

#if 0
    GTK_THEMING_ENGINE_GET_CLASS(engine)->render_background(engine, cr, x, y, width, height);
#endif
    xfce_draw_grips(engine, cr, x, y, width, height, orientation);
}

static void render_activity(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height)
{
    if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_PROGRESSBAR))
    {
        GTK_THEMING_ENGINE_GET_CLASS(engine)->render_background(engine, cr, x, y, width, height);
        GTK_THEMING_ENGINE_GET_CLASS(engine)->render_frame(engine, cr, x, y, width, height);
    }
    else
    {
        GTK_THEMING_ENGINE_CLASS(xfce_engine_parent_class)->render_activity(engine, cr, x, y, width, height);
    }
}

void xfce_engine_register_types(GTypeModule * module)
{
    xfce_engine_register_type(module);
}

static void xfce_engine_init(XfceEngine * engine)
{
}

static void xfce_engine_class_init(XfceEngineClass * klass)
{
    GtkThemingEngineClass *engine_class = GTK_THEMING_ENGINE_CLASS(klass);

    engine_class->render_line = render_line;
    engine_class->render_frame = render_frame;
    engine_class->render_check = render_check;
    engine_class->render_option = render_option;
    engine_class->render_frame_gap = render_frame_gap;
    engine_class->render_extension = render_extension;
    engine_class->render_slider = render_slider;
    engine_class->render_handle = render_handle;
    engine_class->render_activity = render_activity;

    gtk_theming_engine_register_property(XFCE_NAMESPACE, NULL,
            g_param_spec_boolean(SMOOTH_EDGE, "Smooth edge", "Smooth edge",
                FALSE, 0));
    gtk_theming_engine_register_property(XFCE_NAMESPACE, NULL,
            g_param_spec_enum(GRIP_STYLE, "Grip style", "Grip style",
                XFCE_TYPE_GRIP_STYLE, XFCE_GRIP_ROUGH, 0));
}

static void xfce_engine_class_finalize(XfceEngineClass * klass)
{
}

