/********************************************************************\
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

#ifndef GNUCASH_STYLE_H
#define GNUCASH_STYLE_H

#include <gnome.h>

#include "splitreg.h"
#include "gnucash-sheet.h"

typedef struct
{
        gint pixel_height;
        gint pixel_width;

        gint origin_x;
        gint origin_y;

        gboolean can_span_over;
} CellDimensions;

typedef struct
{
        gint nrows;
        gint ncols;

        /* totals, in pixels */
        gint height;
        gint width;

        /* per cell parameters */
        GTable *cell_dimensions;

        gint refcount;
} BlockDimensions;

struct _SheetBlockStyle
{
        CellBlock * cursor;

        gint nrows;
        gint ncols;

        CursorType cursor_type;

        BlockDimensions *dimensions;

        gint refcount;
};


void gnucash_style_init (void);

void gnucash_sheet_style_init(void);

void gnucash_style_set_register_font_name(const char *name);
void gnucash_style_set_register_hint_font_name(const char *name);

const char * gnucash_style_get_default_register_font_name(void);
const char * gnucash_style_get_default_register_hint_font_name(void);

gint gnucash_style_col_is_resizable (SheetBlockStyle *style, int col);

CellDimensions * gnucash_style_get_cell_dimensions (SheetBlockStyle *style,
                                                    int row, int col);

void gnucash_sheet_set_col_width (GnucashSheet *sheet, int col, int width);

gint gnucash_style_row_width(SheetBlockStyle *style, int row);

void gnucash_sheet_styles_set_dimensions (GnucashSheet *sheet, int width);

void gnucash_sheet_style_destroy (GnucashSheet *sheet, SheetBlockStyle *style);

void gnucash_sheet_create_styles (GnucashSheet *sheet);

void gnucash_sheet_compile_styles (GnucashSheet *sheet);

void gnucash_sheet_styles_recompile (GnucashSheet *sheet);

SheetBlockStyle *gnucash_sheet_get_style (GnucashSheet *sheet,
                                          VirtualCellLocation vcell_loc);

SheetBlockStyle *
gnucash_sheet_get_style_from_table (GnucashSheet *sheet,
                                    VirtualCellLocation vcell_loc);

void gnucash_sheet_style_get_cell_pixel_rel_coords (SheetBlockStyle *style,
                                                    gint cell_row,
						    gint cell_col,
                                                    gint *x, gint *y,
						    gint *w, gint *h);

void gnucash_style_ref   (SheetBlockStyle *style);
void gnucash_style_unref (SheetBlockStyle *style);

void gnucash_style_config_register_borders (gboolean use_vertical_lines,
                                            gboolean use_horizontal_lines);
void gnucash_sheet_get_borders (GnucashSheet *sheet, VirtualLocation virt_loc,
                                PhysicalCellBorders *borders);

void gnucash_sheet_get_header_widths (GnucashSheet *sheet, int *header_widths);
void gnucash_sheet_set_header_widths (GnucashSheet *sheet, int *header_widths);


extern GdkFont *gnucash_register_font;
extern GdkFont *gnucash_register_hint_font;


#endif


/*
  Local Variables:
  c-basic-offset: 8
  End:
*/