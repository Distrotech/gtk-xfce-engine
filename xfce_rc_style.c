#include "xfce_rc_style.h"
#include "xfce_style.h"

static void      xfce_rc_style_init         (XfceRcStyle      *style);
static void      xfce_rc_style_class_init   (XfceRcStyleClass *klass);
static void      xfce_rc_style_finalize     (GObject             *object);

static GtkStyle *xfce_rc_style_create_style (GtkRcStyle          *rc_style);

static GtkRcStyleClass *parent_class;

GType xfce_type_rc_style = 0;

void
xfce_rc_style_register_type (GTypeModule *module)
{
  static const GTypeInfo object_info =
  {
    sizeof (XfceRcStyleClass),
    (GBaseInitFunc) NULL,
    (GBaseFinalizeFunc) NULL,
    (GClassInitFunc) xfce_rc_style_class_init,
    NULL,           /* class_finalize */
    NULL,           /* class_data */
    sizeof (XfceRcStyle),
    0,              /* n_preallocs */
    (GInstanceInitFunc) xfce_rc_style_init,
  };
  
  xfce_type_rc_style = g_type_module_register_type (module, GTK_TYPE_RC_STYLE, "XfceRcStyle", &object_info, 0);
}

static void
xfce_rc_style_init (XfceRcStyle *style)
{
}

static void
xfce_rc_style_class_init (XfceRcStyleClass *klass)
{
  GtkRcStyleClass *rc_style_class = GTK_RC_STYLE_CLASS (klass);
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);
  rc_style_class->create_style = xfce_rc_style_create_style;
}

/* Create an empty style suitable to this RC style
 */
static GtkStyle *
xfce_rc_style_create_style (GtkRcStyle *rc_style)
{
  void *ptr;
  ptr = GTK_STYLE (g_object_new (XFCE_TYPE_STYLE, NULL));
  return (GtkStyle *)ptr;
}
