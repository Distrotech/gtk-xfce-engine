#include <gtk/gtkrc.h>

typedef struct _XfceRcStyle XfceRcStyle;
typedef struct _XfceRcStyleClass XfceRcStyleClass;

extern GType xfce_type_rc_style;

#define XFCE_TYPE_RC_STYLE              xfce_type_rc_style
#define XFCE_RC_STYLE(object)           (G_TYPE_CHECK_INSTANCE_CAST ((object), XFCE_TYPE_RC_STYLE, XfceRcStyle))
#define XFCE_RC_STYLE_CLASS(klass)      (G_TYPE_CHECK_CLASS_CAST ((klass), XFCE_TYPE_RC_STYLE, XfceRcStyleClass))
#define XFCE_IS_RC_STYLE(object)        (G_TYPE_CHECK_INSTANCE_TYPE ((object), XFCE_TYPE_RC_STYLE))
#define XFCE_IS_RC_STYLE_CLASS(klass)   (G_TYPE_CHECK_CLASS_TYPE ((klass), XFCE_TYPE_RC_STYLE))
#define XFCE_RC_STYLE_GET_CLASS(obj)    (G_TYPE_INSTANCE_GET_CLASS ((obj), XFCE_TYPE_RC_STYLE, XfceRcStyleClass))

struct _XfceRcStyle
{
  GtkRcStyle parent_instance;

  guint scrollbar_type:1;
  guint scrollbar_marks:1;
  guint scroll_button_marks:1;
  guint handlebox_marks:1;
  guint mark_type1;
  guint mark_type2;
};

struct _XfceRcStyleClass
{
  GtkRcStyleClass parent_class;
};

void xfce_rc_style_register_type (GTypeModule *module);

/* Default stuff */
#define DEFAULT_SCROLLTHUMB_SIZE  12
#define DEFAULT_MIN_SLIDER_SIZE   9
#define SMALLEST_HANDLE           17
