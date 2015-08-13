#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "diff_tensor.h" 

/*--------------------------------------------------------------------------*/
/*                                                                          */
/*           COHERENCE-ENHANCING ANISOTROPIC DIFFUSION FILTERING            */
/*                                                                          */
/*                       (Joachim Weickert, 6/2000)                         */
/*                                                                          */
/*--------------------------------------------------------------------------*/


/* 
 features:
 - explicit scheme
 - presmoothing at noise scale:  convolution-based, Neumann b.c.
 - presmoothing at integration scale: convolution-based, Dirichlet b.c.
*/

/* ------------------------------------------------------------------------ */

void dummies
     (float **v,        /* image matrix */
      long  nx,         /* size in x direction */
      long  ny)         /* size in y direction */

/* creates dummy boundaries by mirroring */

{
long i, j;  /* loop variables */

for (i=1; i<=nx; i++)
    {
    v[i][0]    = v[i][1];
    v[i][ny+1] = v[i][ny];
    }

for (j=0; j<=ny+1; j++)
    {
    v[0][j]    = v[1][j];
    v[nx+1][j] = v[nx][j];
    }
return;
}

void PA_trans 

     (float a11,        /* coeffs of (2*2)-matrix */ 
      float a12,        /* coeffs of (2*2)-matrix */ 
      float a22,        /* coeffs of (2*2)-matrix */ 
      float *c,         /* 1. comp. of 1. eigenvector, output */ 
      float *s,         /* 2. comp. of 1. eigenvector, output */ 
      float *lam1,      /* larger  eigenvalue, output */
      float *lam2)      /* smaller eigenvalue, output */

/*
 Principal axis transformation. 
*/

{
float  help, norm;    /* time savers */ 

/* eigenvalues */
help  = sqrt (pow(a22-a11, 2.0) + 4 * a12 * a12);
*lam1 = (a11 + a22 + help) / 2.0; 
*lam2 = (a11 + a22 - help) / 2.0; 

/* eigenvectors */
*c = 2.0 * a12;
*s = a22 - a11 + help;

/* normalized eigenvectors */
norm = sqrt (*c * *c + *s * *s);
if (norm >= 0.0000001)
   {
   *c = *c / norm;
   *s = *s / norm;
   }
else
   {
   *c = 1.0;
   *s = 0.0;
   }
return;

} /* PA_trans */

/* ----------------------------------------------------------------------- */

void PA_backtrans 

     (float  c,      /* 1. comp. of 1. eigenvector */ 
      float  s,      /* 2. comp. of 1. eigenvector */ 
      float  lam1,   /* 1. eigenvalue */ 
      float  lam2,   /* 2. eigenvalue */ 
      float  *a11,   /* coeff. of (2*2)-matrix, output */ 
      float  *a12,   /* coeff. of (2*2)-matrix, output */ 
      float  *a22)   /* coeff. of (2*2)-matrix, output */ 


/*
 Principal axis backtransformation of a symmetric (2*2)-matrix. 
 A = U * diag(lam1, lam2) * U_transpose with U = (v1 | v2)     
 v1 = (c, s) is first eigenvector
*/

{

*a11 = c * c * lam1 + s * s * lam2;
*a22 = lam1 + lam2 - *a11;             /* trace invariance */
*a12 = c * s * (lam1 - lam2);

return;

} /* PA_backtrans */

/*--------------------------------------------------------------------------*/

void diff_tensor_ced
     (float    C,        /* contrast parameter */
      float    alpha,    /* linear diffusion fraction */
      long     nx,       /* image dimension in x direction */
      long     ny,       /* image dimension in y direction */
      float    **dxx,    /* in: structure tensor el., out: diff. tensor el. */
      float    **dxy,    /* in: structure tensor el., out: diff. tensor el. */ 
      float    **dyy)    /* in: structure tensor el., out: diff. tensor el. */ 

/*
 Calculates the diffusion tensor of CED by means of the structure tensor.
*/

{
long    i, j;          /* loop variables */
float   beta;          /* time saver */
float   c, s;          /* specify first eigenvector */
float   mu1, mu2;      /* eigenvalues of structure tensor */
float   lam1, lam2;    /* eigenvalues of diffusion tensor */

beta = 1.0 - alpha;

for (i=1; i<=nx; i++)
 for (j=1; j<=ny; j++)
     {
     /* principal axis transformation */
     PA_trans (dxx[i][j], dxy[i][j], dyy[i][j], &c, &s, &mu1, &mu2);

     /* calculate eigenvalues */
     lam1 = alpha;
     if (mu1 == mu2)
        lam2 = alpha;
     else
        lam2 = alpha + beta * exp (- C / ((mu1 - mu2) * (mu1 - mu2)));

     /* principal axis backtransformation */
     PA_backtrans (c, s, lam1, lam2, &dxx[i][j], &dxy[i][j], &dyy[i][j]); 
     }

return;

}  /* diff_tensor_ced */

void diff_tensor_eed

     (float    **v,      /* in: regularized image, unchanged */
      float    lambda,   /* contrast parameter */
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
float   c, s;             /* specify first eigenvector */
float   grad;             /* |grad(v)| */
float   mu1, mu2;         /* eigenvalues of structure tensor */
float   lam1, lam2;       /* eigenvalues of diffusion tensor */

/* time saver variables */
two_hx = 2.0 * hx;
two_hy = 2.0 * hy;
dummies (v, nx, ny);

/* for each pixel */
for (i=1; i<=nx; i++)
 for (j=1; j<=ny; j++)
     {
       /* calculate grad(v) */
       dv_dx = (v[i+1][j] - v[i-1][j]) / two_hx;
       dv_dy = (v[i][j+1] - v[i][j-1]) / two_hy;
       grad  = sqrt (dv_dx * dv_dx + dv_dy * dv_dy);
       
       /* set up strcuture tensor */
       dxx[i][j]=dv_dx*dv_dx;
       dxy[i][j]=dv_dx*dv_dy;
       dyy[i][j]=dv_dy*dv_dy;


       /* principal axis transformation of structure tensor*/
       PA_trans(dxx[i][j], dxy[i][j], dyy[i][j], &c, &s, &mu1, &mu2);

       /* calculate eigenvalues lam1, lam2 of diffusion tensor */
       lam1 = 1.0 / (1 + grad / (lambda * lambda));
       lam2 = 1;

       /* principal axis backtransformation of diffusion tensor */
       PA_backtrans (c, s, lam1, lam2, &dxx[i][j], &dxy[i][j], &dyy[i][j]);
     }
 
 return;

}  /* diff_tensor_eed */
