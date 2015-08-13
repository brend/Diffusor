#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "ip.h"

#define STD_SIGMA 1.0f
#define STD_KY 9
#define STD_KX 9

void print_help(const char * filename);
int perform_convolution(FILE *in, FILE *out, float sigma, int kx, int ky, const char *comment, BOOL binary);

int main(int argc, char **argv) {
  FILE *in = stdin, *out = stdout;
  float sigma = STD_SIGMA;
  int kernel_width = STD_KX, kernel_height = STD_KY;
  char *comment = NULL;

  char *arg_in = NULL, *arg_out = NULL, *arg_sigma = NULL, *arg_kernelsize = NULL;
  BOOL arg_comment = FALSE, arg_binary = TRUE;

  int choice;

  // Parsing options
  opterr = 0;

  while ((choice = getopt(argc, argv, "f:o:s:k:cvh")) != -1)
    switch (choice) {
    case 'f':
      arg_in = optarg;
      break;
    case 'o':
      arg_out = optarg;
      break;
    case 's':
      arg_sigma = optarg;
      break;
    case 'k':
      arg_kernelsize = optarg;
      break;
    case 'c':
      arg_comment = TRUE;
      break;
    case 'v':
      arg_binary = FALSE;
      break;
    case 'h':
      print_help(argv[0]);
      return 0;
    case '?':
      if (isprint(optopt))
	fprintf(stderr, "Unknown option '-%c'.\n", optopt);
      else
	fprintf(stderr, "Unknown option character '\\x%x'.\n", optopt);
      return 1;
    default:
      abort();
    }

  for (choice = optind; choice < argc; ++choice)
    if (arg_in == NULL)
      arg_in = argv[choice];
    else if (arg_out == NULL)
      arg_out = argv[choice];
    else
      fprintf(stderr, "Unknown non-option argument %s\n", argv[choice]);

  // Processing options
  if (arg_in != NULL)
    if (!(in = fopen(arg_in, "rb"))) {
      fprintf(stderr, "Couldn't open file for reading: %s\n", arg_in);
      return 2;
    }
  if (arg_out != NULL)
    if (!(out = fopen(arg_out, "wb"))) {
      fprintf(stderr, "Couldn't open file for writing: %s\n", arg_out);
      return 3;
    }
  if (arg_sigma != NULL)
    if ((sigma = atof(arg_sigma)) <= 0.0f) {
      fprintf(stderr, "Standard deviation sigma must be greater than zero.\n");
      return 7;
    }
  if (arg_kernelsize != NULL)
    if ((kernel_width = kernel_height = atoi(arg_kernelsize)) < 1) {
      fprintf(stderr, "Kernel size must be greater than zero.\n");
      return 8;
    }
  if (arg_comment) {
    if (arg_in)
      asprintf(&comment, "Convolution with Gaussian\nInitial image:\t%s\nStandard deviation sigma: %f\nKernel size:\t%d * %d", arg_in, sigma, kernel_width, kernel_height);
    else
      asprintf(&comment, "Convolution with Gaussian\nStandard deviation sigma: %f\nKernel size:\t%d * %d", sigma, kernel_width, kernel_height);
  }

  // Performing convolution operation
  int result = perform_convolution(in, out, sigma, kernel_width, kernel_height, comment, arg_binary);

  fclose(out);
  fclose(in);
  if (comment)
    free(comment);

  switch (result) {
  case 4:
    if (arg_in == NULL)
      fprintf(stderr, "Couldn't read image data from standard input.\n");
    else
      fprintf(stderr, "Couldn't read data in image file: %s.\n", arg_in);
    break;
  case 5:
    fprintf(stderr, "Couldn't create Gaussian kernel.\n");
    break;
  case 6:
    fprintf(stderr, "Error while performing convolution.\n");
    break;
  }

  return result;
}

int perform_convolution(FILE *in, FILE *out, float sigma, int kx, int ky, const char *comment, BOOL binary) {
  int ux = 0, uy = 0;
  float **u = ip_load_image(in, &ux, &uy, NULL);

  if (!u)
    return 4;

  float **kernel = gaussian_kernel(kx, ky, sigma);

  if (!kernel) {
    ip_deallocate_image(ux, uy, u);
    return 5;
  }

  // TODO Schummlung entfernen
  dummies(u, ux, uy);
  float **v = convolve(ux + 2, uy + 2, u, kx, ky, kernel);

  ip_deallocate_image(ux, uy, u);
  ip_deallocate_image(kx, ky, kernel);

  if (!v)
    return 6;

  ip_save_image(out, ux, uy, v, comment, binary);

  ip_deallocate_image(ux, uy, v);

  return 0;
}

void print_help(const char *filename) {
  printf("usage: %s [-h] [-v] [-f source] [-o target] [-s sigma] [-k kernel] [input file] [output file]\n"
	 "The options are:\n"
	 "-f\tread input from file \"source\" (default stdin or specify \"input file\")\n"
	 "-o\twrite output to file \"target\" (default stdout or specify \"output file\")\n"
	 "-s\tuse standard deviation \"sigma\" (float, default %f)\n"
	 "-k\tuse quadratic kernel of size \"kernel\" squared (integer, default %d^2)\n"
	 "-v\tsave image in ascii (P2) format (default binary, i.e. P5)\n"
	 "-h\tshow this help\n", filename, STD_SIGMA, STD_KX);
}
