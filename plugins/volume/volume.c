/*
 * OSS volume plugin. Will works with ALSA since it usually 
 * emulates OSS layer.
 */

#include "misc.h"
#include "../meter/meter.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#if defined __linux__
#include <linux/soundcard.h>
#endif
#define DEBUGPRN
#include "dbg.h"

static gchar *names[] = {
    "stock_volume-min",
    "stock_volume-med",
    "stock_volume-max",
};

static gchar *s_names[] = {
    "stock_volume-mute",
};
  
typedef struct {
    meter_priv meter;
    int fd, dev;
    guchar vol, muted_vol;
    int timer;
    gboolean muted;
} volume_priv;

static meter_class *k;


static gboolean
volume_get_load(volume_priv *c)
{
    guchar vols[2];

    if (c->dev == -1) {
        c->dev = SOUND_MIXER_VOLUME;
        if (!ioctl(c->fd, MIXER_READ(c->dev), &vols))
            goto ready;
        c->dev = SOUND_MIXER_PCM;
        if (!ioctl(c->fd, MIXER_READ(c->dev), &vols))
            goto ready;
        return FALSE;
    }
ready:
    ioctl(c->fd, MIXER_READ(c->dev), &vols);
    DBG("dev=%d vol=%d/%d oldvol=%d\n", c->dev, vols[0], vols[1], c->vol);
    if ((vols[0] != 0) != (c->vol != 0)) {
        if (vols[0])
            k->set_icons(&c->meter, G_N_ELEMENTS(names), names);
        else
            k->set_icons(&c->meter, G_N_ELEMENTS(s_names), s_names);
        DBG("seting %s icons\n", vols[0] ? "normal" : "muted");
    }
    c->vol = vols[0];
    k->set_level(&c->meter, (gfloat) (vols[0] / 100.0));
    RET(TRUE);
}


static gboolean
clicked(GtkWidget *widget, GdkEventButton *event, volume_priv *c)
{
    guchar vols[2];
    
    if (!(event->type == GDK_BUTTON_PRESS && event->button == 2))
        RET(FALSE);
    
    if (c->muted) {
        vols[0] = vols[1] = c->muted_vol;
    } else {
        c->muted_vol = c->vol;
        vols[0] = vols[1] = 0;
    }
    ioctl(c->fd, MIXER_WRITE(c->dev), &vols);
    c->muted = !c->muted;
    DBG("btn 2 press - %smute\n", c->muted ? "" : "un");
    volume_get_load(c);
    RET(FALSE);
}

static gboolean
scrolled(GtkWidget *widget, GdkEventScroll *event, volume_priv *c)
{
    guchar vols[2];
    gchar i;
    ENTER;

    if (c->muted) {
        i = c->muted_vol;
    } else {
        ioctl(c->fd, MIXER_READ(c->dev), &vols);
        i = vols[0];
    }
    i += 3 * ((event->direction == GDK_SCROLL_UP
            || event->direction == GDK_SCROLL_LEFT) ? 1 : -1);
    if (i > 100)
        i = 100;
    if (i < 0)
        i = 0;
    
    if (c->muted) {
        c->muted_vol = i;
    } else {
        vols[0] = vols[1] = (guchar) i;
        ioctl(c->fd, MIXER_WRITE(c->dev), &vols);
    }
    DBG("seting vol=%d\n", i);
    volume_get_load(c);
    return FALSE;
}



#if 0
static gchar *names[] = {
    "gpm-battery-000-charging",
    "gpm-battery-020-charging",
    "gpm-battery-040-charging",
    "gpm-battery-060-charging",
    "gpm-battery-080-charging",
    "gpm-battery-100-charging",
};

#endif


    
static int
volume_constructor(plugin_instance *p)
{
    volume_priv *c;
    
    if (!(k = class_get("meter")))
        RET(0);
    if (!PLUGIN_CLASS(k)->constructor(p))
        RET(0);
    c = (volume_priv *) p;
    if ((c->fd = open ("/dev/mixer", O_RDWR, 0)) < 0) {
        ERR("volume: can't open /dev/mixer\n");
        RET(0);
    }
    k->set_icons(&c->meter, G_N_ELEMENTS(names), names);
    c->timer = g_timeout_add(1000, (GSourceFunc) volume_get_load, (gpointer) c);
    c->vol = 200;
    volume_get_load(c);
    g_signal_connect(G_OBJECT(p->pwid), "scroll-event",
        G_CALLBACK(scrolled), (gpointer) c);
    g_signal_connect(G_OBJECT(p->pwid), "button_press_event",
        G_CALLBACK(clicked), (gpointer)c);

    RET(1);
}


static void
volume_destructor(plugin_instance *p)
{
    volume_priv *c = (volume_priv *) p;

    ENTER;
    g_source_remove(c->timer);
    PLUGIN_CLASS(k)->destructor(p);
    class_put("meter");
    RET();
}



static plugin_class class = {
    .count       = 0,
    .type        = "volume",
    .name        = "Volume usage",
    .version     = "1.0",
    .description = "Display volume usage",
    .priv_size   = sizeof(volume_priv),
    .constructor = volume_constructor,
    .destructor  = volume_destructor,
};

static plugin_class *class_ptr = (plugin_class *) &class;
