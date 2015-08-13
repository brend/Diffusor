
void dummies
     (float **v,        /* image matrix */
      long  nx,         /* size in x direction */
      long  ny);        /* size in y direction */

void PA_trans

     (float a11,        /* coeffs of (2*2)-matrix */
      float a12,        /* coeffs of (2*2)-matrix */
      float a22,        /* coeffs of (2*2)-matrix */
      float *c,         /* 1. comp. of 1. eigenvector, output */
      float *s,         /* 2. comp. of 1. eigenvector, output */
      float *lam1,      /* larger  eigenvalue, output */
      float *lam2);     /* smaller eigenvalue, output */

void PA_backtrans

     (float  c,         /* 1. comp. of 1. eigenvector */
      float  s,         /* 2. comp. of 1. eigenvector */
      float  lam1,      /* 1. eigenvalue */
      float  lam2,      /* 2. eigenvalue */
      float  *a11,      /* coeff. of (2*2)-matrix, output */
      float  *a12,      /* coeff. of (2*2)-matrix, output */
      float  *a22);     /* coeff. of (2*2)-matrix, output */

void diff_tensor_ced
     (float    C,        /* contrast parameter */
      float    alpha,    /* linear diffusion fraction */
      long     nx,       /* image dimension in x direction */
      long     ny,       /* image dimension in y direction */
      float    **dxx,    /* in: structure tensor el., out: diff. tensor el. */
      float    **dxy,    /* in: structure tensor el., out: diff. tensor el. */
      float    **dyy);   /* in: structure tensor el., out: diff. tensor el. */

void diff_tensor_eed

     (float    **v,      /* in: regularized image, unchanged */
      float    lambda,   /* contrast parameter */
      long     nx,       /* image dimension in x direction */
      long     ny,       /* image dimension in y direction */
      float    hx,       /* pixel width in x directopn */
      float    hy,       /* pixel width in y directopn */
      float    **dxx,    /* out: diffusion tensor element */
      float    **dxy,    /* out: diffusion tensor element */
      float    **dyy);    /* out: diffusion tensor element */
