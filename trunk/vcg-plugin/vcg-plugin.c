/* Vcg plugin internal common routines.

   Copyright (C) 2009, 2010, 2011 Mingjie Xing, mingjie.xing@gmail.com. 

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#include <config.h>

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#include "gcc-plugin.h"
#include "plugin.h"
#include "plugin-version.h"

#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "toplev.h"
#include "gimple.h"
#include "tree-pass.h"
#include "intl.h"
#include "langhooks.h"
#include "cfghooks.h"

#include "libiberty.h"
#include "vcg-plugin.h"
#include "gdl.h"

static struct obstack str_obstack;

static void vcg_error (const char *format, ...);
static char *vcg_get_file_name (bool is_temp);
static void vcg_dump (char *fname, gdl_graph *graph);
static void vcg_show (char *fname);

static void
vcg_error (const char *format, ...)
{
  va_list ap;

  va_start (ap, format);
  fprintf (stderr, "%s: error: ", vcg_plugin_common.plugin_name);
  vfprintf (stderr,  format, ap);
  va_end (ap);
  fputc ('\n', stderr);
}

/* Return the dump file name which is going to be used.  Return NULL if there
   is error.  IS_TEMP is true if we want a temp file.  */
static char *
vcg_get_file_name (bool is_temp)
{
  char *str;
  static unsigned int file_number = 0;

  if (is_temp)
    {
      str = "dump-temp.vcg";
    }
  else
    {
      if (asprintf (&str, "dump-%03d.vcg", file_number) < 0)
        return NULL;

      file_number++;
    }
  return str;
}

/* Dump GRAPH into file.  If FP is NULL, then create a new file.  */
static void
vcg_dump (char *fname, gdl_graph *graph)
{
  FILE *fp;

  assert (graph != NULL);

  if ((fp = fopen (fname, "w")) == NULL)
    {
      vcg_plugin_common.error ("failed to open file %s.", fname);
      return;
    }

  /* Put the version information on top.  */
  fprintf (fp, "// Generated by GCC VCG Plugin %s\n" \
               "// Report bugs to <mingjie.xing@gmail.com>\n" \
               "// Home page: http://code.google.com/p/gcc-vcg-plugin\n" \
               "// %s", vcg_plugin_common.version, vcg_plugin_common.info);

  gdl_dump_graph (fp, graph);
  fclose (fp);
}

static void
vcg_show (char *fname)
{
  char *cmd;
  pid_t pid;

  assert (fname != NULL);

  cmd = concat (vcg_plugin_common.vcg_viewer, " ", fname, NULL);
  pid = fork ();
  if (pid == 0)
    {
      system (cmd);
      exit (0);
    }
}

typedef struct vcg_malloc_str vcg_malloc_str;
struct vcg_malloc_str
{
  char *str;
  vcg_malloc_str *next;
};

static vcg_malloc_str malloc_str;
static vcg_malloc_str *current_malloc_str;

static void
vcg_init (void)
{
  gdl_graph *graph;
  gdl_node *node;

  malloc_str.str = NULL;
  malloc_str.next = NULL;  
  current_malloc_str = &malloc_str;

  graph = gdl_new_graph ("top graph");
  gdl_set_graph_node_borderwidth (graph, 1);
  gdl_set_graph_edge_thickness (graph, 1);
  gdl_set_graph_splines (graph, "yes");
  gdl_set_graph_colorentry (graph, 100, 70, 130, 180);
  gdl_set_graph_node_textcolor (graph, "white");
  gdl_set_graph_node_color (graph, "100");
  gdl_set_graph_edge_color (graph, "100");

//  node = gdl_new_node ("info");
//  gdl_set_node_label (node, vcg_plugin_common.info);
//  gdl_set_node_borderwidth (node, 1);
//  gdl_add_node (graph, node);

  vcg_plugin_common.top_graph = graph;

  obstack_init (&str_obstack);
}

static void
vcg_tag (char *str)
{
  vcg_malloc_str *ms;

  ms = (vcg_malloc_str *) xmalloc (sizeof (vcg_malloc_str));
  ms->str = str;
  ms->next = NULL;
  current_malloc_str->next = ms;
  current_malloc_str = ms;
}

static void
vcg_finish (void)
{
  vcg_malloc_str *ms;

  for (ms = &malloc_str; ms;)
    {
      current_malloc_str = ms;
      ms = current_malloc_str->next;
      free (current_malloc_str->str);
      //free (current_malloc_str);
    }
  gdl_free_graph (vcg_plugin_common.top_graph);

  obstack_free (&str_obstack, NULL);
}

static void
vcg_buf_print (char *fmt, ...)
{
  va_list ap;
  char buf[256];

  va_start (ap, fmt);
  vsprintf (buf, fmt, ap);
  va_end (ap);
  obstack_grow (&str_obstack, buf, strlen (buf));
}

static char *
vcg_buf_finish (void)
{
  obstack_1grow (&str_obstack, '\0');
  return obstack_finish (&str_obstack);
}

vcg_plugin_common_t vcg_plugin_common =
{
  "VCG Plugin",
  "1.0",
  "",
  "vcgview",
  "dump-temp.vcg",
  NULL,
  vcg_init,
  vcg_finish,
  vcg_error,
  vcg_dump,
  vcg_show,
  vcg_tag,
  vcg_buf_print,
  vcg_buf_finish
};
