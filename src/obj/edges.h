// -*- c-basic-offset: 2; indent-tabs-mode: nil -*-

/*
 *  edges - Edge detection for even-odd block fills.
 *  Copyright (C) 2001,2005-6,2012  Steven Simpson
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *  Contact Steven Simpson <https://github.com/simpsonst>
 */

#ifndef edges_h
#define edges_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#ifdef __GNUC__
#define edges_deprecated(D) __attribute__ ((deprecated)) D
#else
#define edges_deprecated(D) D
#endif

  /* A co-ordinate */
  typedef int edges_ord;
  edges_deprecated(typedef edges_ord edges_ord_t);
#define edges_PRIdORD "d"
#define edges_PRIiORD "i"
#define edges_PRIxORD "x"
#define edges_PRIXORD "X"
#define edges_PRIoORD "o"

  /* A pair of co-ordinates */
  struct edges_point {
    edges_ord x, y;
  };

  /* A unit of workspace */
  typedef unsigned char edges_workcell;
  edges_deprecated(typedef edges_workcell edges_work_t);
#define edges_worksize(W,H) (((W) + 1) * ((H) + 1))
#define edges_size(W,H) (edges_worksize((W),(H)))

  /* A type that knows how to draw lines */
  struct edges_plotter {
    void (*move)(void *, const struct edges_point *);
    void (*draw)(void *, const struct edges_point *);
  };

  /* A type that knows what an edge looks like */
  typedef int edges_cmpproc(void *, const struct edges_point *,
                            const struct edges_point *);
  edges_deprecated(typedef edges_cmpproc edges_cmp_proc_t);

  /* Plot the edges of a grid of cells identified by width, height,
   * testtype and test.
   *
   * Set up lptype(lp, pos) to receive drawing instructions.
   *
   * Pass edges_workcell[edges_worksize(width, height)] as ws.
   *
   * Set flags to zero. */
  int edges_plot(size_t width, size_t height,
                 edges_cmpproc *testtype, void *test,
                 const struct edges_plotter *lptype, void *lp,
                 unsigned flags, edges_workcell *ws);

  /* A type for a palette index, with 0 => transparent */
  typedef unsigned char edges_pixel;
#define edges_MAX_COLOURS (1ul + (edges_pixel) ~0u)
#define edges_MAX_COLOURBITS ((edges_MAX_COLOURS + CHAR_BIT - 1u) / CHAR_BIT)
#define edges_pixelsize(W,H) ((W)*(H))

#define edges_bitarrsize(N) (((N) + CHAR_BIT - 1u) / CHAR_BIT)
#define edges_setmask(A,N) ((void) ((A)[(N)/CHAR_BIT] |= 1u<<((N)%CHAR_BIT)))

  /* A type for pixel workspace */
  typedef unsigned char edges_workpixel;
#define edges_workpixelsize(W,H) ((W)*(H))

  /* A function that is told the next colour to be drawn */
  typedef int edges_nextcolproc(void *, edges_pixel,
                                edges_cmpproc *cmpproc, void *cmp);

  /* A type that knows what colour a pixel is */
  typedef edges_pixel edges_imgproc(void *, const struct edges_point *);

  /* Convert an indexed image to path.
   *
   * Provide image pixels through (*imgproc)(img, location).
   *
   * Set up nextcoloproc(nextcol, ...) to receive drawing instructions
   * for each colour.
   *
   * Pass edges_workpixel[edges_workpixelsize(width, height)] as pws.
   *
   * Pass edges_pixel[edges_pixelsize(width, height)] as base.
   *
   * Pass an array of bits through mask[edges_bitarrsize(colours)].
   * They should all be zero, except where a colour number is to be
   * treated as transparent.  This array must be modifiable - it will
   * be corrupted.
   *
   * Set flags to zero. */
  int edges_unraster(edges_imgproc *imgproc, void *img,
                     size_t width, size_t height,
                     size_t colours, unsigned char *mask,
                     edges_nextcolproc *nextcolproc, void *nextcol,
                     unsigned flags, edges_pixel *base,
                     edges_workpixel *pws);

#ifdef __cplusplus
}
#endif

#endif
