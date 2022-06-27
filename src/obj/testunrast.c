// -*- c-basic-offset: 2; indent-tabs-mode: nil -*-

#include <stdlib.h>
#include <stdio.h>
#include <limits.h>

#include "edges.h"

static const char pic[][20] = {
  "00000004444400000000",
  "00004443303344400000",
  "00043303101303340000",
  "00043100222001340000",
  "04400220442022004400",
  "04330204142202033400",
  "04312022442220213400",
  "44020222442222020044",
  "43302223443122200334",
  "40020202442231202034",
  "40202322442223220204",
  "43122124444422122334",
  "43302040440442322134",
  "04022143003442120040",
  "04302214334441221340",
  "04334404444400443340",
  "04440000100100004440",
  "00000000100100000000",
  "00000000100100000000",
  "00000001101100000000"
};

#define COLOURS 5

#define WIDTH (sizeof pic[0] / sizeof pic[0][0])
#define HEIGHT (sizeof pic / sizeof pic[0])

static edges_pixel getpix(void *v, const struct edges_point *p)
{
  v = v;
  return pic[p->y][p->x] - '0';
}

static void move(void *v, const struct edges_point *p)
{
  v = v;
  printf("\n*(%d %d)", (int) p->x, (int) p->y);
}

static void draw(void *v, const struct edges_point *p)
{
  v = v;
  printf(" (%d %d)", (int) p->x, (int) p->y);
}

static struct edges_plotter plotter = { &move, &draw };

static int nextcol(void *v, edges_pixel col,
                   edges_cmpproc *cmpproc, void *cmp)
{
  int rc;

  edges_workcell ews[edges_size(WIDTH, HEIGHT)];
  v = v;
  printf("\nColour %u:", (unsigned) col);
  rc = edges_plot(WIDTH, HEIGHT, cmpproc, cmp, &plotter, NULL, 0, ews);
  printf("\n");
  return rc;
}

int main()
{
  unsigned char mask[edges_bitarrsize(COLOURS)] = { 0 };
  int rc;

  edges_pixel base[edges_pixelsize(WIDTH, HEIGHT)];
  edges_workpixel pws[edges_workpixelsize(WIDTH, HEIGHT)];

  edges_setmask(mask, 0);

  printf("Size: %u x %u\n", (unsigned) WIDTH, (unsigned) HEIGHT);

  rc = edges_unraster(&getpix, NULL, WIDTH, HEIGHT, 5, mask,
                      &nextcol, NULL, 0, base, pws);
  printf("\nReturn code: %d\n", rc);
  return EXIT_SUCCESS;
}
