#include <stdlib.h>
#include <stdio.h>
#include "ip.h"

void print_image_information(const char *filename);

int main(int argc, char **argv) {
  char *infilename = NULL;

  switch (argc) {
  case 2:
    infilename = argv[1];
    break;
  default:
    fprintf(stderr, "usage: %s <input image file(pgm)>\n", argv[0]);
    return 1;
  }

  print_image_information(infilename);

  return 0;
}
    
void print_image_information(const char *filename) {
  FILE *in = fopen(filename, "rb");

  if (!in) {
    fprintf(stderr, "Couldn't open file for reading: %s\n", filename);
    return;
  }

  int nx = 0, ny = 0;
  float **u = ip_load_image(in, &nx, &ny, NULL);

  fclose(in);

  if (!u) {
    fprintf(stderr, "Couldn't read image data from file: %s\n", filename);
    return;
  }

  float sum = sum_image(nx, ny, u), mean = mean_value(nx, ny, u);

  printf("Sum of all grey values: %3.4f\nMean: %3.4f\n", sum, mean);
}
