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
 *
 *  Portions based Thinice port by 
 *                       Tim Gerla <timg@rrv.net>,
 *                       Tomas �gren <stric@ing.umu.se,
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

#ifndef INC_GRADIENT_DRAW_H
#define INC_GRADIENT_DRAW_H

typedef enum
{
    GRADIENT_HORIZONTAL,
    GRADIENT_VERTICAL,
    GRADIENT_NORTHERN_DIAGONAL,
    GRADIENT_SOUTHERN_DIAGONAL
} GradientType;

#define GRADIENT(horizontal) (horizontal ? GRADIENT_HORIZONTAL : GRADIENT_VERTICAL)
#define DIAGONALGRADIENT(northern) (northern ? GRADIENT_NORTHERN_DIAGONAL : GRADIENT_SOUTHERN_DIAGONAL)

void gradient_alloc_color(GdkColor * color, GdkColormap * colormap, GdkColor light, GdkColor dark, gint position, gint steps);

void gradient_shade(GdkColor * a, GdkColor * b, gdouble k);

void gradient_draw(GdkWindow * window, GdkGC * gc, GdkColormap * colormap, GdkRectangle * area, gint x, gint y, gint width, gint height, GdkColor light, GdkColor dark, GradientType gradient_style, gboolean noclip);

void gradient_draw_shaded(GdkWindow * window, GdkGC * gc, GdkColormap * colormap, GdkRectangle * area, gint x, gint y, gint width, gint height, GdkColor color, gfloat shine_value, gfloat shade_value, GradientType gradient_style, gboolean noclip);

#endif /* INC_GRADIENT_DRAW_H */
