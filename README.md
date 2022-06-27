# Purpose

This library allows the edges of a two-colour rectangular grid of cells to be traced.
All the user needs to do is provide a function that can determine if an edge exists between two adjacent cells, and two functions to deal with drawing straight lines.
The latter will be called to produce a path that, when filled with an even-odd fill rule, gives one of the colours.
This is usually a more efficient representation than linearly drawing each block, mapping neatly onto PostScript and SVG drawing capabilities.

The library can further be used on a multicolour image &ndash; the fewer colours the better.
It will select each colour used, and allow the user to plot it in such a way that colours plotted later will hide cells that aren't supposed to be there, but improve the drawing efficiency.
For example, the following image has two colours, `#` and `X`:

```
##
#X
```

Each colour could be drawn in its exact shape:

```
##
# 
```

and

```

 X
```

&hellip;but the `#` shape takes 6 drawing steps.  By drawing `#` first,
then `X` on top, the `#` area can be reduced to 4 steps:

```
##
##
```

and

```
  
 X
```

The library tries to select an appropriate order for plotting each colour, in order to exploit such overlaps and reduce the size of the path.
At the moment, this is done by finding the colour appears most often adjacent to the edge of the image, a transparent cell, or a cell which has already been plotted.

The library looks for permissible optimisations by first assuming that all other colours that haven't already been plotted could be drawn as the colour currently in question.
(This is correct, since those colours will have to overlap such areas anyway.)
Then it prunes away (some of) the rough edges.
Finally, it allows the current colour to flood into the areas of the pending colours, so that islands of pending colours don't get plotted.

# Use

## Plotting a single edge

This involves a single call to `edges_plot`.

```
#include <edges.h>

int edges_plot(size_t width, size_t height,
               edges_cmpproc *testtype, void *test,
               const struct edges_plotter *lptype, void *lp,
               unsigned flags, edges_workcell *ws);
```

In order to be independent of any particular image format, the API requires that the image be described through a series of calls to a function of type `edges_cmpproc`.
For example:

```
static int my_edges(void *test,
                    const struct edges_point *p1,
                    const struct edges_point *p2);
```

`test` is the argument of the same name supplied to the `edges_plot` call, and can be used to pass any image context required.
The remaining arguments provide the co-ordinates of two adjacent cells in the image.
The two dimensions of the image are taken as starting from 0 (inclusive) upto `width` (for the 'x' dimension) or `height` (for 'y') (exclusive).

The function should return `0` if and only if no edge exists between the two cells.
A non-zero value indicates an edge &ndash; the sign indicates the relative direction of the edge, so the same call with the two cells swapped should give a value of the opposite sign.
(In fact, this sign is ignored, so non-zero is sufficient to indicate an edge, but I'd like to preserve the sign requirement for now, in case it still turns out useful.)

The user also needs to specify how to draw any given path, by providing a `struct edges_plotter` which contains pointers to two of the user's functions, both of the form:

```
void func(void *lp, const struct edges_point *p);
```

`lp` is the argument of the same name supplied to the `edges_plot` call, and conveys any user-defined context to the functions.
The point `*p` refers to a corner of a cell, rather than the cell itself.
For example, if 'x' increases to the right, and 'y' down, then the top-left corner of the cell `*p` is specified.

As an alternative example, if `p = { .x = 3, .y = 5 }`, the point marked by the 'X' is specified:

```
X-----+-----+
|     |     |
| x=3 | x=4 |
|     |     |
| y=5 | y=5 |
|     |     |
+-----+-----+
|     |     |
| x=3 | x=4 |
|     |     |
| y=6 | y=6 |
|     |     |
+-----+-----+
```

The two fields of the structure are `move` and `draw`.
The first call will be to `move`, and will be followed by 4 or more `draw`s, then possibly other `move`s and `draw`s.
The last `draw` of each sequence will be to the same co-ordinates as the most recent `move`, indicating the end of the current path segment.

Finally, some workspace is needed.
The library leaves allocation up to the user, and it should be an array of `edges_workcell`, at least `edges_worksize(width,height)` in length.


## Plotting an image

This involves a single call to `edges_unraster`.

```
#include <edges.h>

int edges_unraster(edges_imgproc *imgproc, void *img,
                   size_t width, size_t height,
                   size_t colors, unsigned char *mask,
                   edges_nextcolproc *nextcolproc, void *nextcol,
                   unsigned flags, edges_pixel *base,
                   edges_workpixel *pws);
```

Again, the image format is independent, and the library obtains it by calling a user-supplied function several times.
It has the following form:

```
edges_pixel edges_imgproc(void *img, const struct edges_point *p);
```

`img` is the user-defined context pointer, as passed to `edges_unraster`.
`*p` refers to a pixel position.

The function should return a code for the colour at that pixel, less than the `colors` argument supplied to `edges_unraster`.
A unique code is needed for each RGBA colour value used, except for fully transparent pixels, which can share the same code.

The user must also set up a bit mask in an array of unsigned char.
The array must be writable; it will be corrupted by the call.

Each pixel corresponds to a colour code, and must be set if the colour is completely transparent.
If the number of colours is `NCOLS`, mark code `TCOL` as fully transparent, use:

```
#include <limits.h>

unsigned char mask[edges_bitarrsize(NCOLS)] = { 0 };
edges_setmask(mask, TCOL);
```

If there is no transparent colour, just don't use `edges_setmask`.

The user must also provide a function to handle each colour.
It has the form:

```
int plot_colour(void *nextcol, edges_pixel col,
                edges_cmpproc *cmpproc, void *cmp);
```

`nextcol` is the user-context pointer argument of the same name, given to `edges_unraster`.
`col` gives the code of the pixel being drawn.
`cmpproc` and `cmp` describe the edges of the colour, and can be passed directly to `edges_plot` as the second and third arguments.
However, they are only valid for the duration of the call to this user
function.

Finally, two areas of workspace are needed.
They can be allocated by these, for example:

```
edges_pixel base[edges_pixelsize(width, height)];
edges_workpixel pws[edges_workpixelsize(width, height)];
```
