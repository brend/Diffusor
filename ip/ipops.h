#ifndef _IPOPS_H_
#define _IPOPS_H_

#include "diff_tensor.h"

#define STD_KERNELSIZE 5

/*
Convolutes images u (of dimension ux*uy) and v (of dimensions vx*vy),
allocates and returns resulting image (of dimension ux*uy).
 */
float **convolve(
		 int ux, int uy, float **u, /* input image with dimensions */
		 int vx, int vy, float **v /* input kernel with dimensions */
		 );

/*
Creates a Gaussian kernel of dimension nx * ny with standard deviation sigma.
*/
float **gaussian_kernel(
		     int nx, /* kernel dimension in x direction */
		     int ny, /* kernel dimension in y direction */
		     float sigma /* standard deviation of kernel */
		     );

void ip_gaussian_smooth(
			float sigma,
			int nx,
			int ny,
			float **u
			);

/*
Rescales image s.t. image values sum up to 1.0f (i.e., sum_image(nx,ny,u) == 1.0f.
 */
void norm_image(
		int nx,
		int ny,
		float **u
		);

void norm_vector(
		 int n,
		 float *u
		 );

float sum_image(
		int nx,
		int ny,
		float **u);

float sum_vector(
		 int n,
		 float *u
		 );

float mean_value(
		 int nx,
		 int ny,
		 float **u
		 );

void scale_image(
		 float f,
		 int nx,
		 int ny,
		 float **u
		 );

void structure_tensor
     (float    **v,      /* in: regularized image, unchanged */
      long     nx,       /* image dimension in x direction */
      long     ny,       /* image dimension in y direction */
      float    hx,       /* pixel width in x directopn */
      float    hy,       /* pixel width in y directopn */
      float    **dxx,    /* out: diffusion tensor element */
      float    **dxy,    /* out: diffusion tensor element */
      float    **dyy);   /* out: diffusion tensor element */

/*
  Performs CED.
*/
void ip_ced(
	    float C,   /* coherence parameter */
	    float rho, /* structure tensor integration scale */
	    float alpha,
	    float ht,  /* time step size */
	    int nx,    /* image dimension in x direction */
	    int ny,    /* image dimension in y direction */
	    float **f  /* in: gaussian smoothed image, out: filtered image */
	    );

void ip_eed(
	    float lambda,
	    float ht,
	    int nx, 
	    int ny,
	    float **f
	    );

#endif
