/********************************************************************\
 * io-gncxml-v2.h -- api for gnucash xml i/o                        *
 *                                                                  *
 * Copyright (c) 2001 Gnumatic Incorporated                         *
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

/*
 * io-gncxml.h -- api for new XML-based file format
 *
 * Initial code by James LewisMoss
 */

#ifndef __IO_GNCXML_V2_H__
#define __IO_GNCXML_V2_H__

#include <glib.h>

#include "gnc-book.h"
#include "Account.h"
#include "Transaction.h"
#include "gnc-commodity.h"
#include "gnc-pricedb.h"

struct _load_counter_struct
{
    int accounts_total;
    int accounts_loaded;

    int commodities_total;
    int commodities_loaded;

    int transactions_total;
    int transactions_loaded;

    int prices_total;
    int prices_loaded;
};

typedef struct _load_counter_struct load_counter;

struct sixtp_global_data_v2_struct
{
    GNCBook *book;
    load_counter counter;
    void (*countCallback)(const char *type, load_counter counter);
};

typedef struct sixtp_global_data_v2_struct sixtp_gdv2;

/* read in an account group from a file */
gboolean gnc_book_load_from_xml_file_v2(
    GNCBook *book,
    void (*countcallback)(const char *type, load_counter count));

/* write all account info to a file */
gboolean gnc_book_write_to_xml_file_v2(GNCBook *book, const char *filename);

/* The is_gncxml_file() routine checks to see if the first few 
 * chars of the file look like gnc-xml data.
 */
gboolean gnc_is_xml_data_file_v2(const gchar *name);


#endif /* __IO_GNCXML_V2_H__ */