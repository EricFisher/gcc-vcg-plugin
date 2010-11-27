/* vcg plugin

   Copyright (C) 2009, 2010 Mingjie Xing, mingjie.xing@gmail.com. 

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef VCG_PLUGIN_H
#define VCG_PLUGIN_H

/* gcc's header files */

#include "system.h"
#include "coretypes.h"
#include "tm.h"
#include "toplev.h"
#include "gimple.h"
#include "tree-pass.h"
#include "rtl.h"
#include "intl.h"
#include "langhooks.h"
#include "cfghooks.h"

/* libgdl */
#include "gdl.h"

/* dump file names */


struct vcg_plugin
{
  char *plugin_name;
  char *plugin_version;
  char *vcg_viewer;
  char *vcg_filename;
  int (*init) (int argc, char *argv[]);
  void (*dump) (void);
  void (*show) (void);
};

extern struct vcg_plugin vcg; 

/* These are available functins, which can be used to
   dump and view gcc internal data structures. */

extern void vcg_plugin_dump_function (tree fn, int flags); 
extern void vcg_plugin_view_function (tree fn, int flags); 

extern void vcg_plugin_view (char *filename);

#endif