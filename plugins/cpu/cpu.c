/*
 * CPU usage plugin to fbpanel
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

//#define DEBUGPRN
#include "dbg.h"



/* cpu.c */
struct cpu_stat {
    gulong u, n, s, i, w; // user, nice, system, idle, wait
};

typedef struct {
    chart_priv chart;
    struct cpu_stat cpu_prev;
    int timer;
} cpu_priv;



static chart_class *k;

static int
cpu_get_load(cpu_priv *c)
{
    gfloat a, b;
    struct cpu_stat cpu, cpu_diff;
    FILE *stat;
    float total;

    ENTER;
    stat = fopen("/proc/stat", "r");
    if(!stat)
        RET(TRUE);
    fscanf(stat, "cpu %lu %lu %lu %lu %lu", &cpu.u, &cpu.n, &cpu.s, &cpu.i, &cpu.w);
    fclose(stat);

    cpu_diff.u = cpu.u - c->cpu_prev.u;
    cpu_diff.n = cpu.n - c->cpu_prev.n;
    cpu_diff.s = cpu.s - c->cpu_prev.s;
    cpu_diff.i = cpu.i - c->cpu_prev.i;
    cpu_diff.w = cpu.w - c->cpu_prev.w;
    c->cpu_prev = cpu;

    a = cpu_diff.u + cpu_diff.n + cpu_diff.s;
    b = a + cpu_diff.i + cpu_diff.w;
    total = b ?  a / b : 1.0;
    DBG("total=%f a=%f b=%f\n", total, a, b);
    k->add_tick(&c->chart, &total);
    RET(TRUE);

}


static int
cpu_constructor(plugin_instance *p)
{
    cpu_priv *c;
    char *colors[] = { "green" };

    if (!(k = class_get("chart")))
        RET(0);
    c = p->priv = g_new0(cpu_priv, 1);
    if (!PLUGIN_CLASS(k)->constructor(p))
        RET(0);
    k->set_rows(&c->chart, 1, colors);
    c->timer = g_timeout_add(1000, (GSourceFunc) cpu_get_load, (gpointer) c);
    RET(1);
}


static void
cpu_destructor(plugin_instance *p)
{
    cpu_priv *c = (cpu_priv *) p->priv;

    ENTER;
    g_source_remove(c->timer);
    PLUGIN_CLASS(k)->destructor(p);
    g_free(p->priv);
    class_put("chart");
    RET();
}



plugin_class class = {
    .count       = 0,
    .type        = "cpu",
    .name        = "Cpu usage",
    .version     = "1.0",
    .description = "Display cpu usage",
    .priv_size   = sizeof(cpu_priv),
    .constructor = cpu_constructor,
    .destructor  = cpu_destructor,
};
