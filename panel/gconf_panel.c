
#include "gconf.h"

#define DEBUGPRN
#include "dbg.h"

static GtkWidget *dialog;
static GtkWidget *width_spin, *width_opt;
static GtkWidget *margin_spin;
static GtkWidget *allign_opt;

static gconf_block *gl_block;
static gconf_block *geom_block;
static gconf_block *prop_block;
static gconf_block *effects_block;

/*********************************************************
 * panel effects
 *********************************************************/
static void
effects_changed(gconf_block *b)
{
    //int i, j;
    
    ENTER;
    RET();
}
    
static void
mk_effects_block(xconf *xc)
{
    GtkWidget *w;
    
    ENTER;

    /* label */
    w = gtk_label_new(NULL);
    gtk_misc_set_alignment(GTK_MISC(w), 0, 0.5);
    gtk_label_set_markup(GTK_LABEL(w), "<b>Visual Effects</b>");
    gconf_block_add(gl_block, w, TRUE);

    /* geometry */
    effects_block = gconf_block_new((GCallback)effects_changed, xc);
    
    w = gconf_edit_boolean(effects_block, xconf_get(xc, "transparent"),
        "Transparency");
    gconf_block_add(effects_block, w, TRUE);

    w = gconf_edit_boolean(effects_block, xconf_get(xc, "roundcorners"),
        "Round corners");
    gconf_block_add(effects_block, w, TRUE);
    
    gconf_block_add(gl_block, effects_block->main, TRUE);
    
    /* empty row */
    gconf_block_add(gl_block, gtk_label_new(" "), TRUE);
}

/*********************************************************
 * panel properties
 *********************************************************/
static void
prop_changed(gconf_block *b)
{
    //int i, j;
    
    ENTER;
    RET();
}
    
static void
mk_prop_block(xconf *xc)
{
    GtkWidget *w;
    
    ENTER;

    /* label */
    w = gtk_label_new(NULL);
    gtk_misc_set_alignment(GTK_MISC(w), 0, 0.5);
    gtk_label_set_markup(GTK_LABEL(w), "<b>Properties</b>");
    gconf_block_add(gl_block, w, TRUE);

    /* properties */
    prop_block = gconf_block_new((GCallback)prop_changed, xc);
    

    gconf_block_add(gl_block, prop_block->main, TRUE);
    
    /* empty row */
    gconf_block_add(gl_block, gtk_label_new(" "), TRUE);
}

/*********************************************************
 * panel geometry
 *********************************************************/
static void
geom_changed(gconf_block *b)
{
    int i, j;
    
    ENTER;
    i = gtk_combo_box_get_active(GTK_COMBO_BOX(allign_opt));
    gtk_widget_set_sensitive(margin_spin, (i != ALLIGN_CENTER));
    i = gtk_combo_box_get_active(GTK_COMBO_BOX(width_opt));
    gtk_widget_set_sensitive(width_spin, (i != WIDTH_REQUEST));
    if (i == WIDTH_PERCENT)
    {
        j = (int) gtk_spin_button_get_value(GTK_SPIN_BUTTON(width_spin));
        if (j > 100)
            gtk_spin_button_set_value(GTK_SPIN_BUTTON(width_spin), 100);
    }
    RET();
}
    
static void
mk_geom_block(xconf *xc)
{
    GtkWidget *w;
    
    ENTER;

    /* label */
    w = gtk_label_new(NULL);
    gtk_misc_set_alignment(GTK_MISC(w), 0, 0.5);
    gtk_label_set_markup(GTK_LABEL(w), "<b>Geometry</b>");
    gconf_block_add(gl_block, w, TRUE);

    /* geometry */
    geom_block = gconf_block_new((GCallback)geom_changed, xc);
    
    w = gconf_edit_int(geom_block, xconf_get(xc, "width"), 0, 300);
    gconf_block_add(geom_block, gtk_label_new("Width"), TRUE);
    gconf_block_add(geom_block, w, FALSE);
    width_spin = w;
    
    w = gconf_edit_enum(geom_block, xconf_get(xc, "widthtype"),
        widthtype_enum);
    gconf_block_add(geom_block, w, FALSE);
    width_opt = w;
    
    w = gconf_edit_int(geom_block, xconf_get(xc, "height"), 0, 300);
    gconf_block_add(geom_block, gtk_label_new("Height"), TRUE);
    gconf_block_add(geom_block, w, FALSE);

    w = gconf_edit_enum(geom_block, xconf_get(xc, "edge"),
        edge_enum);
    gconf_block_add(geom_block, gtk_label_new("Edge"), TRUE);
    gconf_block_add(geom_block, w, FALSE);

    w = gconf_edit_enum(geom_block, xconf_get(xc, "allign"),
        allign_enum);
    gconf_block_add(geom_block, gtk_label_new("Allign"), TRUE);
    gconf_block_add(geom_block, w, FALSE);
    allign_opt = w;
    
    w = gconf_edit_int(geom_block, xconf_get(xc, "margin"), 0, 300);
    gconf_block_add(geom_block, gtk_label_new("Margin"), FALSE);
    gconf_block_add(geom_block, w, FALSE);
    margin_spin = w;

    gconf_block_add(gl_block, geom_block->main, TRUE);

    /* empty row */
    gconf_block_add(gl_block, gtk_label_new(" "), TRUE);
}
 
static GtkWidget *
mk_tab_global(xconf *xc)
{
    GtkWidget *page;
    
    ENTER;
    page = gtk_vbox_new(FALSE, 1);
    gl_block = gconf_block_new(NULL, NULL);
    gtk_box_pack_start(GTK_BOX(page), gl_block->main, FALSE, TRUE, 0);

    mk_geom_block(xc);
    mk_prop_block(xc);
    mk_effects_block(xc);
    
    gtk_widget_show_all(page);
    geom_changed(geom_block);
    
    RET(page);
}

static void
dialog_response_event(GtkDialog *_dialog, gint rid, xconf *xc)
{
    ENTER;
    if (rid == GTK_RESPONSE_APPLY ||
        rid == GTK_RESPONSE_OK)
    {
        DBG2("apply changes\n");
        xconf_prn(stdout, xconf_get(xc, "global"), 0, FALSE);
    }
    if (rid == GTK_RESPONSE_DELETE_EVENT ||
        rid == GTK_RESPONSE_CLOSE ||
        rid == GTK_RESPONSE_OK)
    {
        gtk_widget_destroy(dialog);
        dialog = NULL;
        gconf_block_free(geom_block);
        gconf_block_free(gl_block);
    }
    RET();
}

static GtkWidget *
mk_dialog(xconf *xc)
{
    GtkWidget *sw, *nb, *label;
    gchar *name;
    
    ENTER;
    DBG("creating dialog\n");
    //name = g_strdup_printf("fbpanel settings: <%s> profile", cprofile);
    name = g_strdup_printf("fbpanel settings: profile");
    dialog = gtk_dialog_new_with_buttons (name,
        NULL,
        GTK_DIALOG_NO_SEPARATOR, //GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_STOCK_APPLY,
        GTK_RESPONSE_APPLY,
        GTK_STOCK_OK,
        GTK_RESPONSE_OK,
        GTK_STOCK_CLOSE,
        GTK_RESPONSE_CLOSE,
        NULL);
    g_free(name);
    DBG("connecting sugnal to %p\n",  dialog);

    g_signal_connect (G_OBJECT(dialog), "response",
        (GCallback) dialog_response_event, xc);
#if 0    
    g_signal_connect (G_OBJECT(dialog), "destroy",
        (GCallback) dialog_destroy_event, NULL);
    g_signal_connect (G_OBJECT(dialog), "delete_event",
        (GCallback) dialog_delete_event,  NULL);
#endif
    gtk_window_set_modal(GTK_WINDOW(dialog), FALSE);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 500);
    gtk_window_set_icon_from_file(GTK_WINDOW(dialog),
        IMGPREFIX "/logo.png", NULL);
      
    nb = gtk_notebook_new();
    gtk_notebook_set_show_border (GTK_NOTEBOOK(nb), FALSE);
    gtk_container_add (GTK_CONTAINER (GTK_DIALOG(dialog)->vbox), nb);

    sw = mk_tab_global(xconf_get(xc, "global"));
    label = gtk_label_new("General");
    gtk_misc_set_padding(GTK_MISC(label), 4, 1);
    gtk_notebook_append_page(GTK_NOTEBOOK(nb), sw, label);

    gtk_widget_show_all(dialog);
    RET(dialog);
}

void
configure(xconf *xc)
{
    ENTER;
    DBG("dialog %p\n",  dialog);
    if (!dialog) 
        dialog = mk_dialog(xc);
    gtk_widget_show(dialog);
    RET();
}
