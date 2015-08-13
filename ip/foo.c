#include "ipio.h"
#include "ipmem.h"

int main() {
  FILE *f = fopen("pics/test.pgm", "rb");
  int nx = -1, ny = -1;
  float **u = ip_load_image(f, &nx, &ny, NULL);

  printf("ip_load_image behauptet w = %d, h = %d, image = %d\n", nx, ny, (int) u);

  fclose(f);

  if (!u) {
    ip_deallocate_image(nx, ny, u);

    return 0;
  }

  f = fopen("out.pgm", "wb");
  ip_save_image(f, nx, ny, u, "Heute ist ein schoener Tag.\nGanz ehrlich.", FALSE);
  fclose(f);

  return 0;
}
