/*
 * net usage plugin to fbpanel
 *
 * Copyright (C) 2004 by Alexandre Pereira da Silva <alexandre.pereira@poli.usp.br>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * 
 */
/*A little bug fixed by Mykola <mykola@2ka.mipt.ru>:) */


#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <sys/sysinfo.h>
#include <stdlib.h>

#include "misc.h"
#include "../chart/chart.h"

//#define DEBUG
#include "dbg.h"



/* net.c */
struct net_stat {
    gulong tx, rx;
};

typedef struct {
    chart_t chart;
    struct net_stat net_prev;
    int timer;
    char *iface;
    gulong max_tx;
    gulong max_rx;
} net_t;

static chart_class_t *k;

static int
net_get_load(net_t *c)
{
    struct net_stat net, net_diff;
    FILE *stat;
    float total[2];
    char buf[256], *s = NULL;

    ENTER;
    stat = fopen("/proc/net/dev", "r");
    if(!stat)
        RET(TRUE);
    fgets(buf, 256, stat);
    fgets(buf, 256, stat);

    while (!s && !feof(stat) && fgets(buf, 256, stat))  
        s = g_strrstr(buf, c->iface);
    fclose(stat);
    if (!s)
        RET(0);
    s = g_strrstr(s, ":");     
    if (!s)
        RET(0);
    s++;
    if (sscanf(s, "%lu  %*d     %*d  %*d  %*d  %*d   %*d        %*d       %lu", &net.tx, &net.rx)
          != 2) {
        DBG("can't read %s statistics\n", c->iface);
        RET(0);
    }
    net_diff.tx = (net.tx - c->net_prev.tx) >> 10;
    net_diff.rx = (net.rx - c->net_prev.rx) >> 10;
    c->net_prev = net;
    total[0] = (float)(net_diff.tx) / (float)(c->max_tx + c->max_rx);
    total[1] = (float)(net_diff.rx) / (float)(c->max_tx + c->max_rx);
    DBG("%f %ul %ul\n", total, net_diff.tx, net_diff.rx);
    k->add_tick(&c->chart, total);
    
    RET(TRUE);

}


static int
net_constructor(plugin *p)
{
    net_t *c;
    gchar *colors[] = { "blue", "violet" };

    if (!(k = class_get("chart")))
        RET(0);
    c = p->priv = g_new0(net_t, 1);
    if (!k->constructor(p))
        RET(0);
    k->set_rows(&c->chart, 2, colors);
    c->timer = g_timeout_add(1000, (GSourceFunc) net_get_load, (gpointer) c);
    c->iface = "eth0";
    c->max_rx = 120;
    c->max_tx = 12;
    RET(1);
}


static void
net_destructor(plugin *p)
{
    net_t *c = (net_t *) p->priv;

    ENTER;
    g_source_remove(c->timer);
    k->destructor(p);
    g_free(p->priv);
    class_put("chart");
    RET();
}


plugin_class class = {
    .count       = 0,
    .type        = "net",
    .name        = "net usage",
    .version     = "1.0",
    .description = "Display net usage",

    .constructor = net_constructor,
    .destructor  = net_destructor,
};
