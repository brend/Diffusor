#include "ipops.h"

#include "ipmem.h"
#include <math.h>
#include <stdlib.h>
#include <assert.h>

/*
Creates a Gaussian kernel of dimension nx * ny with standard deviation sigma.
*/
float **gaussian_kernel(
		     int nx, /* kernel dimension in x direction */
		     int ny, /* kernel dimension in y direction */
		     float sigma /* standard deviation of kernel */
		     )
{
  float **u = ip_allocate_image(nx, ny);

  if (u) {
    float variance = sigma * sigma;
    float f1 = 1.0f / sqrt(2 * M_PI * sigma), f2 = -1.0f / (2 * variance);

    float m_x = (float) nx / 2.0f, m_y = (float) ny / 2.0f;
    
    int x, y;
    
    for (y = 0; y < ny; ++y) {
      float k_y = (float) y - m_y + 0.5f;
      
      for (x = 0; x < nx; ++x) {
	float k_x = (float) x - m_x + 0.5f;
	float norm_sq = k_x*k_x + k_y*k_y;
	
	u[x][y] = f1 * exp(f2 * norm_sq);
      }
    }

    // Normalize the kernel
    norm_image(nx, ny, u);
  }

  return u;
}

void ip_gaussian_kernel(
			float sigma,
			int n,
			float *u
			)
{
  assert(u);

  float variance = sigma * sigma;
  float f1 = 1.0f / sqrt(2 * M_PI * sigma), f2 = -1.0f / (2 * variance);
  
  float m_x = (float) n / 2.0f;
  
  int x;
  
  for (x = 0; x < n; ++x) {
    float k_x = (float) x - m_x + 0.5f;
    float norm_sq = k_x*k_x;
    
    u[x] = f1 * exp(f2 * norm_sq);
  }
  
  norm_vector(n, u);
}

void ip_gaussian_smooth(
			float sigma,
			int nx,
			int ny,
			float **u
			)
{
  // TODO Durch bessere Methode ersetzen (ohne Schummeln (nx + ...))
  int kx = 5, ky = 5;
  float **kernel = gaussian_kernel(kx, ky, sigma);

  dummies(u, nx, ny);

  float **v = convolve(nx + 2, ny + 2, u, kx, ky, kernel);
  int i, j;

  for (j = 1; j <= ny; ++j)
    for (i = 1; i <= nx; ++i)
      u[i][j] = v[i][j];

  ip_deallocate_image(nx + 2, ny + 2, v);
  ip_deallocate_image(kx, ky, kernel);
}

/*
Convolutes images u (of dimension ux*uy) and v (of dimensions vx*vy),
allocates and returns resulting image (of dimension ux*uy).
 */
float **convolve(
		 int ux, int uy, float **u, /* input image with dimensions */
		 int vx, int vy, float **v /* input kernel with dimensions */
		 )
{
  assert(u && v);
  assert(ux >= 0 && uy >= 0 && vx >= 0 && vy >= 0);

  float **r = ip_allocate_image(ux, uy);

  if (r) {
    int i, j;

    for (j = 0; j < uy; ++j) {
      for (i = 0; i < ux; ++i) {
	float sum = 0;
	int k, l;

	for (l = 0; l < vy; ++l)
	  for (k = 0; k < vx; ++k) {
	    // int px = abs((i - k) % ux), py = abs((j - l) % uy);
	    int px = abs((i - (k - vx/2)) % ux), py = abs((j - (l - vy/2)) % uy);

	    sum += u[px][py] * v[k][l];
	  }

	r[i][j] = sum;
      }
    }
  }

  return r;
}

void norm_image(
		int nx,
		int ny,
		float **u
		)
{
  assert(u);
  assert(nx >= 0 && ny >= 0);

  float f = 1.0f / sum_image(nx, ny, u);
  int x, y;

  for (y = 0; y < ny; ++y)
    for (x = 0; x < nx; ++x)
      u[x][y] = f * u[x][y];
}

void norm_vector(
		 int n,
		 float *u
		 )
{
  assert(u);
  assert(n >= 0);

  float f = 1.0f / sum_vector(n, u);
  int x;

  for (x = 0; x < n; ++x)
    u[x] = f * u[x];
}

float mean_value(
		 int nx,
		 int ny,
		 float **u
		 )
{
  float sum = sum_image(nx, ny, u);
  float mean = sum / (nx * ny);

  return mean;
}

float sum_image(
		int nx,
		int ny,
		float **u)
{
  float sum = 0.0f;
  int x, y;

  for (y = 0; y < ny; ++y)
    for (x = 0; x < nx; ++x)
      sum += u[x][y];

  return sum;
}

float sum_vector(
		 int n,
		 float *u
		 )
{
  float sum = 0.0f;
  int x;

  for (x = 0; x < n; ++x)
    sum += u[x];

  return sum;
}

void scale_image(float f, int nx, int ny, float **u)
{
  int x, y;

  for (y = 0; y < ny; ++y)
    for (x = 0; x < nx; ++x)
      u[x][y] *= f;
}

void structure_tensor
     (float    **v,      /* in: regularized image, unchanged */
      long     nx,       /* image dimension in x direction */
      long     ny,       /* image dimension in y direction */
      float    hx,       /* pixel width in x directopn */
      float    hy,       /* pixel width in y directopn */
      float    **dxx,    /* out: diffusion tensor element */
      float    **dxy,    /* out: diffusion tensor element */
      float    **dyy)    /* out: diffusion tensor element */

/*
 Calculates the diffusion tensor of EED.
*/

{
  long    i, j;             /* loop variables */
  float   dv_dx, dv_dy;     /* derivatives of v */
  float   two_hx, two_hy;   /* time savers */
  
  /* time saver variables */
  two_hx = 2.0 * hx;
  two_hy = 2.0 * hy;
  dummies (v, nx, ny);
  
  /* for each pixel */
  for (i=1; i<=nx; i++)
    for (j=1; j<=ny; j++) {
      /* calculate grad(v) */
      dv_dx = (v[i+1][j] - v[i-1][j]) / two_hx;
      dv_dy = (v[i][j+1] - v[i][j-1]) / two_hy;
      
      /* set up strcuture tensor */
      dxx[i][j]=dv_dx*dv_dx;
      dxy[i][j]=dv_dx*dv_dy;
      dyy[i][j]=dv_dy*dv_dy;
    }
}

/*
  Performs CED.
*/
void ip_ced(
	    float C,   /* coherence parameter */
	    float rho, /* structure tensor integration scale */
	    float alpha,
	    float ht,  /* time step size; implicitly hx = hy = 1 */
	    int nx,    /* image dimension in x direction */
	    int ny,    /* image dimension in y direction */
	    float **f  /* in: gaussian smoothed image, out: filtered image */
	    )
{
  float **v = ip_allocate_image(nx + 2, ny + 2);
  float hx = 1, hy = 1;

  int i, j;

  for (j = 1; j <= ny; ++j)
    for (i = 1; i <= nx; ++i)
      v[i][j] = f[i][j];

  float
    **a = ip_allocate_image(nx + 2, ny + 2),
    **b = ip_allocate_image(nx + 2, ny + 2),
    **c = ip_allocate_image(nx + 2, ny + 2);

  /*
  // EED
  float lambda = 3.5f;

  diff_tensor_eed(v, lambda, nx, ny, hx, hy, a, b, c);
  // END OF EED
  */

  // CED-specific
  structure_tensor(v, nx, ny, hx, hy, a, b, c);

  ip_gaussian_smooth(rho, nx, ny, a);
  ip_gaussian_smooth(rho, nx, ny, b);
  ip_gaussian_smooth(rho, nx, ny, c);

  diff_tensor_ced(C, alpha, nx, ny, a, b, c);
  // END OF CED-specific

  // compute nonlinear anisotropic diffusion
  dummies(v, nx, ny);
  dummies(a, nx, ny);
  dummies(b, nx, ny);
  dummies(c, nx, ny);

  for (j = 1; j <= ny; ++j)
    for (i = 1; i <= nx; ++i)
      f[i][j] = v[i][j] + ht * (
	  v[i-1][j+1] * (-b[i-1][j] - b[i][j+1]) / (4 * hx * hy)
	+ v[ i ][j+1] * (c[i][j+1] + c[i][j]) / (2 * hy * hy)
	+ v[i+1][j+1] * (b[i+1][j] + b[i][j+1]) / (4 * hx * hy)
	+ v[i-1][ j ] * (a[i-1][j] + a[i][j]) / (2 * hx * hx)
	+ v[ i ][ j ] * (-(a[i-1][j] + 2 * a[i][j] + a[i+1][j]) / (2 * hx * hx) - (c[i][j-1] + 2 * c[i][j] + c[i][j+1]) / (2 * hy * hy))
	+ v[i+1][ j ] * (a[i+1][j] + a[i][j]) / (2 * hx * hx)
	+ v[i-1][j-1] * (b[i-1][j] + b[i][j-1]) / (4 * hx * hy)
	+ v[ i ][j-1] * (c[i][j-1] + c[i][j]) / (2 * hy * hy)
	+ v[i+1][j-1] * (-b[i+1][j] - b[i][j-1]) / (4 * hx * hy)
		      );

  ip_deallocate_image(nx + 2, ny + 2, v);

  ip_deallocate_image(nx + 2, ny + 2, a);
  ip_deallocate_image(nx + 2, ny + 2, b);
  ip_deallocate_image(nx + 2, ny + 2, c);
}

void ip_eed(
	    float lambda,
	    float ht,
	    int nx, 
	    int ny,
	    float **f
	    )
{
  float **v = ip_allocate_image(nx + 2, ny + 2);
  float hx = 1, hy = 1;

  int i, j;

  for (j = 1; j <= ny; ++j)
    for (i = 1; i <= nx; ++i)
      v[i][j] = f[i][j];

  float
    **a = ip_allocate_image(nx + 2, ny + 2),
    **b = ip_allocate_image(nx + 2, ny + 2),
    **c = ip_allocate_image(nx + 2, ny + 2);

  // EED-specific
  diff_tensor_eed(v, lambda, nx, ny, hx, hy, a, b, c);
  // END OF EED-specific

  // compute nonlinear anisotropic diffusion
  dummies(v, nx, ny);
  dummies(a, nx, ny);
  dummies(b, nx, ny);
  dummies(c, nx, ny);

  for (j = 1; j <= ny; ++j)
    for (i = 1; i <= nx; ++i)
      f[i][j] = v[i][j] + ht * (
	  v[i-1][j+1] * (-b[i-1][j] - b[i][j+1]) / (4 * hx * hy)
	+ v[ i ][j+1] * (c[i][j+1] + c[i][j]) / (2 * hy * hy)
	+ v[i+1][j+1] * (b[i+1][j] + b[i][j+1]) / (4 * hx * hy)
	+ v[i-1][ j ] * (a[i-1][j] + a[i][j]) / (2 * hx * hx)
	+ v[ i ][ j ] * (-(a[i-1][j] + 2 * a[i][j] + a[i+1][j]) / (2 * hx * hx) - (c[i][j-1] + 2 * c[i][j] + c[i][j+1]) / (2 * hy * hy))
	+ v[i+1][ j ] * (a[i+1][j] + a[i][j]) / (2 * hx * hx)
	+ v[i-1][j-1] * (b[i-1][j] + b[i][j-1]) / (4 * hx * hy)
	+ v[ i ][j-1] * (c[i][j-1] + c[i][j]) / (2 * hy * hy)
	+ v[i+1][j-1] * (-b[i+1][j] - b[i][j-1]) / (4 * hx * hy)
		      );

  ip_deallocate_image(nx + 2, ny + 2, v);

  ip_deallocate_image(nx + 2, ny + 2, a);
  ip_deallocate_image(nx + 2, ny + 2, b);
  ip_deallocate_image(nx + 2, ny + 2, c);
}
