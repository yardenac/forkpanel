#ifndef MISC_H
#define MISC_H

#include <X11/Xatom.h>
#include <gtk/gtk.h>
#include <gdk/gdk.h>
#include <gdk/gdkx.h>
#include <stdio.h>

#include "panel.h"

enum { LINE_NONE, LINE_BLOCK_START, LINE_BLOCK_END, LINE_VAR };

typedef struct {
    int num, len, type;
    gchar str[256];
    gchar *t[3];
} line;


typedef struct {
    int num;
    gchar *str;
} pair;

extern pair allign_pair[];
extern pair edge_pair[];
extern pair width_pair[];
extern pair height_pair[];
extern pair bool_pair[];
extern pair pos_pair[];
extern pair layer_pair[];

int str2num(pair *p, gchar *str, int defval);
gchar *num2str(pair *p, int num, gchar *defval);
extern int get_line(FILE *fp, line *s);
int get_line_as_is(FILE *fp, line *s);

void Xclimsg(Window win, long type, long l0, long l1, long l2, long l3, long l4);
void Xclimsgwm(Window win, Atom type, Atom arg);
void *get_xaproperty (Window win, Atom prop, Atom type, int *nitems);
char *get_textproperty(Window win, Atom prop);
void *get_utf8_property(Window win, Atom atom);
char **get_utf8_property_list(Window win, Atom atom, int *count);

void resolve_atoms();
//Window Select_Window(Display *dpy);
guint get_net_number_of_desktops();
guint get_net_current_desktop ();
guint get_net_wm_desktop(Window win);
void get_net_wm_state(Window win, net_wm_state *nws);
void get_net_wm_window_type(Window win, net_wm_window_type *nwwt);

void calculate_position(panel *np);
gchar *expand_tilda(gchar *file);
GdkPixbuf *gdk_pixbuf_scale_ratio(GdkPixbuf *p, int width, int height, GdkInterpType itype,
                                  gboolean keep_ratio);

void get_button_spacing(GtkRequisition *req, GtkContainer *parent, gchar *name);
guint32 gcolor2rgb24(GdkColor *color);
/*
GtkWidget *fb_button_new_from_file(gchar *fname, int width, int height, gulong hicolor,
      gboolean keep_ratio);
*/
GtkWidget *fb_button_new_from_icon_file(gchar *iname, gchar *fname, int width, int height,
      gulong hicolor, gboolean keep_ratio);

GtkWidget *fb_button_new_from_icon_file_with_label(gchar *iname, gchar *fname, int width, int height,
      gulong hicolor, gboolean keep_ratio, gchar *name);
GdkPixbuf *fb_pixbuf_new_from_icon_file(gchar *iname, gchar *fname, int width, int height);
GtkWidget *fb_image_new(gchar *iname, gchar *fname, int width, int height,
      gboolean keep_ratio);
void menu_pos(GtkMenu *menu, gint *x, gint *y, gboolean *push_in, GtkWidget *widget);

void configure();
gchar *indent(int level);

#endif
