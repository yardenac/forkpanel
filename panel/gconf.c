
#include "gconf.h"

#define DEBUGPRN
#include "dbg.h"

#define INDENT_SIZE 20


gconf_block *
gconf_block_new(GCallback cb, gpointer data)
{
    GtkWidget *w;
    gconf_block *b;

    b = g_new0(gconf_block, 1);
    b->cb = cb;
    b->data = data;
    b->main = gtk_hbox_new(FALSE, 0);
    /* indent */
    w = gtk_hbox_new(FALSE, 0);
    gtk_widget_set_size_request(w, INDENT_SIZE, -1);
    gtk_box_pack_start(GTK_BOX(b->main), w, FALSE, FALSE, 0);
    /* area */
    w = gtk_vbox_new(FALSE, 2);
    gtk_box_pack_start(GTK_BOX(b->main), w, FALSE, FALSE, 0);
    b->area = w;

    b->sgr = gtk_size_group_new(GTK_SIZE_GROUP_HORIZONTAL);
    return b;
}

void
gconf_block_free(gconf_block *b)
{
    g_object_unref(b->sgr);
    g_slist_free(b->rows);
    g_free(b);
}

void
gconf_block_add(gconf_block *b, GtkWidget *w, gboolean new_row)
{
    GtkWidget *hbox;
    
    if (!b->rows || new_row)
    {
        GtkWidget *s;
        
        new_row = TRUE;
        hbox = gtk_hbox_new(FALSE, 8);
        b->rows = g_slist_prepend(b->rows, hbox);
        gtk_box_pack_start(GTK_BOX(b->area), hbox, FALSE, FALSE, 0);
        /* space */
        s = gtk_vbox_new(FALSE, 0);
        gtk_box_pack_end(GTK_BOX(hbox), s, TRUE, TRUE, 0);

        /* allign first elem */
        if (GTK_IS_MISC(w))
        {
            DBG2("misc \n");
            gtk_misc_set_alignment(GTK_MISC(w), 0, 0.5);
            gtk_size_group_add_widget(b->sgr, w);
        }
    }
    else
        hbox = b->rows->data;
    gtk_box_pack_start(GTK_BOX(hbox), w, FALSE, FALSE, 0);
}

/*********************************************************
 * Edit int
 *********************************************************/
static void
gconf_edit_int_cb(GtkSpinButton *w, xconf *xc)
{
    int i;
    
    i = (int) gtk_spin_button_get_value(w);
    xconf_set_int(xc, i);
}

GtkWidget *
gconf_edit_int(gconf_block *b, xconf *xc, int min, int max)
{
    gint i = 0;
    GtkWidget *w;

    xconf_get_int(xc, &i);
    w = gtk_spin_button_new_with_range((gdouble) min, (gdouble) max, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(w), (gdouble) i);
    g_signal_connect(G_OBJECT(w), "value-changed",
        G_CALLBACK(gconf_edit_int_cb), xc);
    if (b && b->cb)
    {
        g_signal_connect_swapped(G_OBJECT(w), "value-changed",
            G_CALLBACK(b->cb), b);
    }
    return w;
}

/*********************************************************
 * Edit enum
 *********************************************************/
static void
gconf_edit_enum_cb(GtkComboBox *w, xconf *xc)
{
    int i;
    
    i = gtk_combo_box_get_active(w);
    DBG2("%s=%d\n", xc->name, i);
    xconf_set_enum(xc, i, g_object_get_data(G_OBJECT(w), "enum"));
}

GtkWidget *
gconf_edit_enum(gconf_block *b, xconf *xc, xconf_enum *e)
{
    gint i = 0;
    GtkWidget *w;

    xconf_get_enum(xc, &i, e);

    w = gtk_combo_box_new_text();
    g_object_set_data(G_OBJECT(w), "enum", e);
    while (e && e->str)
    {
        gtk_combo_box_insert_text(GTK_COMBO_BOX(w), e->num,
            e->desc ? e->desc : e->str);
        e++;
    }
    gtk_combo_box_set_active(GTK_COMBO_BOX(w), i);
    g_signal_connect(G_OBJECT(w), "changed",
        G_CALLBACK(gconf_edit_enum_cb), xc);
    if (b && b->cb)
    {
        g_signal_connect_swapped(G_OBJECT(w), "changed",
            G_CALLBACK(b->cb), b);
    }

    return w;
}

/*********************************************************
 * Edit boolean
 *********************************************************/
static void
gconf_edit_bool_cb(GtkToggleButton *w, xconf *xc)
{
    int i;
    
    i = gtk_toggle_button_get_active(w);
    DBG2("%s=%d\n", xc->name, i);
    xconf_set_enum(xc, i, bool_enum);
}

GtkWidget *
gconf_edit_boolean(gconf_block *b, xconf *xc, gchar *text)
{
    gint i = 0;
    GtkWidget *w;

    xconf_get_enum(xc, &i, bool_enum);
    w = gtk_check_button_new_with_label(text);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(w), i);
    
    g_signal_connect(G_OBJECT(w), "toggled",
        G_CALLBACK(gconf_edit_bool_cb), xc);
    if (b && b->cb)
    {
        g_signal_connect_swapped(G_OBJECT(w), "toggled",
            G_CALLBACK(b->cb), b);
    }

    return w;
}
