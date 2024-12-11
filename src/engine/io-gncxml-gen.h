/********************************************************************\
 * gnc-xml-gen.h -- api for gnucash xml i/o                         *
 *                                                                  *
 * Copyright (C) 2001 James LewisMoss <dres@debian.org>             *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 59 Temple Place - Suite 330        Fax:    +1-617-542-2652       *
 * Boston, MA  02111-1307,  USA       gnu@gnu.org                   *
 *                                                                  *
\********************************************************************/

#ifndef __IO_GNCXML_GEN_H__
#define __IO_GNCXML_GEN_H__

#include <glib.h>
#include "sixtp.h"

typedef gboolean (*gxpf_callback)(const char *tag, gpointer globaldata,
                                  gpointer data);

struct gxpf_data_struct
{
    gxpf_callback cb;
    gpointer data;
};

typedef struct gxpf_data_struct gxpf_data;

gboolean
gnc_xml_parse_file(sixtp *top_parser, const char *filename,
                   gxpf_callback callback, gpointer data);

#endif /* __IO_GNCXML_GEN_H__ */