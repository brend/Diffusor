#include "ipio.h"

#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <stdlib.h>
#include "ipmem.h"
// #include "/usr/local/netpbm/include/pam.h"

#define P5 5
#define P2 2

#define iswhitespace(c) ((c) == ' ' || (c) == '\t' || (c) == '\n' || (c) == '\r')
#define ord(c) ((c) - '0')

// Reads f discarding whitespace characters. The first non-whitespace character or EOF will returned.
int eatwhites(FILE *f)
{
  int c;

  do
    c = getc(f);
  while (c != EOF && iswhitespace(c));
  
  return c;
}

// Reads f discarding non-whitespace characters. The first whitespace character or EOF will returned.
int eatnonwhites(FILE *f) 
{
  int c;
  
  do
    c = getc(f);
  while (!(c == EOF || iswhitespace(c)));
  
  return c;
}

void eatline(FILE *f)
{
  int c;

  do
    c = getc(f);
  while (!(c == '\n' || c == '\r' || c == EOF));
}

int eatnoise(FILE *f)
{
  int c;

  do {
    if (EOF == (c = eatwhites(f)))
      return EOF;
    
    if (c == '#')
      eatline(f);
    else
      return c;
  } while (c != EOF);

  return EOF;
}

int readnat(int firstdigit, FILE *f)
{
  assert(isdigit(firstdigit));

  int n = ord(firstdigit);
  int c;

  while (!feof(f))
    if (isdigit(c = getc(f)))
      n = n * 10 + ord(c);
    else
      return n;

  return n;
}

BOOL ip_load_p5(
	       FILE *f,
	       int nx,
	       int ny,
	       float **u
	       )
{
  assert(f);
  assert(nx >= 0 && ny >= 0);
  assert(u);

  int i, j;

  for (j = 1; j <= ny; ++j)
    for (i = 1; i <= nx; ++i)
      if (feof(f))
	return FALSE;
      else
	u[i][j] = getc(f);

  return TRUE;
}

BOOL ip_load_p2(
		FILE *f,
		int nx,
		int ny,
		float **u
		)
{
  assert(f);
  assert(nx >= 0 && ny >= 0);
  assert(u);

  int i, j;

  for (j = 1; j <= ny; ++j)
    for (i = 1; i <= nx; ++i) {
      int c = eatwhites(f);
      int n = readnat(c, f);

      if (n < 0 || n >= 255)
	return FALSE;

      u[i][j] = n;
    }

  return TRUE;
}

float **ip_load_image(
		   FILE *f,
		   int *nx,
		   int *ny,
		   float ***u
		   )
{
  assert(nx && ny);

  if (f == NULL)
    f = stdin;

  // read magic number and determine file type (P2 or P5)
  char magic[] = { 0, 0, 0 };
  int type = 0;

  magic[0] = getc(f);
  magic[1] = getc(f);

  if (strcmp(magic, "P5") == 0)
    type = P5;
  else if (strcmp(magic, "P2") == 0)
    type = P2;
  else
    return NULL;

  int header[] = { -1, -1, -1 };
  int w = 0, h = 0, max = 0;
  int c = EOF;
  int i;

  for (i = 0; i < 3; ++i) {
    if (EOF == (c = eatnoise(f)))
      return NULL;

    int n = readnat(c, f);

    if (n < 0)
      return NULL;

    header[i] = n;
  }

  w = header[0];
  h = header[1];
  max = header[2];

  float **v = ip_allocate_image(w + 2, h + 2);
  int result = FALSE;

  if (v == NULL)
    return NULL;

  switch (type) {
  case P5:
    result = ip_load_p5(f, w, h, v);
    break;
  case P2:
    result = ip_load_p2(f, w, h, v);
    break;
  default:
    abort();
  }

  if (!result) {
    ip_deallocate_image(w, h, v);

    return NULL;
  }

  *nx = w;
  *ny = h;

  if (u != NULL)
    *u = v;

  return v;
}

/*
Writes an image to a Portable Greymap (PGM) file, either in ascii or raw format.
*/
void save_image(
		FILE *file, /* file to save image to; if NULL, stdout will be used */
		int nx, /* image dimension in x direction */
		int ny, /* image dimension in y direction */
		float **u, /* image to be writen */
		const char *comment, /* additional information to be stored in the file as a comment (may be NULL) */
		BOOL binary
		)
{
  assert(u);
  assert(nx >= 0 && ny >= 0);

  if (file == NULL)
    file = stdout;

  const char *imagetype = binary ? "P5" : "P2";
  int maxval = 255;

  // fprintf(file, "%s\n%d %d\n%d\n", imagetype, nx, ny, maxval);
  fprintf(file, "%s\n", imagetype);
  if (comment)
    write_comment(file, comment);
  fprintf(file, "%d %d\n%d\n", nx, ny, maxval);

  int x, y;

  for (y = 0; y < ny; ++y) {
    for (x = 0; x < nx; ++x) {
      int value = (int) round(maxval * u[x][y]);

      value = value > 255 ? 255 : (value < 0 ? 0 : value);

      if (binary)
	fputc(value, file);
      else
	fprintf(file, "%3d ", value);
    }
    if (!binary)
      fprintf(file, "\n");
  }
}

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
		   )
{
  assert(u);
  assert(nx >= 0 && ny >= 0);

  if (file == NULL)
    file = stdout;

  const char *imagetype = binary ? "P5" : "P2";
  int maxval = 255;

  fprintf(file, "%s\n", imagetype);
  if (comment)
    write_comment(file, comment);
  fprintf(file, "%d %d\n%d\n", nx, ny, maxval);

  int x, y;

  for (y = 1; y <= ny; ++y) {
    for (x = 1; x <= nx; ++x) {
      int value = (int) u[x][y];

      value = value > 255 ? 255 : (value < 0 ? 0 : value);

      if (binary)
	fputc(value, file);
      else
	fprintf(file, "%3d ", value);
    }
    if (!binary)
      fprintf(file, "\n");
  }
}

/*
Writes a comment to a file.
The comment may consist of multiple (\n-separated) lines. Each line will be prefixed with a "#",
which denotes a comment in a PGM file.
If comment == NULL, then no action will be taken.
*/
void write_comment(
		   FILE *file, /* file to write to */
		   const char *comment /* comment to write */
		   )
{
  assert(file);

  char 
    *c = (char *) malloc(sizeof(char) * strlen(comment) + 1),
    *d = NULL;

  strcpy(c, comment);

  while (NULL != (d = strsep(&c, "\r\n")))
    fprintf(file, "# %s\n", d);

  free(c);
}

/*
Prints grey values of an image to a file.
Image scan lines are separated with \n, columns (values) are separated with spaces.
*/
void print_image(
		 int nx, /* image dimension in x direction */
		 int ny, /* image dimension in y direction */
		 float **u, /* image data */
		 FILE *file /* file to write to */
		 )
{
  if (file == NULL)
    file = stdout;

  int x, y;

  for (y = 0; y < ny; ++y) {
    for (x = 0; x < nx; ++x)
      fprintf(file, "%3.3f ", u[x][y]);
    fprintf(file, "\n");
  }
}

