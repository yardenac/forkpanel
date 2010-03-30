
#include "plugin.h"
#include "panel.h"
#include "meter.h"


//#define DEBUGPRN
#include "dbg.h"
float roundf(float x);


static void
meter_set_level(meter_priv *m, gfloat level)
{
    int i;
    GdkPixbuf *pb;
    
    i = roundf(level * m->num);
    if (i != m->cur_icon) {
        m->cur_icon = i;
        pb = gtk_icon_theme_load_icon(icon_theme, m->icons[i],
            m->size, GTK_ICON_LOOKUP_FORCE_SIZE, NULL);
        gtk_image_set_from_pixbuf(GTK_IMAGE(m->meter), pb);
        g_object_ref(G_OBJECT(pb));
    }
    m->level = level;
}

static void
meter_set_icons(meter_priv *m, gchar *th_icon, int num, gchar **icons)
{
    m->num = num;
    m->icons = icons;
    m->cur_icon = -1;
}

static int
meter_constructor(plugin_instance *p)
{
    meter_priv *m;
    
    ENTER;
    m = (meter_priv *) p;
    m->meter = gtk_image_new();
    gtk_misc_set_alignment(GTK_MISC(m->meter), 0.5, 0.5);
    gtk_misc_set_padding(GTK_MISC(m->meter), 0, 0);
    m->cur_icon = -1;
    m->size = 24;
    // m->itc_id = g_signal_connect_after(G_OBJECT(icon_theme),
    // "changed", (GCallback) update_view, m);
    RET(1);
}

static void
meter_destructor(plugin_instance *p)
{
    meter_priv *m = (meter_priv *) p;

    ENTER;
    //g_signal_handler_disconnect(G_OBJECT(icon_theme), m->itc_id);
    RET();
}

meter_class class = {
    .plugin = {
        .type        = "meter",
        .name        = "Meter",
        .description = "Basic meter plugin",
        .priv_size   = sizeof(meter_priv),

        .constructor = meter_constructor,
        .destructor  = meter_destructor,
    },
    .set_level = meter_set_level,
    .set_icons = meter_set_icons,
};


static plugin_class *class_ptr = (plugin_class *) &class;
