#include "ipio.h"

int main() {
  float **x;
  FILE *f = fopen("pics/test.pgm", "rb");
  char magic[256] = "";
  int nx, ny, max;
 
  fscanf(f, "%s%d%d%d", magic, &nx, &ny, &max);

  printf("magic = %s\nnx = %d\nny = %d\nmax = %d\n", magic, nx, ny, max);

  int i, j;

  getc(f);

  for (j = 0; j < ny; ++j)
    for (i = 0; i < nx; ++i)
      getc(f);

  int n = ftell(f);

  fseek(f, 0, SEEK_END);

  printf("Remaining bytes: %d\n", ftell(f) - n);
  printf("Okey-dokey.\n");

  fclose(f);

  return 0;
}
