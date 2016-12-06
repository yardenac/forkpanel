#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "panel.h"
#include "misc.h"
#include "plugin.h"

//#define DEBUG
#include "dbg.h"

typedef struct {
    plugin_instance plugin;
    int time;
    int timer;
    int max_text_len;
    char *command;
    char *textfmt;
    GtkWidget *main;
} inmon_priv;

static int
text_update(inmon_priv *im)
{
    FILE *fp;
    char text[256];
    char *markup;
    int len, ret;

    ENTER;
    fp = popen(im->command, "r");
    if (fgets(text, sizeof(text), fp));
    pclose(fp);
    len = strlen(text) - 1;
    if (len >= 0) {
        if (text[len] == '\n')
            text[len] = 0;

        markup = g_markup_printf_escaped(FMT, im->textfmt, text);
        gtk_label_set_markup (GTK_LABEL(im->main), markup);
        g_free(markup);
    }
    RET(TRUE);
}

static void
inmon_destructor(plugin_instance *p)
{
    inmon_priv *im = (inmon_priv *) p;

    ENTER;
    if (im->timer) {
        g_source_remove(im->timer);
    }
    RET();
}

static int
inmon_constructor(plugin_instance *p)
{
    inmon_priv *im;

    ENTER;
    im = (inmon_priv *) p;
    im->command = "date +%R";
    im->time = 1;
    im->textfmt = "<span>%s</span>";
    im->max_text_len = 30;

    XCG(p->xc, "Command", &im->command, str);
    XCG(p->xc, "TextFmt", &im->textfmt, str);
    XCG(p->xc, "PollingTime", &im->time, int);
    XCG(p->xc, "MaxTextLength", &im->max_text_len, int);

    im->main = gtk_label_new(NULL);
    gtk_label_set_max_width_chars(GTK_LABEL(im->main), im->max_text_len);
    text_update(im);
    gtk_container_set_border_width (GTK_CONTAINER (p->pwid), 1);
    gtk_container_add(GTK_CONTAINER(p->pwid), im->main);
    gtk_widget_show_all(p->pwid);
    im->timer = g_timeout_add((guint) im->time * 1000,
        (GSourceFunc) text_update, (gpointer) im);

    RET(1);
}


static plugin_class class = {
    .count       = 0,
    .type        = "inmon",
    .name        = "Inotify Monitor",
    .version     = "0.2",
    .description = "genmon with inotify waits instead of looping",
    .priv_size   = sizeof(inmon_priv),

    .constructor = inmon_constructor,
    .destructor  = inmon_destructor,
};
static plugin_class *class_ptr = (plugin_class *) &class;
