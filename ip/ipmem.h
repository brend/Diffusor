#ifndef _IPMEM_H_
#define _IPMEM_H_

/*
Allocates memory for an image of dimension nx * ny.
Returns allocated buffer or NULL, if allocation failed.
*/
float **ip_allocate_image(
			  int nx, /* image dimension in x direction */
			  int ny  /* image dimension in y direction */
			  );

float **ip_duplicate_image(
			   int nx,
			   int ny,
			   float **f
			   );

/*
Deallocates memory of an image previously allocated using allocate_image with
dimension nx * ny.
*/
void ip_deallocate_image(
		      int nx, /* image dimension in x direction */
		      int ny, /* image dimension in y direction */
		      float **u /* in: image of dimension nx*ny */
		      );

#endif
