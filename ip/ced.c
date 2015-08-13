#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <ctype.h>
#include "ip.h"

#define STD_SIGMA 0.5f
#define STD_RHO 4.0f
#define STD_C 1.0f
#define STD_ALPHA 0.001f
#define STD_TAU 0.24f
#define STD_ITERATIONS 1

void print_help(const char * filename);
int perform_ced(FILE *in, FILE *out, float sigma, float rho, float C, float alpha, float tau, int iterations, const char *comment, BOOL binary);

int main(int argc, char **argv) {
  FILE *in = stdin, *out = stdout;
  float sigma = STD_SIGMA, rho = STD_RHO, C = STD_C, alpha = STD_ALPHA, tau = STD_TAU, iterations = STD_ITERATIONS;
  char *comment = NULL;

  char *arg_in = NULL, *arg_out = NULL, *arg_sigma = NULL, *arg_rho = NULL, *arg_c = NULL, *arg_alpha = NULL, *arg_tau = NULL,
    *arg_iterations = NULL;
  BOOL arg_binary = TRUE;

  int choice;

  // Parsing options
  opterr = 0;

  while ((choice = getopt(argc, argv, "f:o:s:r:c:a:t:i:vh")) != -1)
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
    case 'r':
      arg_rho = optarg;
      break;
    case 'c':
      arg_c = optarg;
      break;
    case 'a':
      arg_alpha = optarg;
      break;
    case 't':
      arg_tau = optarg;
      break;
    case 'i':
      arg_iterations = optarg;
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
      fprintf(stderr, "Ignored non-option argument %s\n", argv[choice]);

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
  if (arg_rho != NULL)
    if ((rho = atof(arg_rho)) <= 0.0f) {
      fprintf(stderr, "Tensor integration scale rho must be greater than zero.\n");
      return 8;
    }
  if (arg_c != NULL)
    if ((C = atof(arg_c)) <= 0.0f) {
      fprintf(stderr, "Contrast parameter C must be greater than zero.\n");
      return 9;
    }
  if (arg_alpha != NULL)
    if ((alpha = atof(arg_alpha)) <= 0.0f) {
      fprintf(stderr, "Regularisation parameter alpha must be greater than zero.\n");
      return 10;
    }
  if (arg_tau != NULL)
    if ((tau = atof(arg_tau)) <= 0.0f || tau >= 0.25f) {
      fprintf(stderr, "Time step size tau must be greater than zero and less than 0.25.\n");
      return 11;
    }
  if (arg_iterations != NULL)
    if ((iterations = atoi(arg_iterations)) <= 0) {
      fprintf(stderr, "Number of iterations must be greater than zero.\n");
      return 12;
    }

  // Performing convolution operation
  int result = perform_ced(in, out, sigma, rho, C, alpha, tau, iterations, comment, arg_binary);
  
  fclose(out);
  fclose(in);
  if (comment)
    free(comment);

  if (result != 0)
    fprintf(stderr, "Error while performing filtering.\n");

  return result;
}

int perform_ced(FILE *in, FILE *out, float sigma, float rho, float C, float alpha, float tau, int iterations, const char *comment, BOOL binary) {
  int nx = 0, ny = 0;
  float **f = ip_load_image(in, &nx, &ny, NULL);

  if (!f)
    return ~0;

  ip_gaussian_smooth(sigma, nx, ny, f);

  int i;
  for (i = 0; i < iterations; ++i) {
    ip_ced(C, rho, alpha, tau, nx, ny, f);
  }

  ip_save_image(out, nx, ny, f, comment, binary);

  ip_deallocate_image(nx, ny, f);

  return 0;
}

void print_help(const char *filename) {
  printf("usage: %s [-h] [-v] [-f source] [-o target] [-s sigma] [-r rho] [-c C] [-a alpha] [-t tau] [-i iterations] [input file] [output file]\n"
	 "The options are:\n"
	 "-f\tread input from file \"source\" (default stdin or specify \"input file\")\n"
	 "-o\twrite output to file \"target\"(default stdout or specify \"output file\")\n"
	 "-s\tuse pre-smoothing \"sigma\" (float, default %f)\n"
	 "-r\tuse integration scale \"rho\" (float, default %f)\n"
	 "-c\tuse contrast parameter \"C\" (float, default %f)\n"
	 "-a\tuse regularisation parameter \"alpha\" (float, default %f)\n"
	 "-t\tuse time step size \"tau\" (float, default %f)\n"
	 "-i\tperform \"iterations\" iterations (integer, default 1)\n"
	 "-v\tsave image in ascii (P2) format (default binary, i.e. P5)\n"
	 "-h\tshow this help\n", filename, STD_SIGMA, STD_RHO, STD_C, STD_ALPHA, STD_TAU);
}
