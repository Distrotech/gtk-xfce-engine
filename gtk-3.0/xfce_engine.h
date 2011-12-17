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
 *                       Tomas Ã–gren <stric@ing.umu.se>
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

typedef struct _XfceEngine XfceEngine;
typedef struct _XfceEngineClass XfceEngineClass;

#define XFCE_TYPE_ENGINE              xfce_engine_get_type()
#define XFCE_ENGINE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), XFCE_TYPE_ENGINE, XfceEngine))
#define XFCE_ENGINE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_TYPE_ENGINE, XfceEngineClass))
#define XFCE_IS_ENGINE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), XFCE_TYPE_ENGINE))
#define XFCE_IS_ENGINE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_TYPE_ENGINE))
#define XFCE_ENGINE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_TYPE_ENGINE, XfceEngineClass))

struct _XfceEngine
{
    GtkThemingEngine parent_instance;
};

struct _XfceEngineClass
{
    GtkThemingEngineClass parent_class;
};

GType xfce_engine_get_type(void);

void xfce_engine_register_types(GTypeModule * module);
