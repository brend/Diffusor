#include "ipmem.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/*
Allocates memory for an image of dimension nx * ny.
Returns allocated buffer or NULL, if allocation failed.
*/
float **ip_allocate_image(
			  int nx, /* image dimension in x direction */
			  int ny  /* image dimension in y direction */
			  )
{
  assert(nx >= 0 && ny >= 0);

  float **u = (float **) malloc(sizeof(float *) * nx);

  if (u == NULL)
    return NULL;

  int x;
  float *col;

  for (x = 0; x < nx; ++x) {
    col = (float *) malloc(sizeof(float) * ny);

    if (col == NULL) {
      int rx;

      for (rx = 0; rx < x; ++rx)
	free(u[rx]);
      free(u);
    } else {
      memset(col, 0, ny * sizeof(float));
      u[x] = col;
    }
  }

  return u;
}

float **ip_duplicate_image(
			   int nx,
			   int ny,
			   float **f
			   )
{
  float **u = ip_allocate_image(nx + 2, ny + 2);
  int i, j;

  for (j = 1; j <= ny; ++j)
    for (i = 1; i <= nx; ++i)
      u[i][j] = f[i][j];

  return u;
}

/*
Deallocates memory of an image previously allocated using allocate_image with
dimension nx * ny.
*/
void ip_deallocate_image(
		      int nx, /* image dimension in x direction */
		      int ny, /* image dimension in y direction */
		      float **u /* in: image of dimension nx*ny */
		      )
{
  if (u == NULL)
    return;

  int x;

  for (x = 0; x < nx; ++x)
    if (u[x] != NULL)
      free(u[x]);

  free(u);
}

