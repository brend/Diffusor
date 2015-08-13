/*
Loads the contents of a Portable Greymap into memory.
*/
float **load_image(
		   FILE *file, /* file containing pgm image; if NULL, stdin will be used */
		   int *nx,    /* out: image dimension in x direction */
		   int *ny,    /* out: image dimension in y direction */
		   float ***u  /* out: image of size nx*ny */
		   )
{
  assert(nx && ny);

  if (file == NULL)
    file = stdin;
 
  struct pam inpam;
  tuple **ta = pnm_readpam(file, &inpam, PAM_STRUCT_SIZE(tuple_type));

  float f = 1.0f / inpam.maxval;
  float **v = ip_allocate_image(inpam.width, inpam.height);

  if (v != NULL) {
    int x, y;
    
    for (y = 0; y < inpam.height; ++y)
      for (x = 0; x < inpam.width; ++x) {
	v[x][y] = (float) ta[y][x][0] * f;
      }
  }

  pnm_freepamarray(ta, &inpam);

  if (u)
    *u = v;
  *nx = inpam.width;
  *ny = inpam.height;

  return v;
}

float **ip_load_image(
		   FILE *file, /* file containing pgm image; if NULL, stdin will be used */
		   int *nx,    /* out: image dimension in x direction */
		   int *ny,    /* out: image dimension in y direction */
		   float ***u  /* out: image of size nx*ny */
		   )
{
  assert(nx && ny);

  if (file == NULL)
    file = stdin;
 
  struct pam inpam;
  tuple **ta = pnm_readpam(file, &inpam, PAM_STRUCT_SIZE(tuple_type));

  float **v = ip_allocate_image(inpam.width + 2, inpam.height + 2);

  if (v != NULL) {
    int x, y;
    
    for (y = 0; y < inpam.height; ++y)
      for (x = 0; x < inpam.width; ++x) {
	v[x + 1][y + 1] = (float) ta[y][x][0];
      }
  }

  pnm_freepamarray(ta, &inpam);

  if (u)
    *u = v;
  *nx = inpam.width;
  *ny = inpam.height;

  return v;
}
