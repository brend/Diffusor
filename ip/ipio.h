#ifndef _IPIO_H_
#define _IPIO_H_

#include <stdio.h>
#include "ipdefs.h"

/*
Loads the contents of a Portable Greymap into memory.
*/
float **ip_load_image(
		   FILE *file, /* file containing pgm image; if NULL, stdin will be used */
		   int *nx,    /* out: image dimension in x direction */
		   int *ny,    /* out: image dimension in y direction */
		   float ***u  /* out: image of size nx*ny */
		   );

/*
Writes an image to a Portable Greymap (PGM) file, either in ascii or raw format.
*/
void ip_save_image(
		   FILE *file, /* file to save image to; if NULL, stdout will be used */
		   int nx, /* image dimension in x direction */
		   int ny, /* image dimension in y direction */
		   float **u, /* image to be writen */
		   const char *comment, /* additional information to be stored in the file as a comment (may be NULL) */
		   BOOL binary
		   );

/*
Writes a comment to a file.
The comment may consist of multiple (\n-separated) lines. Each line will be prefixed with a "#",
which denotes a comment in a PGM file.
If comment == NULL, then no action will be taken.
*/
void write_comment(
		   FILE *file, /* file to write to */
		   const char *comment /* comment to write */
		   );

/*
Prints grey values of an image to a file.
Image scan lines are separated with \n, columns (values) are separated with spaces.
*/
void print_image(
		 int nx, /* image dimension in x direction */
		 int ny, /* image dimension in y direction */
		 float **u, /* image data */
		 FILE *file /* file to write to */
		 );

#endif
