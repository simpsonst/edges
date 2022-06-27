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

#include <stdlib.h>
#include <limits.h>
#include <assert.h>

#include <stdio.h>

#include "edges.h"

enum {
  LEFT = 1,
  TOP = 2,
  LEFTR = 4,
  TOPR = 8
};

static const struct edges_point vector[] = {
  { +1, 0 }, { 0, +1 }, { -1, 0 }, { 0, -1 }
};

static const struct edges_point vector2[] = {
  { +1, 0 }, { +1, +1 },
  { 0, +1 }, { -1, +1 },
  { -1, 0 }, { -1, -1 },
  { 0, -1 }, { +1, -1 }
};

int edges_plot(size_t wid, size_t hei,
               edges_cmpproc *testtype, void *test,
               const struct edges_plotter *lptype, void *lp,
               unsigned flags, edges_workcell *ws)
{
  edges_ord inc = wid + 1;

  struct edges_point p;
  unsigned total = 0;

  flags = flags;

  for (p.y = 0; (unsigned) p.y <= hei; p.y++) {
    for (p.x = 0; (unsigned) p.x <= wid; p.x++) {
      struct edges_point p0, p1;
      int tleft, ttop;

      p0.x = p.x - 1, p0.y = p.y;
      p1.x = p.x, p1.y = p.y - 1;

      tleft = (*testtype)(test, &p, &p0);
      ttop = (*testtype)(test, &p, &p1);

      ws[p.x + p.y * inc] =
        (tleft ? LEFT : 0)
        | (ttop ? TOP : 0);

      /*
        | (tleft < 0 ? LEFTR : 0)
        | (ttop < 0 ? TOPR : 0)
      */

      total += !!tleft + !!ttop;
    }
  }

  for (p.x = 0; total && (unsigned) p.x <= wid; p.x++)
    for (p.y = 0; total && (unsigned) p.y <= hei; p.y++) {
      if (ws[p.x + p.y * inc]) {
        int dir = 0, ndir;
        struct edges_point cur;

        cur.x = p.x + vector[dir].x;
        cur.y = p.y + vector[dir].y;

        (*lptype->move)(lp, &p);

        assert(ws[p.x + p.y * inc] & TOP);
        ws[p.x + p.y * inc] &= ~TOP;

        while (dir >= 0) {
          edges_workcell *cp = &ws[cur.x + cur.y * inc];
          assert(dir < 4);
          switch (dir) {
          case 0:
            if (cur.x < (edges_ord) wid && cp[0] & TOP) {
              ndir = dir;
              cp[0] &= ~TOP;
            } else if (cur.y < (edges_ord) hei && cp[0] & LEFT) {
              ndir = (dir + 1) % 4;
              cp[0] &= ~LEFT;
            } else if (cur.y > 0 && cp[-inc] & LEFT) {
              ndir = (dir + 3) % 4;
              cp[-inc] &= ~LEFT;
            } else {
              ndir = -1;
            }
            break;
          case 1:
            if (cur.y < (edges_ord) hei && cp[0] & LEFT) {
              ndir = dir;
              cp[0] &= ~LEFT;
            } else if (cur.x > 0 && cp[-1] & TOP) {
              ndir = (dir + 1) % 4;
              cp[-1] &= ~TOP;
            } else if (cur.x < (edges_ord) wid && cp[0] & TOP) {
              ndir = (dir + 3) % 4;
              cp[0] &= ~TOP;
            } else {
              ndir = -1;
            }
            break;
          case 2:
            if (cur.x > 0 && cp[-1] & TOP) {
              ndir = dir;
              cp[-1] &= ~TOP;
            } else if (cur.y > 0 && cp[-inc] & LEFT) {
              ndir = (dir + 1) % 4;
              cp[-inc] &= ~LEFT;
            } else if (cur.y < (edges_ord) hei && cp[0] & LEFT) {
              ndir = (dir + 3) % 4;
              cp[0] &= ~LEFT;
            } else {
              ndir = -1;
            }
            break;
          case 3:
            if (cur.y > 0 && cp[-inc] & LEFT) {
              ndir = dir;
              cp[-inc] &= ~LEFT;
            } else if (cur.x < (edges_ord) wid && cp[0] & TOP) {
              ndir = (dir + 1) % 4;
              cp[0] &= ~TOP;
            } else if (cur.x > 0 && cp[-1] & TOP) {
              ndir = (dir + 3) % 4;
              cp[-1] &= ~TOP;
            } else {
              ndir = -1;
            }
            break;
          default:
            fprintf(stderr, "unreachable (%s:%d)\n", __FILE__, __LINE__);
            fflush(stderr);
            abort();
          }

          if (ndir >= 0)
            total--;

          if (ndir != dir) {
            (*lptype->draw)(lp, &cur);
            dir = ndir;
          }

          if (dir >= 0) {
            cur.x += vector[dir].x;
            cur.y += vector[dir].y;
          }
        }
      }
    }

  return 0;
}

/* Find out how popular each colour is. */
static void count_pop(const edges_pixel *base,
                      size_t width, size_t height,
                      size_t colours,
                      unsigned *count)
{
  struct edges_point p;
  edges_pixel col;

  /* Reset the counters. */
  for (col = 0; col < colours; col++)
    count[col] = 0;

  for (p.y = 0; (unsigned) p.y < height; p.y++) {
    for (p.x = 0; (unsigned) p.x < width; p.x++) {
      col = base[p.x + p.y * width];
      count[col]++;
    }
  }
}

/* Find out which colour occurs the most, adjacent to transparent
   areas.  Bits in mask are set if the colour is to be treated as
   transparent. */
static void count_xparadj(const edges_pixel *base,
                          size_t width, size_t height,
                          size_t colours, const unsigned char *mask,
                          unsigned *count)
{
  struct edges_point p;
  edges_pixel col;

  /* Reset the counters. */
  for (col = 0; col < colours; col++)
    count[col] = 0;

  /* Look for transparent cells. */
  for (p.y = 0; (unsigned) p.y < height; p.y++) {
    for (p.x = 0; (unsigned) p.x < width; p.x++) {
      int dir;

      col = base[p.x + p.y * width];

      /* Ignore transparent pixels. */
      if (mask[col / CHAR_BIT] & (1u << (col % CHAR_BIT)))
        continue;

      /* Look at the adjacent cells. */
      for (dir = 0; dir < 4; dir++) {
        struct edges_point t = p;
        t.x += vector[dir].x;
        t.y += vector[dir].y;

        if (t.x >= 0 && t.y >= 0 &&
            (unsigned) t.x < width && (unsigned) t.y < height) {
          edges_pixel adjcol;

          /* What colour is this adjacent cell? */
          adjcol = base[t.x + t.y * width];

          /* Ignore if also solid. */
          if (!(mask[adjcol / CHAR_BIT] & (1u << (adjcol % CHAR_BIT))))
            continue;
        }

        count[col]++;
      }
    }
  }
}

enum { TRANSPARENT, OTHER, HIDDEN, SOLID };

/* Copy pixels of colour 'col' from the image into pws as SOLIDs, other
   solids as HIDDEN, and transparent cells as TRANSPARENT. */
static void copy_colour(const edges_pixel *base, size_t width, size_t height,
                        unsigned char *mask,
                        edges_workpixel *pws, size_t best_col)
{
  struct edges_point p;
  edges_pixel col;

  for (p.y = 0; (unsigned) p.y < height; p.y++) {
    for (p.x = 0; (unsigned) p.x < width; p.x++) {
      col = base[p.x + p.y * width];
      if (col == best_col)
        col = SOLID;
      else
        col = !(mask[col / CHAR_BIT] & (1u << col % CHAR_BIT)) ?
          HIDDEN : TRANSPARENT;
      pws[p.x + p.y * width] = col;
    }
  }
}

/* Find out which of the cells surrounding '*p' are SOLID/HIDDEN.  The
   result is a bit array - bit 0 => top, bit 1 => top left, etc,
   anti-clockwise.  The top cell's state is repeated at the end, to
   give nine bits, starting and ending the same. */
static unsigned short
get_profile(edges_workpixel *pws, size_t width, size_t height,
            const struct edges_point *p)
{
  unsigned short r = 0;
  int dir;

  for (dir = 0; dir < 8; dir++) {
    struct edges_point cur;
    edges_pixel col;

    cur.x = p->x + vector2[dir].x;
    cur.y = p->y + vector2[dir].y;

    if (cur.x < 0 || cur.y < 0 ||
        (size_t) cur.x >= width || (size_t) cur.y >= height)
      /* External cells are always transparent. */
      col = TRANSPARENT;
    else
      col = pws[cur.x + cur.y * width];

    r <<= 1;
    if (col == HIDDEN || col == SOLID)
      r |= 1u;
  }
  return (r << 1) | (r >> 7);
}

/* Take a cell's profile (as from get_profile), and work out the
   effect on the complexity of the path if the cell is removed. */
static int get_change(unsigned short n)
{
  int r = 0;
  int d;

  /* A table of differences for various bit patterns - an odd number
     of bits => +1; even => -1. */
  static const int ch[] = { -1, +1, +1, -1, +1, -1, -1, +1 };

  /* Examine each corner. */
  for (d = 0; d < 4; d++) {
    /* Look at the 3 cells that make up each corner of the target
       cell.  They are in the bottom 3 bits of the profile 'n'.  An
       odd number of neighbours results in an increase in complexity;
       even => decrease. */
    r += ch[n & 7u];

    /* Discard two of the neighbours, but retain the third, as it's
       part of the next corner. */
    n >>= 2;
  }

  return r;
}

/* Keep call (*changeproc)(change, loc) for each location in the grid,
   until the function returns 0 for an entire sweep of the grid. */
static void repeat_until(size_t width, size_t height,
                         int (*changeproc)(void *, const struct edges_point *),
                         void *change)
{
  int found;
  unsigned dir = 0;

  do {
    struct edges_point p;
    edges_ord *maj, *min;
    size_t majmax, minmax;
    found = 0;

    if (dir & 1u) {
      maj = &p.x;
      majmax = width;
      min = &p.y;
      minmax = height;
    } else {
      min = &p.x;
      minmax = width;
      maj = &p.y;
      majmax = height;
    }

    for (*maj = 0; (unsigned) *maj < majmax; ++*maj)
      for (*min = 0; (unsigned) *min < minmax; ++*min) {
        struct edges_point q = p;
        if (dir & 2u)
          q.x = width - 1 - q.x;
        if (dir & 4u)
          q.y = height - 1 - q.y;
        found = (*changeproc)(change, &q) || found;
      }

    dir++;
  } while (found);
}

struct grid_context {
  edges_workpixel *pws;
  size_t width, height;
};

static int prune_cell(void *vc, const struct edges_point *p)
{
  struct grid_context *ctxt = vc;
  int change;

  /* We can only prune HIDDEN cells (by converting to OTHER). */
  if (ctxt->pws[p->x + p->y * ctxt->width] != HIDDEN)
    return 0;

  /* If we remove this cell, how does that alter the path's
     complexity? */
  change = get_change(get_profile(ctxt->pws, ctxt->width, ctxt->height, p));

  /* Ignore cells that increase complexity. */
  if (change > 0)
    return 0;

  /* Delete this cell. */
  ctxt->pws[p->x + p->y * ctxt->width] = OTHER;
  return 1;
}

/* Prune HIDDEN regions so that the HIDDEN/SOLID conglomerate has a
   smoother edge. */
static void prune_region(edges_workpixel *pws, size_t width, size_t height)
{
  struct grid_context ctxt;

  ctxt.pws = pws;
  ctxt.width = width;
  ctxt.height = height;

  repeat_until(width, height, &prune_cell, &ctxt);
}

static int flood_cell(void *vc, const struct edges_point *p)
{
  struct grid_context *ctxt = vc;

  int found = 0;
  int dir;

  /* We can flood only from SOLID cells. */
  if (ctxt->pws[p->x + p->y * ctxt->width] != SOLID)
    return 0;

  /* Look at the adjacent cells. */
  for (dir = 0; dir < 4; dir++) {
    struct edges_point cur;
    cur.x = p->x + vector[dir].x;
    cur.y = p->y + vector[dir].y;

    /* Ignore external regions. */
    if (cur.x < 0 || cur.y < 0 ||
        (size_t) cur.x >= ctxt->width || (size_t) cur.y >= ctxt->height)
      continue;

    /* Ignore non-HIDDEN adjacent cells. */
    if (ctxt->pws[cur.x + cur.y * ctxt->width] != HIDDEN)
      continue;

    /* Flood into this cell. */
    ctxt->pws[cur.x + cur.y * ctxt->width] = SOLID;
    found = 1;
  }

  return found;
}

/* Allow SOLID areas to overflow into neighbouring HIDDENs, so only
   islands of HIDDEN remain. */
static void flood_hidden(edges_workpixel *pws, size_t width, size_t height)
{
  struct grid_context ctxt;

  ctxt.pws = pws;
  ctxt.width = width;
  ctxt.height = height;

  repeat_until(width, height, &flood_cell, &ctxt);
}

struct selection {
  edges_workpixel *pws;
  size_t width, height;
};

/* Return 0 if the cell is outside the image, or is TRANSPARENT or
   OTHER or HIDDEN.  Return 1 if SOLID. */
static int getcol(const struct selection *s,
                  const struct edges_point *p)
{
  if (p->x < 0 || p->y < 0 ||
      (unsigned) p->x >= s->width || (unsigned) p->y >= s->height)
    return 0;
  switch (s->pws[p->x + p->y * s->width]) {
  case SOLID:
    return 1;

  default:
    return 0;
  }
}

/* Determine whether an edge exists between the two adjacent cells p1
   and p2.  Return 0 if no edge exists, and non-zero otherwise. */
static int getedge(void *vp, const struct edges_point *p1,
                   const struct edges_point *p2)
{
  return getcol(vp, p1) - getcol(vp, p2);
}

#ifdef edges_DEBUG
static void print_data(edges_workpixel *pws, size_t width, size_t height)
{
  struct edges_point p;
  for (p.y = 0; (unsigned) p.y < height; p.y++) {
    for (p.x = 0; (unsigned) p.x < width; p.x++) {
      switch (pws[p.x + p.y * width]) {
      case SOLID:
        putchar('#');
        break;

      case OTHER:
        putchar('X');
        break;

      case HIDDEN:
        putchar('-');
        break;

      case TRANSPARENT:
        putchar(' ');
        break;

      default:
        putchar('?');
        break;
      }
    }
    putchar('\n');
  }
}
#endif

int edges_unraster(edges_imgproc *imgproc, void *img,
                   size_t width, size_t height,
                   size_t colours, unsigned char *mask,
                   edges_nextcolproc *nextcolproc, void *nextcol,
                   unsigned flags, edges_pixel *base,
                   edges_workpixel *pws)
{
  struct selection sel;

  unsigned xparadj_count[edges_MAX_COLOURS], pop_count[edges_MAX_COLOURS];

  flags = flags;

  /* Copy the image into our format. */
  {
    struct edges_point p;
    for (p.y = 0; (unsigned) p.y < height; p.y++)
      for (p.x = 0; (unsigned) p.x < width; p.x++)
        base[p.x + p.y * width] = (*imgproc)(img, &p);
  }

  /* How often does each colour occur? */
  count_pop(base, width, height, colours, pop_count);

  /* This is our context for edge detection. */
  sel.pws = pws;
  sel.width = width;
  sel.height = height;

  for ( ; ; ) {
    edges_pixel col, best_col;
    int rc;

    /* Find the colour that occurs the most adjacent to transparent
       areas (which includes areas already accounted for. */
    count_xparadj(base, width, height, colours, mask, xparadj_count);

    /* Select the best colour from these - the one with the highest
       transparent-adjacency.  If there is a tie, use the more popular. */
    best_col = 0;
    for (col = 1; col < colours; col++) {
      /* We can ignore colours that have already been dealt with
         (i.e. currently treated as a mask - but in any case, these
         will have zero adjacency. */

      if (xparadj_count[col] < xparadj_count[best_col])
        continue;

      if (xparadj_count[col] == xparadj_count[best_col] &&
          pop_count[col] < pop_count[best_col])
        continue;

      best_col = col;
    }

    /* Does the best colour have no cells?  We've completed the image
       then! */
    if (xparadj_count[best_col] == 0)
      break;

    /* We've chosen the best colour to do next.  Let's make a copy of
       it, see if we can smooth out its edges, and submit this back to
       the caller. */

    /* Make the copy, such that cells of unaccounted colours become
       HIDDEN - i.e. we could paint them with the current colour,
       because we'll be painting over them later anyway.  Cells in the
       current colour will be marked as SOLID - we can't do any
       optimizations on these.  Everything else will be
       TRANSPARENT. */
    copy_colour(base, width, height, mask, pws, best_col);

#ifdef edges_DEBUG
    printf("\nInitial copy:\n");
    print_data(pws, width, height);
#endif

    /* Now try and straighten out the edges of the combined
       HIDDEN/SOLID areas by pruning some of the HIDDEN. */
    prune_region(pws, width, height);

#ifdef edges_DEBUG
    printf("\nPruned:\n");
    print_data(pws, width, height);
#endif

    /* Allow the SOLID areas to flood adjacent HIDDEN areas, so that
       only HIDDEN islands remain. */
    flood_hidden(pws, width, height);

#ifdef edges_DEBUG
    printf("\nFlooded:\n");
    print_data(pws, width, height);
#endif

    /* Tell the caller to plot this colour, using cells marked SOLID. */
    rc = (*nextcolproc)(nextcol, best_col, &getedge, &sel);
    if (rc != 0)
      return rc;

    /* Treat the colour as transparent from now on. */
    mask[best_col / CHAR_BIT] |= 1u << (best_col % CHAR_BIT);
  }

  return 0;
}
