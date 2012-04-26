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
 *  Copyright (C) 2011-2012 Peter de Ridder (peter@xfce.org)
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

#define BUTTON_DEFAULT_BORDER "button-default-border"
#define XFCE_BUTTON_DEFAULT_BORDER "-"XFCE_NAMESPACE"-"BUTTON_DEFAULT_BORDER

/* macros to make sure that things are sane ... */
#define GE_CAIRO_INIT                               \
    cairo_set_line_width (cr, 1.0);                 \
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_SQUARE); \
    cairo_set_line_join (cr, CAIRO_LINE_JOIN_MITER);

G_DEFINE_DYNAMIC_TYPE(XfceEngine, xfce_engine, GTK_TYPE_THEMING_ENGINE)

#define CHECK_MIN_SIZE 15
#define CHECK_DRAW_SIZE 11

/* internal functions */
static void xfce_draw_grips(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height, GtkOrientation orientation);
static void xfce_draw_frame(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height, GtkBorderStyle border_style);

static void render_line(GtkThemingEngine * engine, cairo_t * cr, gdouble x1, gdouble y1, gdouble x2, gdouble y2);
static void render_background(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height);
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
    gint gx, gy, gwidth, gheight;
    GdkRGBA light, dark, mid, bg;
    GtkBorder border;

    gtk_theming_engine_get_border(engine, state, &border);

    gtk_theming_engine_get_border_color(engine, state, &dark);
    color_dark2light_mid(&dark, &light, &mid);
    gtk_theming_engine_get_color(engine, state, &bg);

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

        y_1 = floor(y_1);
        y_2 = floor(y_2);

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

        x1 = floor(x1);
        x2 = floor(x2);

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

static void render_background(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height)
{
    GdkRGBA bg_color;
    cairo_pattern_t *pattern;
    gint xthick, ythick;
    gint xt, yt;
    GtkStateFlags state;
    GtkBorder border;
    gboolean smooth_edge;

    state = gtk_theming_engine_get_state(engine);

    gtk_theming_engine_get_background_color(engine, state, &bg_color);
    gtk_theming_engine_get(engine, state, GTK_STYLE_PROPERTY_BACKGROUND_IMAGE, &pattern, NULL);

    gtk_theming_engine_get(engine, state, XFCE_SMOOTH_EDGE, &smooth_edge, NULL);
    gtk_theming_engine_get_border (engine, state, &border);

    xthick = border.left;
    ythick = border.top;

    if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_SPINBUTTON) && gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_BUTTON))
    {
        if (!(state & GTK_STATE_FLAG_ACTIVE) &&  !(state & GTK_STATE_FLAG_PRELIGHT))
            return;
    }

    cairo_save(cr);
    cairo_translate(cr, x, y);

    xt = MIN(xthick, width - 1);
    yt = MIN(ythick, height - 1);
    if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_NOTEBOOK))
    {
        xt = 0;
        yt = 0;
    }
    else if ((smooth_edge && gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_PROGRESSBAR)) ||
             (smooth_edge && gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_TROUGH)))
    {
        xt = 1;
        yt = 1;
    }
    else
    {
        xt = MIN(xt, 2);
        xt = MIN(xt, yt);
        yt = xt;
    }

    /* The menubar only draws a bottom line */
    if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_MENUBAR) &&
        !gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_MENUITEM))
    {
        cairo_rectangle(cr, 0, 0, width, height - yt);
    }
    else
    {
        cairo_rectangle(cr, xt, yt, width - xt * 2, height - yt * 2);
    }

    if(pattern)
    {
        cairo_scale(cr, width, height);
        cairo_set_source(cr, pattern);
        cairo_scale(cr, 1.0 / width, 1.0 / height);
    }
    else
    {
        gdk_cairo_set_source_rgba(cr, &bg_color);
    }

    cairo_fill(cr);

    if(pattern)
    {
        cairo_pattern_destroy (pattern);
    }

    cairo_restore(cr);
}

static void render_frame(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height)
{
    gint xt, yt;
    gint xthick, ythick;
    GtkStateFlags state;
    GtkBorderStyle border_style;
    GtkBorder border;
    GtkBorder *default_border;

    state = gtk_theming_engine_get_state(engine);
    gtk_theming_engine_get(engine, state, GTK_STYLE_PROPERTY_BORDER_STYLE, &border_style, NULL);

    xthick = border.left;
    ythick = border.top;

    xt = MIN(xthick, width - 1);
    yt = MIN(ythick, height - 1);

    /* Spin buttons are a special case */
    if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_SPINBUTTON) && gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_BUTTON))
    {
        /* Draw an outset border when hovering a spinner button */
        if (!(state & GTK_STATE_FLAG_ACTIVE))
            border_style = GTK_BORDER_STYLE_OUTSET;
    }

    /* Default buttons are a special case */
    if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_BUTTON) && gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_DEFAULT))
    {
        /* Draw an inset border around the default border */
        gtk_theming_engine_get(engine, state, XFCE_BUTTON_DEFAULT_BORDER, &default_border, NULL);

        if (default_border &&
            (default_border->left > xt) && (default_border->right > xt) &&
            (default_border->top > yt) && (default_border->bottom > yt))
        {
            xfce_draw_frame(engine, cr, x - default_border->left, y - default_border->top,
                    width + default_border->left + default_border->right, height + default_border->top + default_border->bottom,
                    GTK_BORDER_STYLE_INSET);
        }

        gtk_border_free(default_border);
    }

    xfce_draw_frame(engine, cr, x, y, width, height, border_style);
}

static void xfce_draw_frame(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height, GtkBorderStyle border_style)
{
    gint xt, yt;
    gint xthick, ythick;
    GtkStateFlags state;
    GdkRGBA dark, light, mid, bg;
    GdkRGBA black = {0.0, 0.0, 0.0, 1.0}; /* black */
    gboolean smooth_edge;
    GtkBorder border;

    state = gtk_theming_engine_get_state(engine);

    if (border_style == GTK_BORDER_STYLE_NONE)
        return;

    GE_CAIRO_INIT;

    gtk_theming_engine_get_border_color(engine, state, &dark);
    gtk_theming_engine_get_border(engine, state, &border);

    xthick = border.left;
    ythick = border.top;

    xt = MIN(xthick, width - 1);
    yt = MIN(ythick, height - 1);

    switch (border_style)
    {
#if GTK_CHECK_VERSION(3,4,0)
        case GTK_BORDER_STYLE_HIDDEN:
#endif
        case GTK_BORDER_STYLE_NONE:
            break;
#if GTK_CHECK_VERSION(3,4,0)
        case GTK_BORDER_STYLE_DOTTED:
        case GTK_BORDER_STYLE_DASHED:
        case GTK_BORDER_STYLE_DOUBLE:
#endif
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
#if GTK_CHECK_VERSION(3,4,0)
	case GTK_BORDER_STYLE_GROOVE:
            color_dark2light(&dark, &light);
            if ((xt > 1) && (yt > 1))
            {
                gdk_cairo_set_source_rgba(cr, &light);
                cairo_move_to(cr, x + 0.5, y + height - 0.5);
                cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                cairo_line_to(cr, x + width - 0.5, y + 0.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &dark);
                cairo_move_to(cr, x + width - 1.5, y + 0.5);
                cairo_line_to(cr, x + 0.5, y + 0.5);
                cairo_line_to(cr, x + 0.5, y + height - 1.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &light);
                cairo_move_to(cr, x + width - 1.5, y + 1.5);
                cairo_line_to(cr, x + 1.5, y + 1.5);
                cairo_line_to(cr, x + 1.5, y + height - 1.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &dark);
                cairo_move_to(cr, x + 1.5, y + height - 1.5);
                cairo_line_to(cr, x + width - 1.5, y + height - 1.5);
                cairo_line_to(cr, x + width - 1.5, y + 1.5);
                cairo_stroke(cr);
            }
            else if ((xt > 0) && (yt > 0))
            {
                gdk_cairo_set_source_rgba(cr, &dark);
                cairo_rectangle(cr, x + 0.5, y + 0.5, width - 1, height - 1);
                cairo_stroke(cr);
            }
            break;
	case GTK_BORDER_STYLE_RIDGE:
            color_dark2light(&dark, &light);
            if ((xt > 1) && (yt > 1))
            {
                gdk_cairo_set_source_rgba(cr, &dark);
                cairo_move_to(cr, x + 0.5, y + height - 0.5);
                cairo_line_to(cr, x + width - 0.5, y + height - 0.5);
                cairo_line_to(cr, x + width - 0.5, y + 0.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &light);
                cairo_move_to(cr, x + width - 1.5, y + 0.5);
                cairo_line_to(cr, x + 0.5, y + 0.5);
                cairo_line_to(cr, x + 0.5, y + height - 1.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &dark);
                cairo_move_to(cr, x + width - 1.5, y + 1.5);
                cairo_line_to(cr, x + 1.5, y + 1.5);
                cairo_line_to(cr, x + 1.5, y + height - 1.5);
                cairo_stroke(cr);

                gdk_cairo_set_source_rgba(cr, &light);
                cairo_move_to(cr, x + 1.5, y + height - 1.5);
                cairo_line_to(cr, x + width - 1.5, y + height - 1.5);
                cairo_line_to(cr, x + width - 1.5, y + 1.5);
                cairo_stroke(cr);
            }
            else if ((xt > 0) && (yt > 0))
            {
                gdk_cairo_set_source_rgba(cr, &light);
                cairo_rectangle(cr, x + 0.5, y + 0.5, width - 1, height - 1);
                cairo_stroke(cr);
            }
            break;
#endif
        case GTK_BORDER_STYLE_INSET:
            gtk_theming_engine_get(engine, state, XFCE_SMOOTH_EDGE, &smooth_edge, NULL);
            color_dark2light_mid(&dark, &light, &mid);
            if (smooth_edge)
            {
                if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_TROUGH) && !gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_SCALE))
                {
                    gtk_theming_engine_get_background_color(engine, state, &bg);
                    gdk_cairo_set_source_rgba(cr, &bg);
                    cairo_rectangle(cr, x + 0.5, y + 0.5, width - 1, height - 1);
                    cairo_stroke(cr);

                    gdk_cairo_set_source_rgba(cr, &mid);
                    cairo_rectangle(cr, x, y, 1, 1);
                    cairo_rectangle(cr, x + width - 1, y, 1, 1);
                    cairo_rectangle(cr, x, y + height - 1, 1, 1);
                    cairo_rectangle(cr, x + width - 1, y + height - 1, 1, 1);
                    cairo_fill(cr);
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
                else if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_MENUBAR) &&
                         !gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_MENUITEM))
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
                else if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_SCROLLBAR) || gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_SLIDER) || gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_SCALE))
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
                else if (gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_MENUBAR) &&
                         !gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_MENUITEM))
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

static void draw_dash(cairo_t * cr, const GdkRGBA * c, gdouble x, gdouble y, guint size)
{
    guint w, b;

    b = (size + 7) / 10;

    w = size / 4;
    if ((w % 2) != (size % 2))
    {
        w += 1;
    }

    gdk_cairo_set_source_rgba(cr, c);

    cairo_set_line_width (cr, w);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);

    cairo_move_to (cr, x + b, y + size / 2.0);
    cairo_line_to (cr, x + size - b, y + size / 2.0);

    cairo_stroke(cr);
}

static void render_check(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height)
{
    GtkStateFlags state;
    GdkRGBA bg, border, fg;
    guint size;
    guint w, b;

    /* Make sure it doesn't get to small */
    if (width < CHECK_MIN_SIZE)
        width = CHECK_DRAW_SIZE;
    else
    {
        width -= CHECK_MIN_SIZE - CHECK_DRAW_SIZE;
        x += (CHECK_MIN_SIZE - CHECK_DRAW_SIZE) / 2;
    }
    if (height < CHECK_MIN_SIZE)
        height = CHECK_DRAW_SIZE;
    else
    {
        height -= CHECK_MIN_SIZE - CHECK_DRAW_SIZE;
        y += (CHECK_MIN_SIZE - CHECK_DRAW_SIZE) / 2;
    }

    /* Make it square */
    if (width > height)
    {
        x += width - height;
        size = height;
    }
    else
    {
        y += height - width;
        size = width;
    }

    state = gtk_theming_engine_get_state(engine);
    gtk_theming_engine_get_background_color(engine, state, &bg);
    gtk_theming_engine_get_border_color(engine, state, &border);
    gtk_theming_engine_get_color(engine, state, &fg);

    GE_CAIRO_INIT;

    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

    cairo_rectangle (cr, x + 0.5, y + 0.5, size - 1, size - 1);

    if (!gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_MENUITEM))
    {
        /* Draw the background */
        gdk_cairo_set_source_rgba(cr, &bg);
        cairo_fill_preserve(cr);
    }

    /* Draw the border */
    gdk_cairo_set_source_rgba(cr, &border);
    cairo_stroke(cr);

    x += 1;
    y += 1;
    size -= 2;

    if (state & GTK_STATE_FLAG_INCONSISTENT)
    {
        draw_dash(cr, &fg, x, y, size);
    }
    else if (state & GTK_STATE_FLAG_ACTIVE)
    {
        b = (size + 7) / 10;
        w = ((size + 4 - b) / 6);

        /* Draw the check */
        gdk_cairo_set_source_rgba(cr, &fg);

        cairo_move_to (cr, x + b, y + floor(size / 2 - 1.5));

        cairo_line_to (cr, x + b, y + size - b);
        cairo_line_to (cr, x + b + w, y + size - b);

        cairo_line_to (cr, x + size - b, y + b + w);
        cairo_line_to (cr, x + size - b, y + b);
        cairo_line_to (cr, x + size - b + 1 - w, y + b);

        cairo_line_to (cr, x + b + w, y + size - b + 1 - 2 * w);

        cairo_line_to (cr, x + b + w, y + floor(size / 2 - 1.5));

        cairo_close_path (cr);
        cairo_fill(cr);
    }
}

static void render_option(GtkThemingEngine * engine, cairo_t * cr, gdouble x, gdouble y, gdouble width, gdouble height)
{
    GtkStateFlags state;
    GdkRGBA bg, border, fg;
    guint size;

    /* Make sure it doesn't get to small */
    if (width < CHECK_MIN_SIZE)
        width = CHECK_DRAW_SIZE;
    else
    {
        width -= CHECK_MIN_SIZE - CHECK_DRAW_SIZE;
        x += (CHECK_MIN_SIZE - CHECK_DRAW_SIZE) / 2;
    }
    if (height < CHECK_MIN_SIZE)
        height = CHECK_DRAW_SIZE;
    else
    {
        height -= CHECK_MIN_SIZE - CHECK_DRAW_SIZE;
        y += (CHECK_MIN_SIZE - CHECK_DRAW_SIZE) / 2;
    }

    /* Make it square */
    if (width > height)
    {
        x += width - height;
        size = height;
    }
    else
    {
        y += height - width;
        size = width;
    }

    state = gtk_theming_engine_get_state(engine);
    gtk_theming_engine_get_background_color(engine, state, &bg);
    gtk_theming_engine_get_border_color(engine, state, &border);
    gtk_theming_engine_get_color(engine, state, &fg);

    GE_CAIRO_INIT;

    cairo_set_antialias (cr, CAIRO_ANTIALIAS_NONE);

    /* This is required since the render layout leaves a stale cairo point */
    cairo_new_sub_path (cr);
    cairo_arc (cr, x + (size / 2.0), y + (size / 2.0), (size - 1) / 2.0, 0, 2 * M_PI);

    if (!gtk_theming_engine_has_class(engine, GTK_STYLE_CLASS_MENUITEM))
    {
        /* Draw the background */
        gdk_cairo_set_source_rgba(cr, &bg);
        cairo_fill_preserve(cr);
    }

    /* Draw the border */
    gdk_cairo_set_source_rgba(cr, &border);
    cairo_stroke(cr);

    x += 1;
    y += 1;
    size -= 2;

    if (state & GTK_STATE_FLAG_INCONSISTENT)
    {
        draw_dash(cr, &fg, x, y, size);
    }
    else if (state & GTK_STATE_FLAG_ACTIVE)
    {
        /* Draw the dot */
        gdk_cairo_set_source_rgba(cr, &fg);

        cairo_arc (cr, x + (size / 2.0), y + (size / 2.0), (size / 2.0) - ((size + 2) / 5), 0, 2 * M_PI);
        cairo_fill(cr);
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
        default:
            return;
    }

    cairo_save (cr);

    cairo_clip_extents (cr, &x0, &y_0, &x1, &y_1);
    cairo_rectangle (cr, x0, y_0, x1 - x0, ey - y_0);
    cairo_rectangle (cr, x0, ey, ex - x0, eh);
    cairo_rectangle (cr, ex + ew, ey, x1 - (ex + ew), eh);
    cairo_rectangle (cr, x0, ey + eh, x1 - x0, y_1 - (ey + eh));
    cairo_clip (cr);

    xfce_draw_frame (engine, cr, x, y, width, height, border_style);

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

    GTK_THEMING_ENGINE_GET_CLASS(engine)->render_background(engine, cr, x, y, width, height);

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
    engine_class->render_background = render_background;
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

    /* Compatibility properties */
    gtk_theming_engine_register_property(XFCE_NAMESPACE, NULL,
            g_param_spec_boxed (BUTTON_DEFAULT_BORDER,
                "Default Spacing",
                "Extra space to add for GTK_CAN_DEFAULT buttons",
                GTK_TYPE_BORDER, 0));
}

static void xfce_engine_class_finalize(XfceEngineClass * klass)
{
}

