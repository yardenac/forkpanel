#ifndef _XCONF_H_
#define _XCONF_H_

#include <glib.h>

typedef struct _xconf
{
    gchar *name;
    gchar *value;
    GSList *sons;
    struct _xconf *parent;
} xconf;

typedef struct {
    gchar *str;
    int num;
} xconf_enum;

xconf *xconf_new(gchar *name, gchar *value);
void xconf_link(xconf *parent, xconf *son);
void xconf_unlink(xconf *x);
void xconf_del(xconf *x, gboolean sons_only);
void xconf_set_value(xconf *x, gchar *value);
gchar *xconf_get_value(xconf *x);
void xconf_prn(FILE *fp, xconf *x, int n, gboolean sons_only);
xconf *xconf_find(xconf *x, gchar *name, int no);


xconf *xconf_new_from_file(gchar *name);
xconf *xconf_new_from_profile(gchar *profile);
void xconf_save_to_profile(gchar *profile, xconf *xc);
    
void xconf_get_int(xconf *x, int *val);
void xconf_get_enum(xconf *x, int *val, xconf_enum *e);
void xconf_get_str(xconf *x, gchar **val);

void xconf_set_int(xconf *x, int val);
void xconf_set_enum(xconf *x, int val, xconf_enum *e);

#endif
